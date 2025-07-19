#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <getopt.h>
#include <ctype.h>
#include <stdbool.h>
#include <time.h>
#include <errno.h>
#include <limits.h>
#include <sys/time.h>
#include "mta_crypt.h"
#include "mta_rand.h"
#include <openssl/evp.h>

// Shared data structure for all threads (server and clients)
typedef struct {
    char* encrypted_data;
    unsigned int encrypted_len;
    char* key;
    unsigned int key_len;
    char* original_password;
    unsigned int password_len;
    bool solved;
    char* solution;
    int winner_id;
    pthread_mutex_t mutex;
    pthread_cond_t new_data;
    pthread_cond_t solved_cond;
    unsigned int round;
} shared_t;

// Argument struct for each decrypter thread
typedef struct {
    int id;
    shared_t* shared;
} decrypter_arg_t;

// Global parameters set by command-line flags
int num_decrypters = 0;
unsigned int password_len = 0;
int timeout_sec = INT_MAX;

// Utility: get current timestamp (seconds)
long get_timestamp() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec;
}

// Utility: print buffer as hex
void print_hex(const char* buf, unsigned int len) {
    for (unsigned int i = 0; i < len; ++i)
        printf("%02x", (unsigned char)buf[i]);
}

// Utility: print buffer as printable string (dots for non-printable)
void print_str(const char* buf, unsigned int len) {
    for (unsigned int i = 0; i < len; ++i)
        printf("%c", isprint((unsigned char)buf[i]) ? buf[i] : '.');
}

// Generate a random printable password of given length
void generate_random_printable(char* buf, unsigned int len) {
    for (unsigned int i = 0; i < len; ++i) {
        char c;
        do {
            c = MTA_get_rand_char();
        } while (!isprint((unsigned char)c));
        buf[i] = c;
    }
}

// Check if all bytes in buffer are printable chars
bool is_printable_str(const char* buf, unsigned int len) {
    for (unsigned int i = 0; i < len; ++i)
        if (!isprint((unsigned char)buf[i]))
            return false;
    return true;
}

// The encrypter (server) thread: generates, encrypts, and shares passwords
void* encrypter_thread(void* arg) {
    shared_t* shared = (shared_t*)arg;
    unsigned int round = 0;

    // Initialize crypto library for this thread
    if (MTA_crypt_init() != MTA_CRYPT_RET_OK) {
        fprintf(stderr, "[SERVER]\t[ERROR] Failed to initialize crypto library!\n");
        return NULL;
    }

    while (1) {
        unsigned int key_len = password_len / 8;
        char* password = malloc(password_len);
        char* key = malloc(key_len);
        char* encrypted = malloc(password_len);
        generate_random_printable(password, password_len);
        MTA_get_rand_data(key, key_len);

        unsigned int encrypted_len = 0;
        int enc_ret = MTA_encrypt(key, key_len, password, password_len, encrypted, &encrypted_len);
        if (enc_ret != MTA_CRYPT_RET_OK) {
            fprintf(stderr, "[SERVER]\t[ERROR] Encryption failed: ret=%d\n", enc_ret);
            free(password); free(key); free(encrypted);
            sleep(1);
            continue;
        }

        // Lock and update shared data for new round
        pthread_mutex_lock(&shared->mutex);
        if (shared->encrypted_data) free(shared->encrypted_data);
        if (shared->key) free(shared->key);
        if (shared->original_password) free(shared->original_password);
        if (shared->solution) free(shared->solution);

        shared->encrypted_data = encrypted;
        shared->encrypted_len = encrypted_len;
        shared->password_len = password_len;
        shared->key = key;
        shared->key_len = key_len;
        shared->original_password = strndup(password, password_len);
        shared->solved = false;
        shared->solution = NULL;
        shared->winner_id = -1;
        shared->round = ++round;

        // Print info about new password
        printf("%ld\t[SERVER]\t[INFO] New password generated: ", get_timestamp());
        print_str(password, password_len);
        printf(", key: ");
        print_hex(key, key_len);
        printf(", After encryption: %.*s",encrypted_len,encrypted);
        printf("\n");

        pthread_cond_broadcast(&shared->new_data);
        pthread_mutex_unlock(&shared->mutex);

        // Wait for either a solution or a timeout
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_sec += timeout_sec;

        pthread_mutex_lock(&shared->mutex);
        int rc = 0;
        while (!shared->solved && rc != ETIMEDOUT) {
            rc = pthread_cond_timedwait(&shared->solved_cond, &shared->mutex, &ts);
        }
        if (shared->solved) {
            printf("%ld\t[SERVER]\t[OK] Password decrypted successfully by client #%d, received(", get_timestamp(), shared->winner_id);
            print_str(shared->solution, shared->password_len);
            printf("), is (");
            print_str(shared->original_password, shared->password_len);
            printf(")\n");
        } else {
            printf("%ld\t[SERVER]\t[ERROR] No password received during the configured timeout period (%d seconds), regenerating password\n", get_timestamp(), timeout_sec);
        }
        pthread_mutex_unlock(&shared->mutex);

        free(password);
    }
    return NULL;
}

// Each decrypter (client) thread: brute-forces the key and submits solutions
void* decrypter_thread(void* arg) {
    decrypter_arg_t* my_arg = (decrypter_arg_t*)arg;
    shared_t* shared = my_arg->shared;
    int id = my_arg->id;
    unsigned int last_round = 0;
    unsigned long iterations = 0;

    // Initialize crypto library for this thread
    if (MTA_crypt_init() != MTA_CRYPT_RET_OK) {
        fprintf(stderr, "[CLIENT #%d]\t[ERROR] Failed to initialize crypto library!\n", id);
        return NULL;
    }

    char* local_encrypted = malloc(password_len);
    unsigned int local_encrypted_len = 0;
    unsigned int local_key_len = password_len / 8;

    while (1) {
        // Wait for new round
        pthread_mutex_lock(&shared->mutex);
        while (shared->round == last_round)
            pthread_cond_wait(&shared->new_data, &shared->mutex);

        if (shared->encrypted_data && shared->encrypted_len > 0) {
            memcpy(local_encrypted, shared->encrypted_data, shared->encrypted_len);
            local_encrypted_len = shared->encrypted_len;
            local_key_len = shared->key_len;
            last_round = shared->round;
        }
        pthread_mutex_unlock(&shared->mutex);

        iterations = 0;

        // Start brute-forcing
        while (1) {
            pthread_mutex_lock(&shared->mutex);
            bool solved = shared->solved;
            unsigned int current_round = shared->round;
            pthread_mutex_unlock(&shared->mutex);

            if (solved || current_round != last_round)
                break;

            iterations++;
            char* guess_key = malloc(local_key_len);
            MTA_get_rand_data(guess_key, local_key_len);

            char* decrypted = malloc(password_len);
            unsigned int decrypted_len = 0;
            if (MTA_decrypt(guess_key, local_key_len, local_encrypted, local_encrypted_len, decrypted, &decrypted_len) == MTA_CRYPT_RET_OK) {
                if (decrypted_len == password_len && is_printable_str(decrypted, decrypted_len)) {
                    // Print info about each printable decryption attempt
                    printf("%ld\t[CLIENT #%d]\t[INFO] After decryption(", get_timestamp(), id);
                    print_str(decrypted, decrypted_len);
                    printf("), key guessed(");
                    print_hex(guess_key, local_key_len);
                    printf("), sending to server after %lu iterations\n", iterations);

                    pthread_mutex_lock(&shared->mutex);
                    // Out-of-order check
                    if (shared->round != last_round) {
                        pthread_mutex_unlock(&shared->mutex);
                        free(guess_key);
                        free(decrypted);
                        break;
                    }
                    // If correct, update shared state and notify server
                    if (!shared->solved && memcmp(guess_key, shared->key, local_key_len) == 0) {
                        shared->solved = true;
                        shared->solution = strndup(decrypted, decrypted_len);
                        shared->winner_id = id;
                        pthread_cond_signal(&shared->solved_cond);
                    } else if (!shared->solved) {
                        // Wrong password, print error
                        printf("%ld\t[SERVER]\t[ERROR] Wrong password received from client #%d(", get_timestamp(), id);
                        print_str(decrypted, decrypted_len);
                        printf("), should be (");
                        print_str(shared->original_password, shared->password_len);
                        printf(")\n");
                    }
                    pthread_mutex_unlock(&shared->mutex);
                }
            }
            free(guess_key);
            free(decrypted);
        }
    }
    free(local_encrypted);
    return NULL;
}

// Print usage message in the format required by the assignment
void print_usage(const char* prog) {
    fprintf(stderr, "Usage: %s [-t|--timeout <seconds>] <-n|--num-of-decrypters <number>> <-l|--password-length <length>>\n", prog);
}

// Parse command-line arguments and validate required flags
void parse_args(int argc, char** argv) {
    struct option long_opts[] = {
        {"num-of-decrypters", required_argument, 0, 'n'},
        {"password-length", required_argument, 0, 'l'},
        {"timeout", required_argument, 0, 't'},
        {0, 0, 0, 0}
    };
    int c;
    bool got_n = false, got_l = false;
    while ((c = getopt_long(argc, argv, "n:l:t:", long_opts, NULL)) != -1) {
        switch (c) {
            case 'n':
                num_decrypters = atoi(optarg);
                got_n = true;
                break;
            case 'l':
                password_len = atoi(optarg);
                got_l = true;
                break;
            case 't':
                timeout_sec = atoi(optarg);
                break;
            default:
                goto print_usage_label;
        }
    }
    if (!got_n) {
        fprintf(stderr, "Missing num of decrypters\n");
        goto print_usage_label;
    }
    if (!got_l) {
        fprintf(stderr, "Missing password length\n");
        goto print_usage_label;
    }
    if (password_len % 8 != 0) {
        fprintf(stderr, "Password length must be a multiple of 8!\n");
        exit(1);
    }
    return;

print_usage_label:
    print_usage(argv[0]);
    exit(1);
}

int main(int argc, char** argv) {
    parse_args(argc, argv);

    // Initialize crypto library in main thread
    if (MTA_crypt_init() != MTA_CRYPT_RET_OK) {
        fprintf(stderr, "[SERVER]\t[ERROR] Failed to initialize crypto library!\n");
        exit(EXIT_FAILURE);
    }

    // Initialize shared data and synchronization primitives
    shared_t shared = {
        .mutex = PTHREAD_MUTEX_INITIALIZER,
        .new_data = PTHREAD_COND_INITIALIZER,
        .solved_cond = PTHREAD_COND_INITIALIZER
    };

    // Start encrypter (server) thread
    pthread_t enc_thread;
    pthread_create(&enc_thread, NULL, encrypter_thread, &shared);

    // Start decrypter (client) threads
    pthread_t dec_threads[num_decrypters];
    decrypter_arg_t dec_args[num_decrypters];
    for (int i = 0; i < num_decrypters; i++) {
        dec_args[i].id = i + 1;
        dec_args[i].shared = &shared;
        pthread_create(&dec_threads[i], NULL, decrypter_thread, &dec_args[i]);
    }

    // Wait for all threads (not really reached, infinite loop)
    pthread_join(enc_thread, NULL);
    for (int i = 0; i < num_decrypters; i++)
        pthread_join(dec_threads[i], NULL);

    // Free resources (not really reached)
    free(shared.encrypted_data);
    free(shared.key);
    free(shared.original_password);
    free(shared.solution);
    return 0;
}