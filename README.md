# 🧩 Blockchain & Multithreading Linux Exercises

Welcome to a Linux-based project containing **three** separate exercises — each focusing on a different topic, implemented using C++/C, Bash scripts, shared libraries, Makefiles, and detailed documentation.

---

## 📂 Repository Structure

This repository includes three main sub-projects:

### 🔹 [ex1](./ex1) — Bitcoin Blockchain Data Processing

- An exercise that fetches, processes, and analyzes Bitcoin blockchain data.
- Includes a Bash script to download data, C++ code to process it, and a shared library (`libinfra.so`).
- Run `make` and then execute the `blockchain1.sh` script with a number of blocks to fetch data.
- Several `.cpp` files provide different features: search, display, CSV export, and more.

📄 For full instructions, usage, and examples — see [`ex1/README.md`](./ex1/README.md)

---

### 🔹 [ex2](./ex2) — Multi-threaded Password Decryption Simulation

- A simulation where a server encrypts a password and multiple client threads try to crack it.
- Implemented in C using external libraries for encryption and random data generation.
- Each round includes a server and competing client threads. The first to decrypt the password wins.
- Thread synchronization is handled using `mutex` and `condition variables`.

💡 You must install the `mta-utils-dev-x86_64.deb` package before running.

📄 For complete details, usage examples, and notes — see [`ex2/README.md`](./ex2/README.md)

#### 📸 Sample Output from ex2

These screenshots show live execution of the decryption game:

##### ✅ Clients Successfully Decrypting Password

![Clients Successfully Decrypting Password](./images/screenshot_clients_success.png)

##### ⏱ Timeout Handling by Server

![Server Timeout Screenshot](./images/screenshot_timeout_error.png)

👉 Curious to learn more?  
Dive into [`ex2/README.md`](./ex2/README.md) for the full technical breakdown!

---

### 🔹 [ex3](./ex3) — MtaCrypt (Dockerized IPC via Named Pipes)

- A containerized version of the password-encryption exercise where the **server (encrypter)** and multiple **clients (decrypters)** run in separate Docker containers.
- Inter-process communication is done through **named pipes (FIFOs)** mounted from the host (e.g., host `/tmp/mtacrypt` ↔ container `/mnt/mta`).
- The server broadcasts the current encrypted password and rotates it periodically; clients subscribe via their own FIFOs and keep trying keys until a match is found.
- The server reads configuration (e.g., password length) from `mtacrypt.conf` in the mounted directory.

💡 `mta-utils-dev-x86_64.deb` is required. You can install it manually **or** use the provided `launcher.sh` (which installs it automatically before starting the containers).

📄 For full details and run instructions — see [`ex3/README.md`](./ex3/README.md).

#### 📸 Sample Output from ex3

- **Server + multiple clients (logs):**  
  ![Server + clients logs](./images/ex3/containers-and-logs.png)

- **Client log stream (tail -f inside container):**  
  ![Client logs](./images/ex3/decrypter-logs.png)

- **FIFOs & config on host (`/tmp/mtacrypt`):**  
  ![Pipes + config](./images/ex3/pipes-and-conf.png)

---

## 🛠 General Requirements

- Linux-based OS.
- Bash and G++/GCC installed.
- Docker (for `ex3`).
- Basic terminal usage knowledge.
- It’s highly recommended to read the README in each folder before running.

---

## 🧑‍💻 Getting Started

1. Choose a sub-project (`ex1`, `ex2`, or `ex3`) based on your interest.
2. Read the relevant README file.
3. Install any necessary dependencies.
4. Run `make` to build the code.
5. Execute the relevant script/binary as instructed.

Good luck! 🚀
