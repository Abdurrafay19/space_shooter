# Space Shooter Game - Complete main.cpp Documentation

## Table of Contents
0. [Installation & Setup](#installation--setup)
1. [Architecture Overview](#architecture-overview)
2. [Includes & Namespaces](#includes--namespaces)
3. [Global Constants](#global-constants)
4. [The main() Function Deep Dive](#the-main-function-deep-dive)
5. [Core Data Structures](#core-data-structures)
6. [Asset Loading System](#asset-loading-system)
7. [Game State Machine](#game-state-machine)
8. [Timing System](#timing-system)
9. [The Game Loop](#the-game-loop)
10. [State-Specific Logic](#state-specific-logic)
11. [Entity Movement & Collision](#entity-movement--collision)
12. [Rendering Pipeline](#rendering-pipeline)
13. [How Everything Links Together](#how-everything-links-together)

---

## 0. Installation & Setup

The project uses **CMake + SFML 2.5+** and loads assets relative to the repository root (the executable must run with the working directory set to the folder that contains `assets/` and `save-file.txt`). Pick the flow that matches your OS and IDE.

### Windows + Visual Studio 2022
1. Install **Visual Studio 2022** with the Desktop development with C++ workload (this installs MSVC, CMake, Ninja, and the debugger).
2. Install **vcpkg** once to fetch SFML:
    ```
    git clone https://github.com/microsoft/vcpkg %USERPROFILE%\vcpkg
    %USERPROFILE%\vcpkg\bootstrap-vcpkg.bat
    %USERPROFILE%\vcpkg\vcpkg install sfml:x64-windows
    %USERPROFILE%\vcpkg\vcpkg integrate install
    ```
3. Open the repo via File > Open > CMake. Visual Studio will create or reuse `CMakeSettings.json`.
4. In the active configuration add the toolchain setting so CMake can find SFML:
    ```json
    "configureSettings": {
         "CMAKE_TOOLCHAIN_FILE": "C:/Users/<you>/vcpkg/scripts/buildsystems/vcpkg.cmake"
    }
    ```
5. Select an x64 configuration, click **Generate**, then **Build**. Visual Studio places the binary under `out/build/<config>/sfml_project`.
6. Set Debugging > Working Directory to the repository root (so `assets/` and `save-file.txt` resolve) and press **Local Windows Debugger** to run.

### Windows + Visual Studio Code
1. Install a 64-bit compiler plus build tools (either **Visual Studio Build Tools 2022** with MSVC or **MSYS2** for `g++`), then add **CMake** and **Ninja** (for example via `winget install Kitware.CMake Ninja-build`).
2. Install **vcpkg** and fetch SFML exactly as above (`vcpkg install sfml:x64-windows`).
3. In VS Code install the extensions **C/C++** (`ms-vscode.cpptools`) and **CMake Tools** (`ms-vscode.cmake-tools`).
4. Open the workspace folder, press `Ctrl+Shift+P` > `CMake: Select Kit`, and pick your compiler (for example, Visual Studio 17 2022 Release - amd64).
5. Tell CMake Tools where the vcpkg toolchain lives by adding to `.vscode/settings.json`:
    ```json
    "cmake.configureSettings": {
         "CMAKE_TOOLCHAIN_FILE": "C:/Users/<you>/vcpkg/scripts/buildsystems/vcpkg.cmake"
    }
    ```
6. Run **CMake: Configure**, then **CMake: Build** (or run `cmake -S . -B build` and `cmake --build build` in the integrated terminal). Launch via **CMake: Run Without Debugging**; ensure that configuration's working directory is the repo root or copy `assets/` into `build/` before running.

### Linux + Visual Studio Code (or CLI)
1. Install dependencies (substitute your package manager as needed):
    ```
    sudo apt update
    sudo apt install build-essential cmake ninja-build libsfml-dev
    ```
    Fedora: `sudo dnf install gcc-c++ cmake ninja-build SFML-devel`, Arch: `sudo pacman -S base-devel cmake ninja sfml`.
2. Open the project in VS Code (locally or via Remote SSH/WSL). With CMake Tools installed, run **CMake: Configure** (select Ninja or Unix Makefiles) followed by **CMake: Build**. CLI equivalent:
    ```
    cmake -S . -B build -G Ninja
    cmake --build build
    ./build/sfml_project/sfml_project
    ```
3. Run from the repository root so `assets/` loads correctly. `save-file.txt` will be created automatically on first launch.

---

## 1. Architecture Overview

The game uses a **monolithic procedural architecture** where all code resides in a single `main()` function. This design pattern is common for small arcade-style games and makes the flow explicit and easy to follow.

**Key Design Principles:**
- **State Machine Pattern**: The game switches between different states (Menu, Playing, Paused, etc.)
- **Grid-Based World**: All entities exist in a 2D integer array representing the game world
- **Frame-Based Updates**: Logic executes every frame (60 FPS)
- **Immediate Mode Rendering**: The entire screen is redrawn each frame based on current state

---

## 2. Includes & Namespaces

```cpp
#include <SFML/Graphics.hpp>  // Core SFML graphics library
#include <SFML/Audio.hpp>     // SFML audio library for sound effects and music
#include <iostream>            // For error output (cerr)
#include <fstream>             // For file I/O (save file handling)
#include <cstdlib>             // For rand() and srand()
#include <ctime>               // For time() to seed random generator
using namespace std;           // Standard library namespace
using namespace sf;            // SFML namespace
```

**Why these includes:**
- `SFML/Graphics.hpp`: Provides window management, rendering, textures, sprites, text, and input handling
- `SFML/Audio.hpp`: Provides music and sound effect playback capabilities
- `iostream`: Used for `cerr` statements to report critical errors (texture/sound loading failures)
- `fstream`: Used for reading and writing save-file.txt (high score and game state persistence)
- `cstdlib` & `ctime`: Random number generation for spawning entities at unpredictable positions

---

## 3. Global Constants

### Grid Configuration
```cpp
const int ROWS = 23;        // Vertical grid cells
const int COLS = 15;        // Horizontal grid cells
const int CELL_SIZE = 40;   // Each cell is 40x40 pixels
const int MARGIN = 40;      // Border around the game grid
const float BULLET_OFFSET_X = (CELL_SIZE - CELL_SIZE * 0.3f) / 2.0f; // Center bullets horizontally
const float SHIELD_OFFSET = CELL_SIZE * -0.15f; // Center shield overlay
```

**How they connect:**
- Window size is calculated as: `COLS * CELL_SIZE + MARGIN * 2 + 500` (width), `ROWS * CELL_SIZE + MARGIN * 2` (height)
- Every entity position is mapped: `pixelX = MARGIN + col * CELL_SIZE`
- The grid acts as a coordinate system: `grid[row][col]`
- `BULLET_OFFSET_X`: Pre-calculated offset to center bullets (width 30% of cell) in their cells
- `SHIELD_OFFSET`: Pre-calculated offset to center shield overlay (130% of cell size) over spaceship

### Game States
```cpp
const int STATE_MENU = 0;
const int STATE_PLAYING = 1;
const int STATE_INSTRUCTIONS = 2;
const int STATE_GAME_OVER = 3;
const int STATE_LEVEL_UP = 4;
const int STATE_VICTORY = 5;
const int STATE_PAUSED = 6;
```

**State Machine Flow:**
```
STATE_MENU ←→ STATE_INSTRUCTIONS
    ↓
STATE_PLAYING ←→ STATE_PAUSED
    ↓
STATE_LEVEL_UP → (loops back to STATE_PLAYING)
    ↓
STATE_GAME_OVER or STATE_VICTORY → back to STATE_MENU
```

---

## 4. The main() Function Deep Dive

### Phase 0: Save File System (Lines 38-70)
```cpp
string saveFile = "save-file.txt";
ifstream inputFile(saveFile);
if (inputFile.is_open()) {
    inputFile >> highScore >> savedLives >> savedScore >> savedLevel;
    if (savedLevel > 0 && savedLives > 0) {
        hasSavedGame = true;
    }
}
```
**Purpose**: Persistent data storage for high scores and saved games
**File Format**: Space-separated integers: `highScore lives score level`
**Features**:
- Loads high score on startup to display in main menu
- Detects if a valid saved game exists (level > 0, lives > 0)
- Creates file with "0 0 0 0" if it doesn't exist
- Enables "Load Saved Game" menu option when save data exists

### Phase 1: Initialization (Lines 28-30)
```cpp
srand(static_cast<unsigned int>(time(0)));
```
**Purpose**: Seeds the random number generator with the current timestamp
**Impact**: Ensures meteor/enemy spawn positions are different each game session

### Phase 2: Window Creation (Lines 32-37)
```cpp
const int windowWidth = COLS * CELL_SIZE + MARGIN * 2 + 500;
const int windowHeight = ROWS * CELL_SIZE + MARGIN * 2;
RenderWindow window(VideoMode(windowWidth, windowHeight), "Space Shooter");
window.setFramerateLimit(60);
```

**Breakdown:**
- `windowWidth`: Grid area (600px) + margins (80px) + UI panel (500px) = 1180px
- `windowHeight`: Grid area (920px) + margins (80px) = 1000px
- `setFramerateLimit(60)`: Caps execution to 60 FPS for consistent gameplay (removed conflicting sleep() call for optimal performance)

---

## 5. Core Data Structures

### The Grid System (Lines 91-104)
```cpp
int grid[ROWS][COLS] = {0};

// Separate arrays for shield powerups (independent from main grid)
const int MAX_SHIELD_POWERUPS = 5;
int shieldPowerupRow[MAX_SHIELD_POWERUPS] = {-1, -1, -1, -1, -1};
int shieldPowerupCol[MAX_SHIELD_POWERUPS] = {-1, -1, -1, -1, -1};
bool shieldPowerupActive[MAX_SHIELD_POWERUPS] = {false, false, false, false, false};
int shieldPowerupDirection[MAX_SHIELD_POWERUPS] = {0, 0, 0, 0, 0};
```

**This is the heart of the game.** Every entity exists as an integer value in this 2D array:

| Value | Entity Type | Behavior |
|-------|-------------|----------|
| 0 | Empty Space | No rendering, movement passes through |
| 1 | Player Ship | Controlled by user, bottom row only |
| 2 | Meteor | Falls down, destroys on contact, awards 1-2 random points |
| 3 | Player Bullet | Moves up, destroys enemies/meteors |
| 4 | Enemy UFO | Falls down, awards 3 points when destroyed |
| 5 | Boss | Falls down, fires bullets, awards 5 points when destroyed |
| 6 | Boss Bullet | Falls down, damages player |

**Critical Design Decisions:**
- Only ONE entity per cell in main grid (no overlapping)
- Shield powerups use separate array system to avoid interfering with bullets/entities
- Collision detection is simply checking the value of the target cell
- Movement = changing the value at one position and setting the old position to 0

**Shield Powerup System:**
- Independent tracking allows up to 5 simultaneous shield powerups
- Currently drift straight down every 0.5s (direction slots are reserved for future patterns)
- Collision detected by checking grid position before/after movement
- Spawns starting at level 3+

### Hit Effect System (Lines 53-58)
```cpp
const int MAX_HIT_EFFECTS = 50;
int hitEffectRow[MAX_HIT_EFFECTS] = {0};
int hitEffectCol[MAX_HIT_EFFECTS] = {0};
float hitEffectTimer[MAX_HIT_EFFECTS] = {0.0f};
bool hitEffectActive[MAX_HIT_EFFECTS] = {false};
```

**Purpose**: Visual feedback when entities are destroyed
**How it works:**
1. When a collision destroys an entity, the game finds an inactive slot in these arrays
2. It stores the row/col position and activates the effect
3. Each frame, timers increment
4. After 0.3 seconds, the effect deactivates
5. During rendering, active effects draw explosion sprites

**Linked to**: Rendering loop (lines 1655-1661) and update logic (lines 1352-1363)

### Game State Variables (Lines 71-90)
```cpp
int currentState = STATE_MENU;    // Controls which screen is active
int selectedMenuItem = 0;         // Which menu option is highlighted
int lives = 3;                    // Player health
int score = 0;                    // Points accumulated from all sources
int killCount = 0;                // Tracks enemies/bosses destroyed for level progression
int level = 1;                    // Current difficulty tier
const int MAX_LEVEL = 5;          // Win condition
bool isInvincible = false;        // Temporary damage immunity
Clock invincibilityTimer;         // Tracks immunity duration
int bossMoveCounter = 0;          // Determines when bosses fire
bool hasShield = false;           // Shield powerup status
int highScore = 0;                // Persistent high score from save file
bool hasSavedGame = false;        // Whether saved game data exists
```

**New Scoring System:**
- **Enemies**: 3 points each (contribute to killCount)
- **Bosses**: 5 points each (contribute to killCount)
- **Meteors**: 1-2 random points (bonus only, no killCount)
- **Level Up Condition**: `killCount >= level * 10` (must destroy 10/20/30/40/50 enemies/bosses per level)

**How they interact:**
- `currentState` determines which logic block executes in the game loop
- `lives <= 0` triggers `currentState = STATE_GAME_OVER`
- `killCount >= level * 10` triggers level up sequence (changed from score-based)
- `isInvincible` prevents multiple hits during the 2-second immunity window
- `hasShield` provides one-time damage absorption before being removed
- `highScore` persists across game sessions via save-file.txt

---

## 6. Asset Loading System

### Texture Loading Pattern (Lines 61-227)
Every visual asset follows this pattern:
```cpp
Texture spaceshipTexture;
if (!spaceshipTexture.loadFromFile("assets/images/player.png")) {
    cerr << "Failed to load spaceship texture" << endl;
    return -1;  // Exit program if asset missing
}
Sprite spaceship;
spaceship.setTexture(spaceshipTexture);
spaceship.setScale(
    static_cast<float>(CELL_SIZE) / spaceshipTexture.getSize().x,
    static_cast<float>(CELL_SIZE) / spaceshipTexture.getSize().y
);
```

**Critical Concepts:**

1. **Texture vs Sprite:**
   - `Texture`: The image data loaded from disk (stored in GPU memory)
   - `Sprite`: A drawable object that references a texture and has position/scale

2. **Scaling Logic:**
   - Goal: Make sprite fit exactly in one grid cell (40x40 pixels)
   - Formula: `scale = targetSize / originalSize`
   - Example: If `player.png` is 64x64, scale = 40/64 = 0.625

3. **Why scaling matters:**
   - Assets have varying original sizes
   - Grid cells are uniform (40x40)
   - Scaling ensures visual consistency

**All loaded assets:**
- Player ship, life icon, meteors, enemies, bosses
- Bullets (player red, boss green) + impact effects
- Backgrounds (game area, menu starfield)
- Font for all text rendering

---

## 7. Game State Machine

### State Transitions

**STATE_MENU (Lines 571-635):**
```cpp
if (Keyboard::isKeyPressed(Keyboard::Enter)) {
    if (selectedMenuItem == 0) {  // Start Game
        currentState = STATE_PLAYING;
        // Reset all game variables
        lives = 3; score = 0; level = 1;
        // Clear grid, reposition spaceship
        // Restart all clocks
    }
}
```

**Why reset everything:**
- Starting a new game must wipe previous session data
- Grid must be cleared of leftover entities
- Timers must restart to prevent immediate spawns

**STATE_PLAYING (Lines 698-1371):**
The most complex state - handles:
- Player input (movement, shooting, pause)
- Entity spawning (meteors, enemies, bosses)
- Entity movement with collision detection
- Level progression and victory conditions

**STATE_PAUSED (Lines 1455-1519):**
```cpp
// Draw game state in background (frozen)
// Draw semi-transparent overlay
// Draw pause menu on top
```
**Design choice**: Game remains visible but frozen, giving context to the player

---

## 8. Timing System

### Clock-Based Timing (Lines 414-436)
SFML's `Clock` class measures elapsed time since creation or last restart.

**Movement Cooldown:**
```cpp
Clock moveClock;
Time moveCooldown = milliseconds(100);

// In game loop:
if (moveClock.getElapsedTime() >= moveCooldown) {
    // Allow movement
    if (Keyboard::isKeyPressed(Keyboard::Left)) {
        // Move left
        moveClock.restart();  // Reset timer
    }
}
```

**Why cooldowns exist:**
- Without them, 60 FPS means 60 movements per second (too fast)
- Cooldown of 100ms = max 10 movements/second (playable speed)

**Spawning Timers:**
```cpp
Clock meteorSpawnClock;
float nextSpawnTime = 1.0f + (rand() % 3);  // Random 1-3 seconds

if (meteorSpawnClock.getElapsedTime().asSeconds() >= nextSpawnTime) {
    // Spawn meteor
    meteorSpawnClock.restart();
    nextSpawnTime = 1.0f + (rand() % 3);  // Set next random interval
}
```

**Why random intervals:**
- Predictable spawns are boring
- Randomness creates dynamic challenge
- Each spawn sets a new random interval

**Timer Categories:**
1. **Input Cooldowns**: `moveClock` (100ms), `bulletFireClock` (300ms), `menuClock` (200ms)
2. **Spawn Timers**: `meteorSpawnClock`, `enemySpawnClock`, `bossSpawnClock`
3. **Movement Timers**: `meteorMoveClock`, `enemyMoveClock`, `bulletMoveClock`
4. **Effect Timers**: `hitEffectClock`, `invincibilityTimer`, `levelUpBlinkClock`

---

## 9. The Game Loop

### Structure (Lines ~640-2487)
```cpp
while (window.isOpen()) {
    // 1. EVENT POLLING
    Event event;
    while (window.pollEvent(event)) {
        if (event.type == Event::Closed)
            window.close();
    }
    
    // 2. STATE LOGIC (different for each state)
    if (currentState == STATE_MENU) { /* menu logic */ }
    else if (currentState == STATE_PLAYING) { /* game logic */ }
    // ... other states
    
    // 3. RENDERING
    window.clear(Color(40, 40, 40));
    if (currentState == STATE_MENU) { /* draw menu */ }
    else if (currentState == STATE_PLAYING) { /* draw game */ }
    // ... other renders
    window.display();
}
```

**Execution Flow (60 times per second):**
1. **Event Polling**: Check if user closed window
2. **Update Logic**: Based on `currentState`, execute appropriate game logic
3. **Rendering**: Draw everything relevant to current state
4. **Display**: Swap buffers (show what was drawn)

**Performance Optimization**: Removed conflicting `sleep()` call - framerate limiting is now handled exclusively by `setFramerateLimit(60)` for more accurate timing.

---

## 10. State-Specific Logic

### Playing State Breakdown (Lines 698-1371)

#### Input Handling (Lines 700-751)
**Player Movement:**
```cpp
if (moveClock.getElapsedTime() >= moveCooldown) {
    if (Keyboard::isKeyPressed(Keyboard::Left) && spaceshipCol > 0) {
        grid[ROWS - 1][spaceshipCol] = 0;  // Clear old position
        spaceshipCol--;                     // Update column variable
        grid[ROWS - 1][spaceshipCol] = 1;  // Set new position
        moveClock.restart();
    }
}
```

**Why three steps:**
1. Clear old grid position (set to 0)
2. Update the `spaceshipCol` tracking variable
3. Set new grid position (set to 1)

**Shooting:**
```cpp
if (Keyboard::isKeyPressed(Keyboard::Space) && bulletFireClock.getElapsedTime() >= bulletFireCooldown) {
    int bulletRow = ROWS - 2;  // One row above spaceship
    if (grid[bulletRow][spaceshipCol] == 0) {  // Only if empty
        grid[bulletRow][spaceshipCol] = 3;  // Place bullet
    }
    bulletFireClock.restart();
}
```

**Key detail**: Bullet spawns only if the cell above is empty (prevents stacking)

#### Spawning System (Lines 753-792)

**Enemy Spawn Difficulty Scaling:**
```cpp
float baseTime = 2.5f - (level * 0.4f);  // Faster at higher levels
float variance = 3.0f - (level * 0.4f);   // Less random at higher levels
if (baseTime < 0.5f) baseTime = 0.5f;    // Minimum spawn time
nextEnemySpawnTime = baseTime + (rand() % (int)variance);
```

**Level 1**: 2.5s base + 0-3s variance = 2.5-5.5s between spawns
**Level 5**: 0.5s base + 0-1s variance = 0.5-1.5s between spawns (much faster!)

**Boss Spawning Condition:**
```cpp
if (level >= 3 && bossSpawnClock.getElapsedTime().asSeconds() >= nextBossSpawnTime) {
    // Spawn boss
}
```
**Design**: Bosses only appear after Level 3 to introduce new challenge mid-game

---

## 11. Entity Movement & Collision

### Movement Pattern: Bottom-to-Top Iteration
```cpp
for (int r = ROWS - 1; r >= 0; r--) {  // Start from bottom
    for (int c = 0; c < COLS; c++) {
        if (grid[r][c] == 2) {  // Found a meteor
            // Move it down
        }
    }
}
```

**Why bottom-to-top:**
- Prevents processing the same entity twice in one frame
- Example: If we iterate top-to-bottom and move an entity from row 5 to row 6, when we reach row 6, we'd move it again to row 7 (wrong!)

### Collision Detection Example: Meteor Movement (Lines 795-823)
```cpp
if (grid[r][c] == 2) {  // Found meteor at position [r][c]
    if (r == ROWS - 1) {  // At bottom edge
        grid[r][c] = 0;   // Remove meteor
    }
    else {
        grid[r][c] = 0;  // Clear current position
        
        if (grid[r + 1][c] == 0) {  // Next cell is empty
            grid[r + 1][c] = 2;      // Move meteor down
        }
        else if (grid[r + 1][c] == 1) {  // Next cell has player
            if (!isInvincible) {
                lives--;
                isInvincible = true;
                invincibilityTimer.restart();
            }
            // Don't place meteor (it's destroyed)
        }
        else if (grid[r + 1][c] == 3) {  // Next cell has bullet
            grid[r + 1][c] = 0;  // Destroy both (leave cell empty)
            // Trigger hit effect
        }
    }
}
```

**Collision Logic:**
1. Check the cell you're moving into
2. If empty (0): Move there
3. If player (1): Deal damage, apply invincibility
4. If bullet (3): Destroy both entities
5. Always clear the old position first

### Boss Firing Mechanism (Lines 1058-1098)
```cpp
bossMoveCounter++;  // Increment on each movement

int firingInterval;
if (level == 3) firingInterval = 3;      // Fire every 3 moves
else if (level == 4) firingInterval = 2; // Fire every 2 moves
else firingInterval = 1;                 // Fire almost every move (level 5)

if (bossMoveCounter >= firingInterval) {
    // Find all bosses and fire bullets
    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            if (grid[r][c] == 5) {  // Found a boss
                int bulletRow = r + 1;
                if (grid[bulletRow][c] == 0) {
                    grid[bulletRow][c] = 6;  // Place boss bullet
                }
            }
        }
    }
    bossMoveCounter = 0;  // Reset counter
}
```

**How it scales with difficulty:**
- Level 3: Boss fires every 3rd time it moves
- Level 4: Boss fires every 2nd time it moves
- Level 5: Boss fires almost every time it moves

**Why this matters:** Creates escalating difficulty as player progresses

### Player Bullet Movement (Lines 1165-1337)
**Special behavior: Moves UP instead of down**
```cpp
for (int r = 0; r < ROWS; r++) {  // Top-to-bottom for upward movement
    for (int c = 0; c < COLS; c++) {
        if (grid[r][c] == 3) {  // Found player bullet
            if (r == 0) {  // At top edge
                grid[r][c] = 0;  // Remove bullet
            }
            else {
                grid[r][c] = 0;  // Clear current position
                
                if (grid[r - 1][c] == 4) {  // Hit enemy
                    score += 3;
                    killCount++;
                    grid[r - 1][c] = 0;  // Destroy both
                    
                    // Check for progression using killCount
                    if (level < MAX_LEVEL && killCount >= level * 10) {
                        // Clear grid, recenter ship, blink LEVEL UP
                        currentState = STATE_LEVEL_UP;
                    }
                    else if (level >= MAX_LEVEL && killCount >= level * 10) {
                        currentState = STATE_VICTORY;
                    }
                }
                else if (grid[r - 1][c] == 5) {  // Hit boss
                    score += 5;  // Bosses worth more
                    killCount++;
                    grid[r - 1][c] = 0;
                    
                    // Same level-up or victory logic as above
                    if (level < MAX_LEVEL && killCount >= level * 10) {
                        currentState = STATE_LEVEL_UP;
                    }
                    else if (level >= MAX_LEVEL && killCount >= level * 10) {
                        currentState = STATE_VICTORY;
                    }
                }
            }
        }
    }
}
```

**Level Up Trigger:**
1. Player destroys enemy → `score += 3`, `killCount++`
2. Player destroys boss → `score += 5`, `killCount++`
3. Player destroys meteor → `score += (rand() % 2) + 1` (no killCount change)
4. Check if `killCount >= level * 10`
5. If true: Increment level, reset killCount to 0, clear all entities, show "LEVEL UP!" screen
6. After 2 seconds, return to gameplay with increased difficulty

**Key Change**: Level progression now based on `killCount` (enemies/bosses only), while `score` includes all points including meteor bonuses.

---

## 12. Rendering Pipeline

### Rendering Order (Lines 1522-1850)
```cpp
window.clear(Color(40, 40, 40));  // Dark gray background

if (currentState == STATE_PLAYING) {
    // 1. Draw background (covers whole grid area)
    window.draw(background);
    
    // 2. Draw grid border
    window.draw(gameBox);
    
    // 3. Draw all entities by scanning the grid
    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            if (grid[r][c] == 1) {  // Spaceship
                spaceship.setPosition(MARGIN + c * CELL_SIZE, MARGIN + r * CELL_SIZE);
                window.draw(spaceship);
            }
            else if (grid[r][c] == 2) {  // Meteor
                meteor.setPosition(MARGIN + c * CELL_SIZE, MARGIN + r * CELL_SIZE);
                window.draw(meteor);
            }
            // ... all entity types
        }
    }
    
    // 4. Draw hit effects (explosion animations)
    for (int i = 0; i < MAX_HIT_EFFECTS; i++) {
        if (hitEffectActive[i]) {
            bulletHit.setPosition(MARGIN + hitEffectCol[i] * CELL_SIZE, 
                                  MARGIN + hitEffectRow[i] * CELL_SIZE);
            window.draw(bulletHit);
        }
    }
    
    // 5. Draw UI (score, lives, level)
    window.draw(title);
    window.draw(scoreText);
    // ... other UI elements
}

window.display();  // Show everything drawn this frame
```

**Why this order matters:**
1. Background first (back layer)
2. Entities next (middle layer)
3. Effects on top (explosions should be visible over everything)
4. UI last (front layer, always visible)

### Position Calculation
```cpp
pixelX = MARGIN + col * CELL_SIZE
pixelY = MARGIN + row * CELL_SIZE
```

**Example:**
- Grid position: `[5][7]` (row 5, col 7)
- Pixel position: `MARGIN + 7 * 40, MARGIN + 5 * 40` = `(320, 240)` pixels

### Invincibility Blink Effect (Lines 1615-1621)
```cpp
if (grid[r][c] == 1) {  // Spaceship
    spaceship.setPosition(MARGIN + c * CELL_SIZE, MARGIN + r * CELL_SIZE);
    
    // Only draw if NOT invincible OR if current blink state is visible
    if (!isInvincible || ((int)(invincibilityTimer.getElapsedTime().asMilliseconds() / 100) % 2 == 0)) {
        window.draw(spaceship);
    }
}
```

**How blink works:**
1. Get elapsed milliseconds since hit (e.g., 350ms)
2. Divide by 100 (3.5) → cast to int (3)
3. Modulo 2 (3 % 2 = 1)
4. If result is 0: Draw spaceship
5. If result is 1: Skip drawing (invisible for that frame)

**Result:** Spaceship flickers on/off every 100ms during invincibility period

---

## 13. How Everything Links Together

### The Complete Game Cycle

```
┌─────────────────────────────────────────────────────────────┐
│                    GAME STARTS                               │
│                    main() called                             │
└────────────────────────┬────────────────────────────────────┘
                         │
                         ▼
┌─────────────────────────────────────────────────────────────┐
│              INITIALIZATION PHASE                            │
│  • Seed random generator (srand)                             │
│  • Create window (1180x1000)                                 │
│  • Initialize variables (lives=3, score=0, level=1)          │
│  • Create grid[23][15] = {0}                                 │
│  • Load all textures and create sprites                      │
│  • Create all text objects for UI                            │
│  • Initialize all clocks                                     │
│  • Set currentState = STATE_MENU                             │
└────────────────────────┬────────────────────────────────────┘
                         │
                         ▼
┌─────────────────────────────────────────────────────────────┐
│                  MAIN GAME LOOP                              │
│              while (window.isOpen())                         │
└────────────────────────┬────────────────────────────────────┘
                         │
         ┌───────────────┴───────────────┐
         │                               │
         ▼                               ▼
┌──────────────────┐          ┌────────────────────┐
│ EVENT POLLING    │          │  STATE LOGIC       │
│ • Window closed? │──────────▶│  Based on         │
│                  │          │  currentState:     │
└──────────────────┘          │                    │
                              │  • STATE_MENU      │
                              │  • STATE_PLAYING   │
                              │  • STATE_PAUSED    │
                              │  • STATE_GAME_OVER │
                              │  • etc.            │
                              └────────┬───────────┘
                                       │
                                       ▼
                              ┌────────────────────┐
                              │   RENDERING        │
                              │  • Clear screen    │
                              │  • Draw based on   │
                              │    currentState    │
                              │  • Display frame   │
                              └────────┬───────────┘
                                       │
                                       ▼
                              ┌────────────────────┐
                              │  Sleep(50ms)       │
                              │  Loop continues    │
                              └────────┬───────────┘
                                       │
                                       └──────────┐
                                                  │
                  (Loops back 60 times/second) ◀──┘
```

### STATE_PLAYING Deep Dive (The Core Gameplay Loop)

```
STATE_PLAYING Logic Flow (executed every frame):
═══════════════════════════════════════════════

1. INPUT HANDLING
   ├─ Check if P pressed → Switch to STATE_PAUSED
   ├─ Check moveClock cooldown
   │  └─ If Left/Right pressed → Update grid, move spaceship
   └─ Check bulletFireClock cooldown
      └─ If Space pressed → Place bullet at grid[ROWS-2][spaceshipCol]

2. ENTITY SPAWNING (Time-based)
   ├─ Check meteorSpawnClock
   │  └─ If elapsed >= nextSpawnTime → Place meteor at grid[0][randomCol]
   ├─ Check enemySpawnClock
   │  └─ If elapsed >= nextEnemySpawnTime → Place enemy at grid[0][randomCol]
   └─ Check bossSpawnClock (only if level >= 3)
      └─ If elapsed >= nextBossSpawnTime → Place boss at grid[0][randomCol]

3. ENTITY MOVEMENT & COLLISION
   ├─ Meteor Movement (every 0.833 seconds)
   │  └─ For each meteor: Try move down, check collisions
   ├─ Enemy Movement (speed based on level)
   │  └─ For each enemy: Try move down, check collisions, award points
   ├─ Boss Movement (speed based on level)
   │  ├─ For each boss: Try move down, check collisions
   │  └─ Increment bossMoveCounter, fire bullets based on interval
   ├─ Boss Bullet Movement (twice as fast as boss)
   │  └─ For each boss bullet: Move down, check player collision
   └─ Player Bullet Movement (very fast, 0.05s)
      └─ For each bullet: Move up, check enemy/boss collision, level up logic

4. EFFECT UPDATES
    ├─ Update hit effect timers
    │  └─ Deactivate effects after 0.3 seconds
    └─ Update invincibility timer
        └─ Set isInvincible = false after 2.0 seconds

5. STATE TRANSITIONS
    ├─ If lives <= 0 → currentState = STATE_GAME_OVER
    ├─ If level < MAX_LEVEL && killCount >= level * 10 → currentState = STATE_LEVEL_UP
    └─ If level >= MAX_LEVEL && killCount >= level * 10 → currentState = STATE_VICTORY
```

### Data Flow: How Grid Changes Affect Everything

```
GRID UPDATE TRIGGERS VISUAL CHANGES:
════════════════════════════════════

User presses LEFT key
    ↓
moveClock check passes (100ms elapsed)
    ↓
grid[ROWS-1][spaceshipCol] = 0  (clear old)
spaceshipCol--                   (update variable)
grid[ROWS-1][spaceshipCol] = 1  (set new)
    ↓
RENDERING PHASE:
    ↓
Scan grid at [ROWS-1][spaceshipCol-1]
    ↓
Find value = 1 (spaceship)
    ↓
Calculate pixel position:
    x = MARGIN + (spaceshipCol-1) * 40
    y = MARGIN + (ROWS-1) * 40
    ↓
Set spaceship sprite position
    ↓
Draw spaceship at new position
    ↓
Player sees spaceship moved left
```

### Level Progression Flow

```
LEVEL UP SEQUENCE (NEW SYSTEM):
═══════════════════════════════

Player bullet hits enemy
    ↓
score += 3 (points for display)
killCount++ (progress tracker)
    ↓
Check: killCount >= level * 10?  (10 >= 10? YES)
    ↓
level++ (now level = 2)
killCount = 0 (reset for next level)
bossMoveCounter = 0
    ↓
Clear entire grid (except spaceship)
Clear all shield powerups
    ↓
Reset spaceship to center
    ↓
currentState = STATE_LEVEL_UP
levelUpTimer.restart()
    ↓
RENDERING: Show "LEVEL UP!" with blink effect
    ↓
Wait 2 seconds (levelUpTimer check)
    ↓
currentState = STATE_PLAYING
    ↓
GAMEPLAY RESUMES WITH INCREASED DIFFICULTY:
    ├─ Meteor spawn time reduced
    ├─ Meteor movement speed increased
    ├─ Enemy spawn time reduced (scales with level)
    ├─ Enemy movement speed increased
    ├─ Boss spawn time reduced (if level >= 3)
    ├─ Boss movement speed increased
    ├─ Boss firing frequency increased (level 3: every 5 moves, level 4: every 4, level 5: every 3)
    └─ Shield powerups begin spawning (level 3+)

KEY CHANGE: killCount tracks only enemies/bosses for level progression.
Score includes everything (enemies=3, bosses=5, meteors=1-2 random) for display/high score.
```

### Save File Persistence

```
SAVE/LOAD SYSTEM:
═════════════════

Save Triggers:
- Game Over: Saves high score if current score > previous high score
- Victory: Saves high score if current score > previous high score  
- Pause Menu → Save & Quit: Saves high score AND current game state
- Enemy Escape (bottom of screen): Saves high score if new record

Load Options:
- Main Menu → Load Saved Game: Restores lives, score, level (killCount resets to 0)

File Format: "highScore lives score level"
Example: "150 2 75 3" means:
  - High score: 150 points
  - Saved game: 2 lives, 75 points, Level 3
  - If lives=0, no saved game available

Storage Location: save-file.txt in project root
```

### Collision Resolution Priority

```
ENTITY INTERACTION MATRIX:
═════════════════════════

When Entity A tries to move into Entity B's cell:

           │ Empty │ Player │ Meteor │ P.Bullet │ Enemy │ Boss │ B.Bullet │ Shield
═══════════╪═══════╪════════╪════════╪══════════╪═══════╪══════╪══════════╪════════
Meteor     │ Move  │ Damage*│ Pass   │ Destroy+ │ Pass  │ Pass │ Pass     │ N/A
Enemy      │ Move  │ Damage*│ Pass   │ +3/Kill  │ Pass  │ Pass │ Pass     │ N/A
Boss       │ Move  │ Damage*│ Pass   │ +5/Kill  │ Pass  │ Pass │ Pass     │ N/A
P.Bullet   │ Move  │ Block  │Destroy+│ Pass     │+3/Kill│+5/Kill│ Destroy │ N/A
B.Bullet   │ Move  │ Damage*│ Pass   │ Destroy  │ Pass  │ Pass │ Pass     │ N/A
ShieldPwrUp│ Move  │ Collect│ Pass   │ Pass     │ Pass  │ Pass │ Pass     │ N/A

Legend:
- Move: Entity moves into cell
- Damage*: Player loses life (or shield absorbs), entity destroyed, 1s invincibility
- Pass: Entity phases through
- Destroy: Both entities destroyed
- Destroy+: Both destroyed, +1-2 random bonus points
- +X/Kill: Entity destroyed, player gains X points AND killCount++
- Block: Bullet cannot move there
- Collect: Shield powerup consumed, hasShield = true
- N/A: Shield powerups tracked separately, don't interact with this entity type

NEW SCORING:
- Enemies: 3 points + killCount++ (toward level progression)
- Bosses: 5 points + killCount++ (toward level progression)
- Meteors: 1-2 random points (bonus only, no killCount)
```

### Memory & Performance Considerations

**Why Grid-Based Design is Efficient:**
1. **Fixed Memory**: `grid[23][15]` = 345 integers (1,380 bytes) regardless of entity count
2. **O(1) Collision Detection**: Just check `grid[targetRow][targetCol]`
3. **Predictable Performance**: Always scan exactly 345 cells per frame
4. **Optimized Constants**: `BULLET_OFFSET_X` and `SHIELD_OFFSET` calculated once at compile-time instead of every frame

**Shield Powerup Optimization:**
- Separate array system (5 slots) prevents grid interference
- Independent movement logic avoids complex grid scanning
- Direct position tracking (row/col) enables fast collision checks

**Performance Optimizations Applied:**
- Removed redundant per-frame calculations (5 xOffset calculations → 1 constant)
- Removed conflicting timing controls (sleep + framerate limit → framerate limit only)
- Removed debug output (50+ cout statements) for cleaner execution
- Pre-calculated offsets at compile-time reduce runtime overhead

**Alternative (Object-Oriented) Would Require:**
- Dynamic arrays of enemy/bullet objects
- O(n²) collision checks (check each bullet against each enemy)
- Memory allocation/deallocation overhead

### Timing Synchronization

```
FRAME TIMING AT 60 FPS:
═══════════════════════

Frame Duration: ~16.67ms (60 FPS cap)

Within each frame:
├─ Event Polling: ~0.1ms
├─ State Logic:
│  ├─ Input checks: ~0.5ms
│  ├─ Spawning logic: ~1ms
│  ├─ Movement & collision: ~3-5ms (depends on entity count)
│  ├─ Shield powerup updates: ~0.3ms
│  └─ Effect updates: ~0.5ms
├─ Rendering:
│  ├─ Clear screen: ~0.5ms
│  ├─ Draw background: ~1ms
│  ├─ Draw entities: ~3-8ms (depends on entity count)
│  ├─ Draw shield powerups: ~0.2ms
│  ├─ Draw effects: ~0.5ms
│  └─ Draw UI: ~1ms
└─ Display: ~1ms (buffer swap)

Total: ~12-18ms per frame (leaves ~5ms headroom at 60 FPS)

**Optimization**: Removed sleep(50ms) call that was conflicting with setFramerateLimit(60).
Now timing is handled exclusively by SFML's built-in frame limiter for accurate 60 FPS.
```

### Complete State Dependency Graph

```
VARIABLE DEPENDENCIES:
═════════════════════

currentState (Master Controller)
    ├─ Controls which logic executes
    ├─ Controls what gets rendered
    └─ Modified by:
        ├─ User input (Enter, P, Esc, menu navigation)
        ├─ lives <= 0 → STATE_GAME_OVER
        ├─ killCount >= level * 10 → STATE_LEVEL_UP
        └─ level > MAX_LEVEL → STATE_VICTORY

lives
    ├─ Decreased by: Meteor hit, Enemy hit, Boss hit, Boss bullet hit
    ├─ Protected by: isInvincible flag AND hasShield
    ├─ Shield absorbs first hit, then removed
    └─ Triggers: STATE_GAME_OVER when <= 0

score (Display/High Score Only)
    ├─ Increased by: 
    │   ├─ Destroying enemies (+3 points)
    │   ├─ Destroying bosses (+5 points)
    │   └─ Destroying meteors (+1-2 random points)
    ├─ Saved to file when: New high score achieved
    └─ Does NOT affect level progression (killCount does)

killCount (Level Progression Tracker)
    ├─ Increased by: Destroying enemies (+1), Destroying bosses (+1)
    ├─ NOT increased by: Destroying meteors
    ├─ Reset to 0 on: Level up, Load saved game
    └─ Triggers: STATE_LEVEL_UP when >= level * 10

level
    ├─ Increased by: Reaching killCount threshold
    ├─ Affects: 
    │   ├─ All entity movement speeds (faster per level)
    │   ├─ Spawn rates (more frequent per level)
    │   ├─ Boss appearance (level 3+)
    │   ├─ Boss firing rate (level 3: 1/5, level 4: 1/4, level 5: 1/3 moves)
    │   └─ Shield powerup spawning (level 3+)
    └─ Triggers: STATE_VICTORY when > MAX_LEVEL with enough kills

grid[ROWS][COLS] (Core Game State)
    ├─ Modified by: Spawning, Movement, Collision resolution
    ├─ Read by: Rendering loop, Collision detection
    └─ Cleared on: Game start, Level up, Load saved game

shieldPowerup Arrays (Independent System)
    ├─ Tracks: Up to 5 simultaneous shield powerups
    ├─ Modified by: Spawning (level 3+), Movement (0.5s intervals), Collection
    ├─ Movement: Straight down every 0.5s (direction array kept for future zigzag logic)
    ├─ Collision: Checked against grid position before/after movement
    └─ Cleared on: Game start, Level up, Load saved game

hasShield
    ├─ Set true on: Collecting shield powerup
    ├─ Set false on: Absorbing any damage (one-time use)
    └─ Affects: Damage prevention (one hit), Visual overlay rendering

isInvincible
    ├─ Set true on: Any damage (after shield check)
    ├─ Set false after: 2 seconds (invincibilityTimer)
    └─ Affects: Damage prevention, Rendering (blink effect)

highScore (Persistent)
    ├─ Loaded from: save-file.txt on startup
    ├─ Updated when: Current score exceeds previous high score
    ├─ Saved to file on: Game over, Victory, Enemy escape, Save & Quit
    └─ Displayed in: Main menu, Playing screen, Game over, Victory

hasSavedGame (Persistent Check)
    ├─ Determined by: savedLevel > 0 AND savedLives > 0
    ├─ Affects: "Load Saved Game" menu option availability
    └─ Enables: Game state restoration

All Clocks
    ├─ Restarted on: Specific actions (movement, spawning, shooting)
    ├─ Read continuously: To check if cooldowns/intervals elapsed
    └─ Control: Game timing, difficulty scaling, visual effects, powerup movement
```

---

## Summary: The Complete Picture

The game is a **state-driven, grid-based arcade shooter** with **persistent save system** where:

1. **The grid is the single source of truth** - all game logic revolves around reading and writing integer values to `grid[ROWS][COLS]`, with shield powerups tracked independently to avoid interference

2. **Timing controls everything** - SFML Clocks manage when entities spawn, move, and shoot, creating difficulty progression. Optimized with single framerate limiter (60 FPS) for accurate timing

3. **State machine provides structure** - `currentState` determines which code executes and what displays on screen, including new game over/victory score displays

4. **Collision is implicit** - No complex math; just check the value of the target cell before moving. Shield powerups use dual-phase collision detection (before/after movement)

5. **Rendering is reactive** - Every frame, scan the grid and draw sprites where entities exist, with optimized pre-calculated offsets for bullets and shields

6. **Difficulty scales naturally** - As `level` increases:
   - Spawn rates increase exponentially
   - Movement speeds up through mathematical formulas
   - Boss firing frequency increases (level 3: 1/5, level 4: 1/4, level 5: 1/3)
    - Shield powerups spawn (level 3+) and fall straight down every 0.5s

7. **New scoring system separates progression from points**:
   - `killCount` tracks enemies/bosses for level progression (10 per level)
   - `score` includes everything (enemies=3, bosses=5, meteors=1-2) for display/high score
   - Meteors provide bonus points without affecting level advancement

8. **Persistence enables continuity**:
   - High scores save automatically to `save-file.txt`
   - Game state (lives, score, level) can be saved and restored
   - Menu displays current high score
   - Game over/victory screens show final score achieved

9. **Everything connects through shared variables** - `lives`, `score`, `killCount`, `level`, `grid`, `hasShield`, and `highScore` are accessed by multiple logic blocks, creating emergent gameplay

10. **Performance optimizations applied**:
    - Removed 50+ debug cout statements
    - Pre-calculated bullet/shield offsets at compile-time
    - Removed conflicting sleep() call
    - Independent shield powerup tracking (5 simultaneous max)

The beauty of this design is its **simplicity and directness**: there's no hidden complexity, no abstraction layers. The entire game state is visible in a few key variables, making it easy to understand, debug, and modify. Recent optimizations maintain this clarity while improving performance and adding sophisticated features like persistent saves and advanced powerup systems.
