#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <ctype.h>
#include <errno.h>
#include <time.h>
#include <stdarg.h>
#include "mta_crypt.h"
#include "mta_rand.h"

#define PIPE_DIR "/mnt/mta/"
#define ENCRYPTER_PIPE "/mnt/mta/server_pipe"
#define LOG_FILE "/var/log/mtacrypt.log"
#define MAX_MSG 1024
#define MAX_PIPE_NAME 256

int my_id = 1;
FILE* log_file = NULL;

long get_timestamp() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec;
}


void print_str(FILE* out, const char* buf, unsigned int len) {
    for (unsigned int i = 0; i < len; ++i)
        fprintf(out, "%c", isprint((unsigned char)buf[i]) ? buf[i] : '.');
}

int is_printable_str(const char* buf, unsigned int len) {
    for (unsigned int i = 0; i < len; ++i)
        if (!isprint((unsigned char)buf[i]))
            return 0;
    return 1;
}

void log_printf(const char *format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(log_file, format, args);
    fflush(log_file);
    va_end(args);
}

int find_next_available_id() {
    for (int id = 1; id <= 32; id++) {
        char test_path[MAX_PIPE_NAME];
        snprintf(test_path, sizeof(test_path), "%sdecrypter_pipe_%d", PIPE_DIR, id);
        if (access(test_path, F_OK) != 0) {
            return id;
        }
    }
    return 1;
}

int main() {
    log_file = fopen(LOG_FILE, "w");
    if (!log_file) {
        perror("Failed to open log file");
        exit(EXIT_FAILURE);
    }

    if (MTA_crypt_init() != MTA_CRYPT_RET_OK) {
        log_printf("[CLIENT] Failed to initialize crypto library!\n");
        exit(EXIT_FAILURE);
    }

    my_id = find_next_available_id();

    char pipe_name[MAX_PIPE_NAME], pipe_path[MAX_PIPE_NAME * 2];
    snprintf(pipe_name, sizeof(pipe_name), "decrypter_pipe_%d", my_id);
    snprintf(pipe_path, sizeof(pipe_path), "%s%s", PIPE_DIR, pipe_name);

    if (mkfifo(pipe_path, 0666) == -1 && errno != EEXIST) {
        perror("mkfifo pipe_path");
        exit(EXIT_FAILURE);
    }

    // Register to server
    while (1) {
        int reg_fd = open(ENCRYPTER_PIPE, O_WRONLY | O_NONBLOCK);
        if (reg_fd >= 0) {
            char reg_msg[MAX_PIPE_NAME + 16];
            snprintf(reg_msg, sizeof(reg_msg), "SUBSCRIBE:%s\n", pipe_name);
            if (write(reg_fd, reg_msg, strlen(reg_msg)) > 0) {
                close(reg_fd);
                log_printf("%ld  [CLIENT #%d]  [INFO] Sent connect request to server\n", get_timestamp(), my_id);
                break;
            }
            close(reg_fd);
        }
        usleep(100000);
    }

    int fd = open(pipe_path, O_RDONLY);
    if (fd < 0) {
        log_printf("%ld  [CLIENT #%d]  [ERROR] Failed to open %s for reading: %s\n", get_timestamp(), my_id, pipe_path, strerror(errno));
        exit(EXIT_FAILURE);
    }

    char encrypted[MAX_MSG];
    unsigned int password_len = 0;
    unsigned int key_len = 0;
    unsigned long iterations = 0;
    int have_password = 0;
    int first_password = 1;

    while (1) {
        if (!have_password) {
            ssize_t n = read(fd, encrypted, sizeof(encrypted));
            if (n <= 0) {
                usleep(100000);
                continue;
            }
            password_len = n;
            key_len = password_len / 8;
            iterations = 0;
            have_password = 1;

            if (first_password) {
                 log_printf("%ld  [CLIENT #%d]  [INFO] Received encrypted password %.*s\n", get_timestamp(), my_id, password_len, encrypted);
                 first_password = 0;
            } else {
                log_printf("%ld  [CLIENT #%d]  [INFO] Received new encrypted password %.*s\n", get_timestamp(), my_id, password_len, encrypted);
            }
        }

        // Try to brute-force decrypt
        while (have_password) {
            iterations++;
            char* guess_key = malloc(key_len);
            char* decrypted = malloc(password_len);
            unsigned int decrypted_len = 0;

            MTA_get_rand_data(guess_key, key_len);

            if (MTA_decrypt(guess_key, key_len, encrypted, password_len, decrypted, &decrypted_len) == MTA_CRYPT_RET_OK) {
                if (decrypted_len == password_len && is_printable_str(decrypted, decrypted_len)) {
                    log_printf("%ld  [CLIENT #%d]  [INFO] Decrypted password: ", get_timestamp(), my_id);
                    print_str(log_file, decrypted, decrypted_len);
                    log_printf(", Key: ");
                    print_str(log_file, guess_key, key_len);
                    log_printf(" (in %lu iterations)\n", iterations);

                    // Send solution to server via server_pipe
                    int sol_fd = open(ENCRYPTER_PIPE, O_WRONLY | O_NONBLOCK);
                    if (sol_fd >= 0) {
                        char solution_msg[MAX_MSG + 32];
                        int len = snprintf(solution_msg, sizeof(solution_msg), "SOLUTION:%d:", my_id);
                        memcpy(solution_msg + strlen(solution_msg), decrypted, decrypted_len);
                        len = strlen(solution_msg) + decrypted_len;
                        solution_msg[len++] = '\n';
                        write(sol_fd, solution_msg, len);
                        close(sol_fd);
                    }
                    have_password = 0;
                    break;
                }
            }
            free(guess_key);
            free(decrypted);

            // Every 1000 iterations, check for new password (non-blocking)
            if (iterations % 1000 == 0) {
                char new_encrypted[MAX_MSG];
                ssize_t new_n = read(fd, new_encrypted, sizeof(new_encrypted));
                if (new_n > 0 && (new_n != password_len || memcmp(new_encrypted, encrypted, password_len) != 0)) {
                    memcpy(encrypted, new_encrypted, new_n);
                    password_len = new_n;
                    key_len = password_len / 8;
                    iterations = 0;
                    log_printf("%ld  [CLIENT #%d]  [INFO] Received new encrypted password %.*s\n", get_timestamp(), my_id, password_len, encrypted);
                    break;
                }
            }
        }
    }

    close(fd);
    fclose(log_file);
    return 0;
}

