# 🔐 ex3 — MtaCrypt (Containers + Named Pipes)

Run everything with a single script. **No Makefile required.**  
This exercise starts an **encrypter** (server) container and multiple **decrypter<i>** (client) containers that communicate via **named pipes (FIFOs)** on a shared directory.

> Screenshots live in the root README. This file focuses on **how to run and observe** ex3 quickly.

---

## ✨ What’s Inside

- **Server (encrypter)** — runs in its own container; generates & rotates encrypted passwords, validates candidates, and **broadcasts** the current encrypted password to all clients.
- **Clients (decrypters)** — each runs in a separate container named `decrypter1`, `decrypter2`, … . On startup, a client **registers** via its own FIFO, receives the current encrypted password, and keeps trying keys until it finds a match. When the server rotates to a new password, clients receive it and continue.
- **IPC** — implemented using **named pipes (FIFOs)** under the shared path **`/mnt/mta`** (mounted into every container at the same path).
- **Auto-setup** — the launcher handles prerequisites, including installing **`mta-utils-dev-x86_64.deb`** automatically before starting containers.

---

## 🧰 Prerequisites

- Linux with **Docker** and **sudo** access.
- Internet access to pull Docker images from Docker Hub.

> No manual package installation is required — the launcher installs dependencies automatically.

---

## 🚀 Quickstart

From the repository root, run the launcher with an optional number of decrypters (default **3**):
```bash
chmod +x ex3/launcher.sh
sudo ./ex3/launcher.sh          # runs with 3 decrypters
# or
sudo ./ex3/launcher.sh 5        # runs with 5 decrypters
```

**What the launcher does (in order):**
1. 📦 **Installs `mta-utils-dev-x86_64.deb` automatically** (if not already installed).
2. 🧹 Removes old FIFOs and temp files in **`/mnt/mta`**:
   ```bash
   sudo find /mnt/mta/ -type p -delete
   sudo rm -f /mnt/mta/decrypter_pipe_* /mnt/mta/server_pipe
   ```
3. 📝 Writes a fresh config file and opens permissions:
   ```bash
   echo "PASSWORD_LENGTH=24" | sudo tee /mnt/mta/mtacrypt.conf > /dev/null
   sudo chmod 666 /mnt/mta/mtacrypt.conf
   ```
4. 🔐 Sets permissive ACLs (FIFOs and dir):
   ```bash
   sudo chmod 666 /mnt/mta/*           # ignore errors if empty
   sudo find /mnt/mta -type p -exec chmod 666 {} \;
   sudo chmod 777 /mnt/mta
   ```
5. ⏹️ Stops & removes existing containers (if any):
   ```bash
   sudo docker rm -f encrypter
   for i in $(seq 1 $NUM_DECRYPTERS); do sudo docker rm -f decrypter$i; done
   ```
6. ⬇️ Pulls images from Docker Hub:
   ```bash
   sudo docker pull matangur/mta-encrypter:latest
   sudo docker pull matangur/mta-decrypter:latest
   ```
7. ▶️ Starts the **encrypter**:
   ```bash
   sudo docker run -d --name encrypter      -v /mnt/mta:/mnt/mta      -v /var/log/encrypter:/var/log      matangur/mta-encrypter:latest
   ```
8. ▶️ Starts **N decrypters**:
   ```bash
   for i in $(seq 1 $NUM_DECRYPTERS); do
     sudo docker run -d --name decrypter$i        -v /mnt/mta:/mnt/mta        -v /var/log/decrypter$i:/var/log        matangur/mta-decrypter:latest
   done
   ```
9. 🧽 (Optional) Prunes dangling images:
   ```bash
   sudo docker image prune -f
   ```

---

## 📟 Viewing Logs

You can follow logs **on the host** (recommended, since logs are written to files via the mounted `/var/log` folders), or **inside containers**.

### Option A — On the host (no docker exec)

**Server logs:**
```bash
sudo tail -F /var/log/encrypter/*.log
```

**Client logs (choose a client):**
```bash
sudo tail -F /var/log/decrypter1/*.log
# replace 1 with another index, e.g. /var/log/decrypter2/*.log
```

**All clients (iterate):**
```bash
for d in /var/log/decrypter*; do
  echo ">>> $d"
  sudo tail -n 20 "$d"/*.log
  echo
done
```

### Option B — Inside containers

**Server:**
```bash
sudo docker exec -it encrypter /bin/bash -lc 'tail -F /var/log/*.log'
```

**Single client (e.g., decrypter1):**
```bash
sudo docker exec -it decrypter1 /bin/bash -lc 'tail -F /var/log/*.log'
```

**List client containers:**
```bash
sudo docker ps --format '{{.Names}}' | grep '^decrypter'
```

> Note: `docker logs -f <name>` may be empty if the app logs to files only.

---

## ⚙️ Configuration

The server reads the config file from the shared directory at startup. The launcher **overwrites** it each run with:
```ini
# /mnt/mta/mtacrypt.conf
PASSWORD_LENGTH=24
```
To change the default, edit the line in `launcher.sh` that writes the file. Alternatively, modify the file and restart the containers.

---

## 🧪 Inspecting IPC

Check FIFOs & config on the host:
```bash
ls -l /mnt/mta
# Expect: server_pipe, decrypter_pipe_1, decrypter_pipe_2, ..., mtacrypt.conf
```

---

## 🧹 Cleanup

Remove the server and all `decrypter*` containers, and (optionally) clean images:
```bash
sudo docker rm -f encrypter 2>/dev/null || true
for c in $(sudo docker ps -a --format '{{.Names}}' | grep '^decrypter'); do
  sudo docker rm -f "$c" || true
done

# optional image prune
sudo docker image prune -f
```

---

## ✅ Summary

- **Single script**: `sudo ./ex3/launcher.sh [NUM_DECRYPTERS]`
- **Logs**: view on host under `/var/log/encrypter` and `/var/log/decrypter<i>`, or `docker exec ... tail -F /var/log/*.log`
- **Shared directory**: `/mnt/mta` for FIFOs and `mtacrypt.conf` (auto-written as `PASSWORD_LENGTH=24` by the launcher)
