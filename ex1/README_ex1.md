# 🧱 Blockchain Linux Exercise - Task 1

## 🔍 Overview

This project is a Linux-based C++ exercise that processes Bitcoin blockchain data retrieved from the BlockCypher API.  
It includes a Bash script to fetch data, a shared library for parsing and processing, and multiple executable programs to interact with the data.

---

## 📦 Structure

```
.
├── blockchain1.sh         # Bash script to download block data
├── Makefile               # For building all outputs
├── infra.h / infra.cpp    # Shared code (loaded as shared library)
├── *.cpp                  # Programs using the shared infra code
├── info.txt               # Processed block info
├── infoutput.csv          # Exported CSV data
```

---

## ⚙️ Build Instructions

To build all programs:

```bash
make
```

This will:
- Compile `libinfra.so` (shared library with all utility logic).
- Compile each `.cpp` (except `infra.cpp`) into a `.out` file, linked with `libinfra`.

To clean compiled files:
```bash
make clean
```

---

## 📥 Data Fetch Script: `blockchain1.sh`

This script fetches Bitcoin blockchain data using the BlockCypher API and extracts relevant fields into `info.txt`.

### 🔧 Usage:
```bash
./blockchain1.sh <num_of_blocks>
```

- Downloads the latest block and goes back `<num_of_blocks>` using `prev_block_url`.
- Extracts relevant lines (hash, height, total, time, relayed_by, prev_block).
- Appends the data to `info.txt`.

💡 Assumes `GET` is available in your shell (can alias `curl -s` or install `httpie`).

---

## 🚀 Programs Overview

All executables use the shared `infra` library and expect `info.txt` to exist.

### 1. `blockchain1.out`

```cpp
load_db();
print_db();
```

📄 Loads and prints all blocks (with arrows between them).

---

### 2. `blockchain2.out`

```bash
./blockchain2.out --height <height>
./blockchain2.out --hash <hash>
```

🔍 Search and print a block by:
- `--height` (e.g. `--height 123`)
- `--hash` (e.g. `--hash abcdef...`)

---

### 3. `blockchain3.out`

```cpp
load_db();
export_to_csv();
```

📤 Exports the blockchain data from `info.txt` to a CSV file: `infoutput.csv`

---

### 4. `blockchain4.out`

```cpp
load_db();
refresh_data();
```

🔄 Calls the Bash script (`blockchain1.sh`) to refresh the data using the number of currently loaded blocks.

---

### 5. `blockchain5.out` — Interactive Menu

```bash
./blockchain5.out
```

🧭 Menu interface:
```
1. print db
2. Print block by hash
3. Print block by height
4. Export data to csv
5. Refresh data
0. Exit
```

---

## 📌 Notes

- Make sure to run `blockchain1.sh` before executing programs — it generates `info.txt`.
- `libinfra.so` must be in your local directory and compiled (`make` does this).
- Exported CSV (`infoutput.csv`) is overwritten each time.

---

## 📊 Sample Output

```
hash: 000000000...
height: 836293
total: 782394818
...
    |
    v
...
```

---

## 🇮🇱 הסבר בעברית

במטלה זו עיבדנו בלוקים מתוך שרשרת הביטקוין:

- סקריפט Bash מוריד מידע על `<n>` בלוקים מה־API של BlockCypher.
- הקבצים `infra.h/infra.cpp` כוללים פונקציות לטעינה, הדפסה, חיפוש, ייצוא ו־refresh של הבלוקים.
- קובצי `.cpp` מריצים פונקציות שונות בהתאם:
  - הדפסה.
  - חיפוש לפי גובה / האש.
  - ייצוא לקובץ CSV.
  - תפריט אינטראקטיבי.
