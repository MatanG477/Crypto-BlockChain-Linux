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

#define ENCRYPTER_PIPE "/mnt/mta/server_pipe"
#define PIPE_DIR "/mnt/mta/"
#define CONF_FILE "/mnt/mta/mtacrypt.conf"
#define LOG_FILE "/var/log/mtacrypt.log"
#define MAX_DECRYPTERS 32
#define MAX_MSG 1024
#define MAX_PIPE_NAME 512

typedef struct {
    char pipe_name[MAX_PIPE_NAME];
    int id;
    int active;
} decrypter_t;

decrypter_t decrypters[MAX_DECRYPTERS];
int num_decrypters = 0;
unsigned int password_len = 24;
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

void log_printf(const char *format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(log_file, format, args);
    fflush(log_file);
    va_end(args);
}

void generate_random_printable(char* buf, unsigned int len) {
    for (unsigned int i = 0; i < len; ++i) {
        char c;
        do {
            c = MTA_get_rand_char();
        } while (!isprint((unsigned char)c));
        buf[i] = c;
    }
}

void read_config() {
    log_printf("Reading /mnt/mta/mtacrypt.conf...\n");
    FILE* f = fopen(CONF_FILE, "r");
    if (f) {
        char line[256];
        while (fgets(line, sizeof(line), f)) {
            if (strncmp(line, "PASSWORD_LENGTH=", 16) == 0) {
                password_len = atoi(line + 16);
                log_printf("Password length set to %u\n", password_len);
            }
        }
        fclose(f);
    } else {
        log_printf("[SERVER][ERROR] Could not open config file %s: %s\n", CONF_FILE, strerror(errno));
    }
}

int register_decrypter(const char* pipe_name) {
    for (int i = 0; i < num_decrypters; i++) {
        if (strcmp(decrypters[i].pipe_name, pipe_name) == 0) {
            return decrypters[i].id;
        }
    }
    if (num_decrypters >= MAX_DECRYPTERS) return -1;
    int id = num_decrypters + 1;
    snprintf(decrypters[num_decrypters].pipe_name, sizeof(decrypters[num_decrypters].pipe_name), "%s", pipe_name);
    decrypters[num_decrypters].id = id;
    decrypters[num_decrypters].active = 1;
    log_printf("%ld  [SERVER]  [INFO] Received connection request from decrypter id %d, fifo name %s%s\n",
        get_timestamp(), id, PIPE_DIR, pipe_name);
    num_decrypters++;
    return id;
}

void send_password_to_decrypter(int decrypter_idx, const char* encrypted, unsigned int encrypted_len) {
    char full_path[1024];
    snprintf(full_path, sizeof(full_path), "%s%s", PIPE_DIR, decrypters[decrypter_idx].pipe_name);
    int fd = open(full_path, O_WRONLY | O_NONBLOCK);
    if (fd < 0) {
        log_printf("%ld  [SERVER]  [ERROR] Failed to open %s for writing: %s\n", get_timestamp(), full_path, strerror(errno));
        return;
    }
    ssize_t written = write(fd, encrypted, encrypted_len);
    if (written < 0) {
        log_printf("%ld  [SERVER]  [ERROR] Failed to write to %s: %s\n", get_timestamp(), full_path, strerror(errno));
    }
    close(fd);
}

void broadcast_password(const char* encrypted, unsigned int encrypted_len) {
    for (int i = 0; i < num_decrypters; i++) {
        if (!decrypters[i].active) continue;
        send_password_to_decrypter(i, encrypted, encrypted_len);
    }
}

int main() {
    log_file = fopen(LOG_FILE, "w");
    if (!log_file) {
        perror("Failed to open log file");
        exit(EXIT_FAILURE);
    }

    read_config();

    if (MTA_crypt_init() != MTA_CRYPT_RET_OK) {
        log_printf("[SERVER] Failed to initialize crypto library!\n");
        exit(EXIT_FAILURE);
    }

    umask(0);
    unlink(ENCRYPTER_PIPE);
    if (mkfifo(ENCRYPTER_PIPE, 0666) == -1 && errno != EEXIST) {
        perror("mkfifo");
        exit(EXIT_FAILURE);
    }
    chmod(ENCRYPTER_PIPE, 0666);

    int reg_fd = open(ENCRYPTER_PIPE, O_RDONLY | O_NONBLOCK);
    if (reg_fd < 0) {
        perror("open server_pipe");
        exit(EXIT_FAILURE);
    }

    char* current_password = NULL;
    char* current_encrypted = NULL;
    unsigned int current_encrypted_len = 0;
    char* current_key = NULL;
    unsigned int key_len = 0;
    int first_password = 1;

    while (1) {
        if (!current_password) {
            key_len = password_len / 8;
            current_password = malloc(password_len);
            current_key = malloc(key_len);
            current_encrypted = malloc(password_len);

            generate_random_printable(current_password, password_len);
            MTA_get_rand_data(current_key, key_len);

            if (MTA_encrypt(current_key, key_len, current_password, password_len, current_encrypted, &current_encrypted_len) != MTA_CRYPT_RET_OK) {
                log_printf("%ld  [SERVER]  [ERROR] Encryption failed\n", get_timestamp());
                free(current_password); free(current_key); free(current_encrypted);
                current_password = NULL;
                current_encrypted = NULL;
                current_key = NULL;
                continue;
            }

            if (first_password) {
                log_printf("%ld  [SERVER]  [INFO] New password generated: ", get_timestamp());
                print_str(log_file, current_password, password_len);
                log_printf(", key: ");
                print_str(log_file, current_key, key_len);
                fprintf(log_file, ", After encryption: %.*s", current_encrypted_len, current_encrypted);
                log_printf("\nListening on /mnt/mta/server_pipe\n");
                first_password = 0;
            } else {
                log_printf("%ld  [SERVER]  [INFO] New password: ", get_timestamp());
                print_str(log_file, current_password, password_len);
                log_printf(", key: ");
                print_str(log_file, current_key, key_len);
                fprintf(log_file, ", Encrypted: %.*s", current_encrypted_len, current_encrypted);
                log_printf("\n");
            }

            broadcast_password(current_encrypted, current_encrypted_len);
        }

        char buf[2048];
        ssize_t n = read(reg_fd, buf, sizeof(buf) - 1);
        if (n == 0) {
            close(reg_fd);
            reg_fd = open(ENCRYPTER_PIPE, O_RDONLY | O_NONBLOCK);
        } else if (n > 0) {
            buf[n] = '\0';
            char* newline = strchr(buf, '\n');
            if (newline) *newline = '\0';
            
            if (strncmp(buf, "SUBSCRIBE:", 10) == 0) {
                char* pipe_name = buf + 10;
                int id = register_decrypter(pipe_name);
                if (id > 0 && current_encrypted) {
                    send_password_to_decrypter(id - 1, current_encrypted, current_encrypted_len);
                }
            } else if (strncmp(buf, "SOLUTION:", 9) == 0) {
                char* solution_data = buf + 9;
                char* colon = strchr(solution_data, ':');
                if (colon) {
                    int decrypter_id = atoi(solution_data);
                    char* solution = colon + 1;
                    if (strlen(solution) == password_len && current_password && 
                        memcmp(solution, current_password, password_len) == 0) {
                        log_printf("%ld  [SERVER]  [OK] Password decrypted successfully by decrypter #%d\n",
                            get_timestamp(), decrypter_id);

                        free(current_password); free(current_encrypted); free(current_key);
                        current_password = NULL; current_encrypted = NULL; current_key = NULL;
                    }
                }
            }
        }

        usleep(100000);
    }

    fclose(log_file);
    return 0;
}