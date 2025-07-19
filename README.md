# Donkey Kong Console Game

## Overview
This is a console-based recreation of the classic **Donkey Kong** game, developed in C++.
The project implements various game mechanics such as Mario movement, barrels, ghosts, ladders, and the ultimate goal of rescuing Pauline.

### How to Play
The objective of the game is to control Mario (`@`) and navigate through obstacles to rescue Pauline (`$`). Donkey Kong (`&`) throws barrels (`O`), and ghosts (`x`) roam the platforms. Use ladders (`H`) to climb and a hammer (`p`) to destroy barrels and ghosts. If you reach Pauline, you win the level!

## Features
- **ASCII-based graphics** with optional **color mode**.
- **Dynamic game mechanics** including Mario (`@`), Donkey Kong (`&`), barrels (`O`), ghosts (`x`), ladders (`H`), and Pauline (`$`).
- **Multiple stages** loaded from `.screen` files.
- **Save and load functionality** for recording and replaying games.
- **Silent mode** (`-silent`) for automated result verification.
- **Organized file structure** for better maintainability.
- **CI/CD support (planned)** for automated testing.

## Installation & Execution
### **1. Clone the repository**
```sh
git clone https://github.com/Am1its/DonkeyKongConsoleGame.git
cd DonkeyKongConsoleGame
```

### **2. Open the project in Visual Studio**
- Open `donkeykong1.sln` in **Visual Studio 2022**.
- Build the project in **Release mode**.

### **3. Run the game**
Execute the compiled binary:
```sh
dkong.exe
```
Or with command-line options:
```sh
dkong.exe -load
```
```sh
dkong.exe -save
```
```sh
dkong.exe -silent
```

## Controls
- **LEFT**  → `A` / `a`
- **RIGHT** → `D` / `d`
- **UP / JUMP** → `W` / `w`
- **DOWN** → `X` / `x`
- **STAY** → `S` / `s`
- **Use Hammer** → `P` / `p`
- **Pause** → `ESC`

## Screenshots
### Main Menu
![צילום מסך 2025-03-16 133310](https://github.com/user-attachments/assets/a6ec9de8-a9b4-4f7d-97fe-af28eef4f1b6)
![צילום מסך 2025-03-16 133521](https://github.com/user-attachments/assets/7dda0229-3117-40bf-8e00-17c8ea4f5cf4)
![צילום מסך 2025-03-16 133427](https://github.com/user-attachments/assets/73f61007-dd38-4eb1-9aba-b0d08c5fec54)

### Gameplay
![צילום מסך 2025-03-16 133436](https://github.com/user-attachments/assets/18a54895-8443-49fa-9c3d-604ab83febae)
![צילום מסך 2025-03-16 133231](https://github.com/user-attachments/assets/b7d89385-c9d9-4078-b250-be406eb69886)
![צילום מסך 2025-03-16 133125](https://github.com/user-attachments/assets/0ad113bf-31f1-44b1-9796-d30bf52ec57c)

### Game Over
![צילום מסך 2025-03-16 133410](https://github.com/user-attachments/assets/7b74294a-5cf5-49e7-867b-be3dda0a7f9e)

### Winning Screen
![צילום מסך 2025-03-16 133503](https://github.com/user-attachments/assets/8fe119f9-e412-4fe8-aadf-7afc1e762ab2)

## File Formats
The project includes:
- **`.screen` files**: Defines game levels.
- **`.steps` files**: Records movement steps.
- **`.result` files**: Stores expected results.

## Repository Enhancements
To improve the project structure and maintainability, the following updates were made:
- **Added a `.gitignore` file** to exclude compiled binaries and temporary files.
- **Organized file structure**:
  - `src/` → C++ source files.
  - `include/` → Header files.
  - `assets/` → Game assets (screens, steps, results).
  - `docs/` → Documentation and guides.
- **Planned CI/CD integration** using GitHub Actions for automated testing.
- **Added a LICENSE file (MIT)** to clarify usage permissions.
- **Included contributing guidelines (CONTRIBUTING.md)** to help new contributors.
- **Defined a Code of Conduct (CODE_OF_CONDUCT.md)** to maintain a welcoming environment.

## Contributors
- **Ziv Balassiano**
- **Amit Oved**

