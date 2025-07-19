
# 🔐 Ex2 - Multi-threaded Password Decryption Simulation

## 📘 Overview

This project simulates a **multi-threaded password decryption game** where a server encrypts a randomly generated password, and several client threads try to brute-force the key and decrypt it.

Each round includes:
- A **server thread** that generates a new password and encryption key, encrypts the password, and broadcasts it.
- Multiple **client threads** that race to decrypt the password by generating random keys.

The first thread to successfully decrypt the password wins the round.

---

## 📦 Prerequisites

Before running the program, you **must install** the external utility package `mta-utils-dev-x86_64.deb`, which provides essential functions like `MTA_get_rand_data`, `MTA_encrypt`, and `MTA_decrypt`.

To install the package, run:

```bash
sudo dpkg -i mta-utils-dev-x86_64.deb
sudo apt-get install -f    # Fix dependencies if needed

---


## ⚙️ Build Instructions

You can build the project using the provided `Makefile`:

```bash
make
```

This compiles `mta_crypto.c` into `mta_crypto.out` and links against:
- `libmta_crypt`
- `libmta_rand`
- `pthread`

To clean the build:
```bash
make clean
```

---

## 🚀 How to Run

```bash
./mta_crypto.out -n <num_of_decrypters> -l <password_length> [-t <timeout_seconds>]
```

### Flags:

| Flag                        | Description                                                                 |
| -------------------------- | --------------------------------------------------------------------------- |
| `-n`, `--num-of-decrypters`| Number of decrypter (client) threads                                        |
| `-l`, `--password-length`  | Length of the password to be generated (must be a multiple of 8)            |
| `-t`, `--timeout`          | (Optional) Timeout in seconds for each round before generating a new round  |

### Example:

```bash
./mta_crypto.out -n 3 -l 16 -t 10
```

---

## 🧵 Thread Behavior

### Server (Encrypter):
- Generates a printable password and key.
- Encrypts password using `MTA_encrypt`.
- Shares the encrypted data and waits for a valid decryption or timeout.
- On success: prints the client ID and decrypted password.
- On timeout: generates a new password.

### Client (Decrypter):
- Waits for a new encrypted password.
- Repeatedly generates random keys to try and decrypt the data.
- If a printable decryption is successful and matches the correct key:
  - Submits it to the server.
  - If correct, becomes the winner.

---

## 🔐 Synchronization

- `pthread_mutex_t mutex` — guards shared data.
- `pthread_cond_t new_data` — signals clients that a new round has started.
- `pthread_cond_t solved_cond` — used by clients to notify server of a correct decryption.
- Each thread tracks `round` numbers to avoid submitting outdated guesses.

---

## 📦 File Structure

```bash
.
├── mta_crypto.c       # Main program logic (server & client threads)
├── mta_crypt.h        # Encryption/decryption interface
├── mta_rand.h         # Random generators
└── Makefile           # Compilation script
```

---

## 🔍 Sample Output

```text
[SERVER] New password generated: P@ssw0rd, key: a1b2c3d4, After encryption: ...
[CLIENT #2] After decryption(P@ssw0rd), key guessed(a1b2c3d4), sending to server after 182300 iterations
[SERVER] Password decrypted successfully by client #2
```

---

## 🧪 Notes

- Password length must be divisible by 8 due to encryption constraints.
- Crypto operations depend on MTA-supplied libraries (`mta_crypt`, `mta_rand`).
- Threads operate concurrently and terminate only upon forced exit (e.g., `Ctrl+C`).

---

## 🇮🇱 הסבר בעברית

לפני הריצה, חובה להתקין את החבילה `mta-utils-dev-x86_64.deb`:

```bash
sudo dpkg -i mta-utils-dev-x86_64.deb
```

היא מספקת את הפונקציות `MTA_get_rand_data`, `MTA_encrypt`, `MTA_decrypt` וכו'.

אחרי התקנה – מריצים `make` כדי לקמפל את הקוד.

במטלה זו נוצר שרת שמצפין סיסמה אקראית ולקוחות (Threads) שמנסים לפצח את ההצפנה.
- השרת מייצר סיסמה ומפתח, מצפין, ושולח.
- כל לקוח מנסה לנחש את המפתח עד שמצליח לפענח סיסמה תקינה.
- הראשון שמפענח בהצלחה – זוכה, והשרת מתחיל סבב חדש.

התקשורת בין התהליכים מתבצעת באמצעות `mutex` ו־`condition variables` לשמירה על סנכרון תקין.

---
