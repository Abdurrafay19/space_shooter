<div align="center">
  <h1>🚀 Space Shooter</h1>

  <p><strong>A classic, grid-based arcade shooter built in C++ to demonstrate core procedural programming fundamentals.</strong></p>

  <p>
    <img alt="C++" src="https://img.shields.io/badge/C%2B%2B-00599C?logo=c%2B%2B&logoColor=white" />
    <img alt="SFML" src="https://img.shields.io/badge/SFML-2.5-8CC445?logo=c%2B%2B&logoColor=white" />
    <img alt="CMake" src="https://img.shields.io/badge/CMake-3.10-064F8C?logo=cmake&logoColor=white" />
    <img alt="Academic" src="https://img.shields.io/badge/Course-Programming_Fundamentals-blueviolet" />
  </p>
</div>

---

## 📖 Table of Contents
- [About the Project](#-about-the-project)
- [Programming Concepts Demonstrated](#-programming-concepts-demonstrated)
- [Key Features](#-key-features)
- [Tech Stack](#-tech-stack)
- [Getting Started](#-getting-started)
- [Controls](#-controls)
- [Academic Integrity Notice](#-academic-integrity-notice)

---

## 🚀 About the Project

**Space Shooter** was developed as the final term project for the Programming Fundamentals (PF) course. 

The goal of this project was to escape the standard terminal console and build a complete graphical application using purely procedural C++. Instead of relying on an existing game engine or advanced Object-Oriented paradigms (like classes and inheritance), the entire game state, collision detection, and entity management are driven by multi-dimensional arrays, loops, and raw mathematical logic.

### 🧠 Programming Concepts Demonstrated
This project serves as a practical implementation of fundamental computer science concepts:
* **2D Array Manipulation:** The entire game world is mapped to an `int grid[ROWS][COLS]`. Movement, collision detection, and entity spawning are calculated by manipulating indices within this multi-dimensional array.
* **Procedural Architecture:** Organized entirely via discrete functions and localized state variables. 
* **Persistent File I/O:** Utilizes `std::ifstream` and `std::ofstream` to create a save-state system. The game parses text files to read and write high scores, remaining lives, and current levels between sessions.
* **State Machines:** Manages game flow cleanly through discrete integer states (`STATE_MENU`, `STATE_PLAYING`, `STATE_BOSS`, `STATE_GAME_OVER`).
* **Delta-Time & Clocks:** Uses SFML clocks to handle movement delays, bullet fire-rates, and entity spawning independent of the frame rate.

---

### 🎯 Key Features
* **Progressive Difficulty:** 5 distinct levels. As the level increases, spawn rates accelerate and enemy movement speed increases.
* **Boss Fights:** Level 3 introduces heavily armored Boss entities that fire unpredictable projectile patterns.
* **Save & Load System:** Players can pause the game, save their exact state (lives, score, level), and resume it at a later time.
* **Power-up System:** Timed drops for Shield power-ups that grant temporary invincibility.
* **Rich Multimedia:** Full integration of background music, sound effects, and custom sprite rendering using SFML.

---

## 🛠 Tech Stack

* **Language:** Standard C++
* **Graphics & Windowing:** SFML (Simple and Fast Multimedia Library) 2.5
* **Build System:** CMake (Minimum v3.10)

---

## ⚙️ Getting Started

### Prerequisites
* A C++ compiler (GCC, Clang, or MSVC)
* CMake (v3.10 or higher)
* [SFML 2.5](https://www.sfml-dev.org/download/sfml/2.5.1/) installed and added to your system path.

### Build Instructions

**1. Clone the repository:**
```bash
git clone https://github.com/abdurrafay19/space_shooter.git
cd space_shooter

```

**2. Generate the build files:**

```bash
mkdir build
cd build
cmake ..

```

*(Note: If CMake cannot find SFML, you may need to explicitly pass your SFML directory: `cmake -DSFML_DIR="path/to/SFML/lib/cmake/SFML" ..`)*

**3. Compile the project:**

```bash
cmake --build .

```

**4. Run the application:**
Ensure your working directory contains the `assets/` folder, then execute the binary:

```bash
./sfml_project

```

---

## 🎮 Controls

* **Movement:** `Left/Right Arrows` or `A / D`
* **Shoot:** `Spacebar`
* **Pause / Menu:** `P` or `ESC`
* **Navigate Menus:** `Up/Down Arrows` or `W / S`
* **Select Option:** `ENTER`

---

## ⚠️ Academic Integrity Notice

This repository is public to showcase my personal academic progress and foundational programming skills. If you are a current university student taking a Programming Fundamentals course, please respect your institution's academic integrity and honor code policies. **Do not copy, plagiarize, or submit this code as your own term project.**
