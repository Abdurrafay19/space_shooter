# 🚀 Space Shooter Game

<div align="center">

![Space Shooter](https://img.shields.io/badge/Game-Space%20Shooter-blue)
![Language](https://img.shields.io/badge/Language-C++-00599C?logo=cplusplus)
![Framework](https://img.shields.io/badge/Framework-SFML%202.5-8CC445)
![Build System](https://img.shields.io/badge/Build-CMake-064F8C?logo=cmake)
![License](https://img.shields.io/badge/License-MIT-green)

**A classic arcade-style space shooter game built with C++ and SFML**

[Features](#features) • [Installation](#installation) • [How to Play](#how-to-play) • [Game Mechanics](#game-mechanics) • [Documentation](#documentation)

</div>

---

## 📋 Table of Contents

- [Overview](#overview)
- [Features](#features)
- [Screenshots](#screenshots)
- [System Requirements](#system-requirements)
- [Installation](#installation)
  - [Linux Installation](#linux-installation)
  - [Windows Installation](#windows-installation)
  - [macOS Installation](#macos-installation)
- [Building from Source](#building-from-source)
- [How to Play](#how-to-play)
  - [Controls](#controls)
  - [Game Objectives](#game-objectives)
- [Game Mechanics](#game-mechanics)
  - [Entities](#entities)
  - [Power-ups](#power-ups)
  - [Scoring System](#scoring-system)
  - [Level Progression](#level-progression)
- [Game States](#game-states)
- [Technical Architecture](#technical-architecture)
  - [Grid System](#grid-system)
  - [Game Loop](#game-loop)
  - [Collision Detection](#collision-detection)
  - [Asset Management](#asset-management)
- [Code Structure](#code-structure)
- [Save System](#save-system)
- [Audio System](#audio-system)
- [Advanced Features](#advanced-features)
- [Configuration](#configuration)
- [Troubleshooting](#troubleshooting)
- [Future Enhancements](#future-enhancements)
- [Contributing](#contributing)
- [License](#license)
- [Credits](#credits)

---

## 🎮 Overview

**Space Shooter** is a classic arcade-style game developed in C++ using the SFML (Simple and Fast Multimedia Library) framework. Players control a spaceship at the bottom of the screen, defending against waves of meteorites, enemy UFOs, and powerful boss ships while progressing through increasingly challenging levels.

The game features a robust save system, multiple game states, power-ups, visual effects, immersive sound design, and a progressive difficulty curve that keeps players engaged. With 5 challenging levels to conquer, players must master both offense and defense to achieve victory.

### Key Highlights

- **5 Progressive Levels**: Each level increases in difficulty with faster enemies and more aggressive spawning
- **Multiple Enemy Types**: Face meteors, UFOs, and powerful boss enemies with unique behaviors
- **Power-up System**: Collect shield power-ups for protection (available from Level 3+)
- **Boss Battles**: Starting from Level 3, boss enemies can shoot back at you
- **Save/Load System**: Save your progress and resume later
- **High Score Tracking**: Compete with yourself to beat your best score
- **Immersive Audio**: Background music and sound effects for all actions
- **Visual Effects**: Explosion animations and hit effects for satisfying gameplay
- **Responsive Controls**: Smooth keyboard-based movement and shooting

---

## ✨ Features

### Core Gameplay
- **Player-Controlled Spaceship**: Move left and right to dodge enemies and position shots
- **Shooting Mechanics**: Fire bullets to destroy incoming threats
- **Enemy Variety**: Three types of enemies with different point values and behaviors
- **Boss Enemies**: Powerful foes that shoot projectiles (Level 3+)
- **Progressive Difficulty**: Each level increases speed and spawn rates
- **Life System**: Start with 3 lives; lose one when hit by enemies or projectiles
- **Invincibility Frames**: 2-second invincibility after taking damage

### Game Systems
- **5 Levels to Complete**: Each requiring 10 kills to advance
- **High Score Persistence**: Your best score is saved automatically
- **Save/Load Functionality**: Save your progress mid-game and return later
- **Shield Power-ups**: Protective shields that absorb one hit (Level 3+)
- **Real-time Score Tracking**: Points awarded for destroying different enemy types
- **Kill Counter**: Track progress toward the next level

### Visual & Audio
- **Sprite-Based Graphics**: Custom textures for all game entities
- **Animated Effects**: Explosion effects when enemies are destroyed
- **Background Music**: Looping soundtrack for immersive experience
- **Sound Effects**: Unique sounds for shooting, explosions, damage, level-up, and UI interactions
- **Multiple Backgrounds**: Different visuals for menus and gameplay
- **Visual Feedback**: Blinking effects for invincibility and level transitions

### User Interface
- **Main Menu**: Start new game, load saved game, view instructions, or exit
- **Instructions Screen**: Comprehensive guide on controls and gameplay
- **Pause Menu**: Pause mid-game to resume, restart, or save and quit
- **Game Over Screen**: Shows final score with options to restart or return to menu
- **Victory Screen**: Celebrate completing all 5 levels
- **HUD Display**: Shows lives (icons), score, kill count, level, and high score

---

## 📸 Screens

### Main Menu
The main menu features a starry background with options to start a new game, load a saved game, view instructions, or exit. The high score is prominently displayed.

### Gameplay
The game field is a grid-based arena where the player's ship appears at the bottom. Enemies spawn from the top and move downward. The right panel displays game statistics including lives (shown as heart icons), current score, kills required for next level, current level, and high score.

### Instructions Screen
A comprehensive help screen shows all controls, entity descriptions with visual examples, and game system explanations.

### Boss Battle
Starting from Level 3, powerful boss ships appear and can fire green projectiles at the player, adding an extra layer of challenge.

---

## 💻 System Requirements

### Minimum Requirements
- **Operating System**: Linux, Windows 10/11, or macOS 10.15+
- **Processor**: 1.5 GHz dual-core processor
- **Memory**: 512 MB RAM
- **Graphics**: OpenGL 2.0 compatible graphics card
- **Storage**: 100 MB available space
- **Additional**: Speakers or headphones for audio

### Recommended Requirements
- **Operating System**: Linux (Ubuntu 20.04+), Windows 10/11, or macOS 11+
- **Processor**: 2.0 GHz quad-core processor
- **Memory**: 1 GB RAM
- **Graphics**: Dedicated graphics card with OpenGL 3.0 support
- **Storage**: 200 MB available space

### Dependencies
- **SFML 2.5+**: Simple and Fast Multimedia Library
- **CMake 3.10+**: Build system generator
- **C++17 Compatible Compiler**: GCC 7+, Clang 5+, or MSVC 2017+
- **OpenGL**: For rendering

---

## 🔧 Installation

### Linux Installation

#### Ubuntu/Debian-based Systems

1. **Install Dependencies**:
```bash
sudo apt update
sudo apt install build-essential cmake
sudo apt install libsfml-dev
```

2. **Clone the Repository**:
```bash
git clone https://github.com/Abdurrafay19/space_shooter.git
cd space_shooter
```

3. **Build the Project**:
```bash
mkdir -p build
cd build
cmake ..
cmake --build .
```

4. **Run the Game**:
```bash
./sfml_project
```

#### Arch Linux

1. **Install Dependencies**:
```bash
sudo pacman -S base-devel cmake sfml
```

2. **Clone and Build**:
```bash
git clone https://github.com/Abdurrafay19/space_shooter.git
cd space_shooter
mkdir -p build && cd build
cmake .. && cmake --build .
./sfml_project
```

#### Fedora/RHEL-based Systems

1. **Install Dependencies**:
```bash
sudo dnf install gcc-c++ cmake
sudo dnf install SFML-devel
```

2. **Follow the build steps** as shown in Ubuntu section

### Windows Installation

#### Using MinGW-w64

1. **Install Required Tools**:
   - Download and install [MinGW-w64](https://www.mingw-w64.org/)
   - Download and install [CMake](https://cmake.org/download/)
   - Add both to your system PATH

2. **Install SFML**:
   - Download SFML 2.5+ for MinGW from [SFML Downloads](https://www.sfml-dev.org/download.php)
   - Extract to `C:\SFML`

3. **Clone the Repository**:
```cmd
git clone https://github.com/Abdurrafay19/space_shooter.git
cd space_shooter
```

4. **Build with CMake**:
```cmd
mkdir build
cd build
cmake .. -G "MinGW Makefiles" -DSFML_DIR=C:\SFML\lib\cmake\SFML
cmake --build .
```

5. **Run the Game**:
```cmd
sfml_project.exe
```

#### Using Visual Studio

1. **Install Visual Studio 2019 or later** with C++ development tools
2. **Install SFML** for Visual Studio from the official website
3. **Open CMake GUI** and configure the project
4. **Generate Visual Studio solution** and build

### macOS Installation

1. **Install Homebrew** (if not already installed):
```bash
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
```

2. **Install Dependencies**:
```bash
brew install cmake sfml
```

3. **Clone and Build**:
```bash
git clone https://github.com/Abdurrafay19/space_shooter.git
cd space_shooter
mkdir -p build && cd build
cmake .. && cmake --build .
./sfml_project
```

---

## 🏗️ Building from Source

### Prerequisites
- CMake 3.10 or higher
- C++17 compatible compiler
- SFML 2.5 or higher

### Build Process

1. **Clone the repository**:
```bash
git clone https://github.com/Abdurrafay19/space_shooter.git
cd space_shooter
```

2. **Create build directory**:
```bash
mkdir -p build
cd build
```

3. **Generate build files**:
```bash
cmake ..
```

4. **Compile the project**:
```bash
cmake --build .
```

5. **Run the executable**:
```bash
./sfml_project  # Linux/macOS
sfml_project.exe  # Windows
```

### CMake Build Options

The project uses a simple CMakeLists.txt that:
- Sets C++17 as the standard
- Finds and links SFML components (graphics, window, system, audio)
- Copies the assets folder to the build directory automatically
- Creates the executable `sfml_project`

### Directory Structure After Build
```
space_shooter/
├── build/
│   ├── sfml_project        # Executable
│   ├── assets/             # Copied from source
│   └── ...                 # Build files
├── assets/                 # Source assets
├── main.cpp                # Source code
├── CMakeLists.txt          # Build configuration
└── README.md               # This file
```

---

## 🎯 How to Play

### Controls

#### Spaceship Movement
- **Move Left**: `A` or `Left Arrow`
- **Move Right**: `D` or `Right Arrow`
- **Shoot**: `Spacebar`
- **Pause Game**: `P`

#### Menu Navigation
- **Navigate Up**: `W` or `Up Arrow`
- **Navigate Down**: `S` or `Down Arrow`
- **Select Option**: `Enter`
- **Back (Instructions)**: `Escape` or `Backspace`

### Game Objectives

1. **Survive**: Avoid collisions with meteors, enemies, and boss bullets
2. **Destroy Enemies**: Shoot down meteors, UFOs, and bosses to earn points
3. **Level Up**: Destroy 10 enemies per level to progress
4. **Complete All Levels**: Beat Level 5 to achieve victory
5. **Set High Scores**: Compete with yourself to beat your best score

### Starting the Game

1. **Launch the Game**: Run the executable
2. **Main Menu Options**:
   - **Start Game**: Begin a new game from Level 1 with 3 lives
   - **Load Saved Game**: Continue from your last saved progress
   - **Instructions**: View detailed gameplay instructions
   - **Exit**: Close the game

### During Gameplay

- **Movement**: Your spaceship can only move left and right along the bottom row
- **Shooting**: Fire bullets upward by pressing spacebar (0.3-second cooldown)
- **Avoiding**: Dodge incoming meteors, enemies, and boss bullets
- **Collecting**: Fly into shield power-ups to gain protection
- **Pausing**: Press `P` to access the pause menu

### Pause Menu Options

- **Resume**: Continue playing
- **Restart**: Restart the current level
- **Save & Quit**: Save your progress and return to main menu

---

## ⚙️ Game Mechanics

### Entities

#### Player Spaceship
- **Lives**: Starts with 3 lives
- **Movement Speed**: Moves one grid cell per input with 0.1-second cooldown
- **Fire Rate**: Can shoot every 0.3 seconds
- **Invincibility**: 2 seconds of invincibility after taking damage (blinking effect)
- **Position**: Always at the bottom row, can move horizontally

#### Meteors
- **Point Value**: 1-2 points (random)
- **Behavior**: Move straight down at increasing speeds per level
- **Spawn Rate**: 1-3 second intervals (random)
- **Speed Formula**: `0.7 - ((level - 1) * 0.12)` seconds per move, minimum 0.333s
- **Collision**: Destroys on bullet hit; damages player on contact

#### Enemy UFOs
- **Point Value**: 3 points
- **Kill Count**: Counts toward level progression
- **Behavior**: Move straight down at increasing speeds
- **Spawn Rate**: Decreases with level for more frequent spawns
- **Speed**: Same as meteors, scales with level
- **Collision**: Destroys on bullet hit; damages player on contact

#### Boss Enemies
- **Availability**: Level 3 and above
- **Point Value**: 5 points
- **Kill Count**: Counts toward level progression
- **Behavior**: Moves down and fires projectiles at the player
- **Firing Rate**:
  - Level 3: Fires every 4 movements
  - Level 4: Fires every 3 movements
  - Level 5: Fires every 2 movements
- **Spawn Rate**: 10-14 second intervals, decreasing with level
- **Speed**: `0.8 - ((level - 3) * 0.1)` seconds per move, minimum 0.5s

#### Boss Bullets
- **Behavior**: Move downward very quickly (0.15s per move)
- **Collision**: Can be destroyed by player bullets; damages player on hit
- **Visual**: Green laser projectile

#### Player Bullets
- **Speed**: Very fast (0.05s per move)
- **Behavior**: Move straight up
- **Collision**: Destroys meteors, enemies, bosses, and boss bullets

### Power-ups

#### Shield Power-up
- **Availability**: Level 3 and above
- **Spawn Rate**: 
  - Levels 3-4: 20-35 second intervals
  - Level 5: 12-20 second intervals
- **Effect**: Absorbs one hit from any source
- **Visual Indicator**: Blue shield overlay on player sprite
- **Collection**: Fly into the power-up to collect
- **Duration**: Until hit or end of level

### Scoring System

| Entity | Points | Counts Toward Level Progress |
|--------|--------|------------------------------|
| Meteor | 1-2 (random) | ❌ No |
| Enemy UFO | 3 | ✅ Yes |
| Boss Enemy | 5 | ✅ Yes |

**High Score**: Automatically saved when you beat your previous best

### Level Progression

- **Total Levels**: 5
- **Kills Required**: 10 enemies/bosses per level
- **Difficulty Scaling**:
  - **Speed**: Enemies and meteors move faster each level
  - **Spawn Rate**: Enemies spawn more frequently
  - **Boss Introduction**: Bosses appear starting Level 3
  - **Boss Aggression**: Bosses fire more frequently in higher levels
  - **Power-ups**: Shield power-ups become more frequent in Level 5

### Level Transition

When you reach 10 kills:
1. All entities are cleared from the screen
2. "LEVEL UP!" message appears with blinking animation
3. Kill counter resets to 0
4. Player respawns at center bottom
5. After 2 seconds, gameplay resumes with increased difficulty

### Victory Condition

Complete Level 5 by destroying 10 enemies/bosses to win the game.

### Game Over Conditions

You lose all 3 lives by:
- Colliding with meteors
- Colliding with enemy UFOs
- Colliding with boss enemies
- Being hit by boss bullets
- Allowing enemies to reach the bottom of the screen

---

## 🎨 Game States

The game uses a state machine with 7 distinct states:

### 1. Main Menu (`STATE_MENU`)
- **Purpose**: Entry point and navigation hub
- **Features**:
  - Display high score
  - Start new game
  - Load saved game (if available)
  - View instructions
  - Exit game
- **Navigation**: Arrow keys or W/S, Enter to select
- **Background Music**: Playing

### 2. Instructions (`STATE_INSTRUCTIONS`)
- **Purpose**: Tutorial and reference guide
- **Content**:
  - Control scheme
  - Entity descriptions with visual examples
  - Game system explanations
  - Objective overview
- **Exit**: ESC or Backspace to return to menu

### 3. Playing (`STATE_PLAYING`)
- **Purpose**: Main gameplay state
- **Active Systems**:
  - Player input handling
  - Entity spawning and movement
  - Collision detection
  - Score tracking
  - Level progression
  - Shield power-up spawning (Level 3+)
- **Can Pause**: Press P
- **Background Music**: Stopped during gameplay

### 4. Paused (`STATE_PAUSED`)
- **Purpose**: Temporary game suspension
- **Features**:
  - Game state frozen
  - Semi-transparent overlay
  - Options: Resume, Restart, Save & Quit
- **Navigation**: Arrow keys or W/S, Enter to select
- **Game State**: All timers paused, entities frozen

### 5. Level Up (`STATE_LEVEL_UP`)
- **Purpose**: Transition between levels
- **Duration**: 2 seconds
- **Visual**: Blinking "LEVEL UP!" text (0.3s intervals)
- **Behavior**:
  - Clears all entities
  - Resets player position
  - Updates level counter
  - Resets kill counter
- **Transition**: Automatically returns to playing state

### 6. Game Over (`STATE_GAME_OVER`)
- **Purpose**: Handle player defeat
- **Features**:
  - Display final score
  - Update high score if beaten
  - Options: Restart or Main Menu
- **Triggered**: When lives reach 0
- **Sound**: Plays lose sound effect
- **Save System**: Clears any saved game data

### 7. Victory (`STATE_VICTORY`)
- **Purpose**: Celebrate completion
- **Features**:
  - Display final score
  - Update high score if beaten
  - Options: Restart or Main Menu
- **Triggered**: Complete Level 5 (50 total kills)
- **Sound**: Plays victory sound effect
- **Background Music**: Returns when returning to menu

---

## 🏗️ Technical Architecture

### Grid System

The game uses a 2D grid-based coordinate system for entity management:

```cpp
const int ROWS = 23;        // Vertical cells
const int COLS = 15;        // Horizontal cells
const int CELL_SIZE = 40;   // Pixels per cell
const int MARGIN = 40;      // Border around grid
```

#### Grid Cell Values
- `0`: Empty space
- `1`: Player spaceship
- `2`: Meteor
- `3`: Player bullet
- `4`: Enemy UFO
- `5`: Boss enemy
- `6`: Boss bullet

#### Advantages
- **Simplified Collision**: Check grid cell values instead of complex calculations
- **Predictable Movement**: Entities move one cell at a time
- **Easy Debugging**: Visual grid structure matches code structure
- **Memory Efficient**: Small 2D integer array

### Game Loop

The main game loop follows the standard pattern:

```cpp
while (window.isOpen()) {
    // 1. Event Handling
    while (window.pollEvent(event)) {
        // Handle window close
    }
    
    // 2. Update Logic (based on current state)
    // - Player input
    // - Entity movement
    // - Collision detection
    // - Score updates
    // - State transitions
    
    // 3. Rendering
    window.clear();
    // Draw all sprites based on grid state
    window.display();
}
```

**Frame Rate**: Locked at 60 FPS for consistent gameplay

### Collision Detection

#### Grid-Based Collision
The game uses grid cell checking for collision detection:

1. **Entity Movement**: When an entity tries to move to a new cell, check the destination
2. **Cell Value Check**: Read the grid value at destination
3. **Collision Resolution**: Based on the entity types involved

#### Collision Matrix

| Moving Entity | Target Entity | Result |
|--------------|---------------|---------|
| Meteor | Player | Damage player, clear meteor |
| Meteor | Bullet | +1-2 score, clear both |
| Enemy | Player | Damage player, clear enemy |
| Enemy | Bullet | +3 score, +1 kill, clear both |
| Boss | Player | Damage player, clear boss |
| Boss | Bullet | +5 score, +1 kill, clear both |
| Boss Bullet | Player | Damage player, clear bullet |
| Boss Bullet | Bullet | Clear both bullets |

#### Shield Protection
When player has shield active:
1. Collision occurs normally
2. Shield is removed instead of life
3. Player gains 2s invincibility
4. Shield visual disappears

### Asset Management

#### Texture Loading
All textures are loaded at game startup:

```cpp
Texture texture;
if (!texture.loadFromFile("path/to/texture.png")) {
    cerr << "Failed to load texture" << endl;
    return -1;
}
```

#### Sprite Setup
Sprites are scaled to fit grid cells:

```cpp
sprite.setTexture(texture);
sprite.setScale(
    (CELL_SIZE * scaleX) / texture.getSize().x,
    (CELL_SIZE * scaleY) / texture.getSize().y
);
```

#### Asset Organization
```
assets/
├── fonts/
│   └── font.ttf            # UI text rendering
├── images/
│   ├── player.png          # Player spaceship
│   ├── meteorSmall.png     # Meteor sprite
│   ├── enemyUFO.png        # Enemy UFO sprite
│   ├── enemyShip.png       # Boss sprite
│   ├── laserRed.png        # Player bullet
│   ├── laserRedShot.png    # Bullet hit effect
│   ├── laserGreen.png      # Boss bullet
│   ├── laserGreenShot.png  # Boss bullet hit effect
│   ├── shield.png          # Shield overlay
│   ├── shield-powerup.png  # Shield collectible
│   ├── life.png            # Life icon
│   ├── backgroundColor.png # Game background
│   └── starBackground.png  # Menu background
└── sounds/
    ├── bg-music.mp3        # Background music
    ├── shoot.wav           # Player shooting
    ├── explosion.wav       # Enemy destroyed
    ├── damage.mp3          # Player hit
    ├── level-up.mp3        # Level complete
    ├── powerup.wav         # Shield collected
    ├── win.wav             # Victory sound
    ├── lose.wav            # Game over sound
    ├── menu-click.mp3      # Menu selection
    └── menu-navigate.wav   # Menu navigation
```

---

## 📁 Code Structure

### Main Components

#### Helper Functions

**Grid Management**:
```cpp
void clearGrid(int grid[][15])
void clearEntities(int grid[][15])  // Clears only entities, keeps player
void resetSpaceship(int grid[][15], int& spaceshipCol)
```

**Game State Management**:
```cpp
void saveHighScoreAndGameOver(...)  // Save score and trigger game over
void saveHighScoreAndVictory(...)   // Save score and trigger victory
void restartAllClocks(...)          // Reset all timing clocks
```

**Visual Effects**:
```cpp
void createExplosionEffect(...)     // Add explosion animation
```

**UI Utilities**:
```cpp
void setMenuColors(...)             // Highlight selected menu item
bool loadTexture(...)               // Safe texture loading
void setupSprite(...)               // Configure sprite scaling
```

#### Main Function Structure

1. **Initialization** (Lines 1-520):
   - Window setup
   - Save file handling
   - Variable declarations
   - Texture and sprite loading
   - Font loading
   - Audio setup
   - UI text configuration
   - Clock initialization

2. **Game Loop** (Lines 521-1730):
   - Event handling
   - State-specific logic
   - State-specific rendering
   - Window display

### Key Variables

#### Game State
```cpp
int currentState = STATE_MENU;       // Current game state
int selectedMenuItem = 0;            // Menu cursor position
int lives = 3;                       // Player lives remaining
int score = 0;                       // Current score
int killCount = 0;                   // Enemies killed in current level
int level = 1;                       // Current level (1-5)
```

#### Player State
```cpp
int spaceshipCol = COLS / 2;         // Player column position
bool isInvincible = false;           // Invincibility status
bool hasShield = false;              // Shield power-up status
```

#### Timing Clocks
```cpp
Clock moveClock;                     // Movement cooldown
Clock bulletFireClock;               // Shooting cooldown
Clock meteorSpawnClock;              // Meteor spawn timer
Clock meteorMoveClock;               // Meteor movement timer
Clock enemySpawnClock;               // Enemy spawn timer
Clock enemyMoveClock;                // Enemy movement timer
Clock bossSpawnClock;                // Boss spawn timer
Clock bossMoveClock;                 // Boss movement timer
Clock bossBulletMoveClock;           // Boss bullet movement timer
Clock bulletMoveClock;               // Player bullet movement timer
Clock shieldPowerupSpawnClock;       // Shield spawn timer
Clock shieldPowerupMoveClock;        // Shield movement timer
Clock invincibilityTimer;            // Invincibility duration
Clock levelUpTimer;                  // Level transition duration
Clock menuClock;                     // Menu input cooldown
Clock hitEffectClock;                // Explosion effect timer
```

#### Arrays
```cpp
int grid[ROWS][COLS];                                    // Main game grid
int shieldPowerupRow[MAX_SHIELD_POWERUPS];              // Shield positions
int shieldPowerupCol[MAX_SHIELD_POWERUPS];
bool shieldPowerupActive[MAX_SHIELD_POWERUPS];
int hitEffectRow[MAX_HIT_EFFECTS];                      // Explosion effects
int hitEffectCol[MAX_HIT_EFFECTS];
float hitEffectTimer[MAX_HIT_EFFECTS];
bool hitEffectActive[MAX_HIT_EFFECTS];
```

---

## 💾 Save System

### Save File Format

**File**: `save-file.txt`
**Location**: Same directory as executable
**Format**: Space-separated integers

```
<high_score> <saved_lives> <saved_score> <saved_level>
```

**Example**:
```
1250 2 850 3
```
- High Score: 1250
- Lives: 2
- Score: 850
- Level: 3

### Save System Behavior

#### Automatic High Score Saving
- Triggers on game over or victory
- Updates if current score exceeds high score
- Always persists the best score

#### Manual Progress Saving
- Available through pause menu → "Save & Quit"
- Saves current lives, score, and level
- Allows resuming from exact state

#### Load Behavior
- "Load Saved Game" only available if valid save exists
- Valid save requires: `savedLevel > 0 && savedLives > 0`
- Loading restores lives, score, and level
- Kill counter resets to 0 on load

#### Save Clearing
- Occurs on game over or victory
- Sets saved game data to 0
- Prevents loading from lost/won states
- High score always preserved

### Implementation Details

**Reading Save File**:
```cpp
ifstream inputFile(saveFile);
if (inputFile.is_open()) {
    inputFile >> highScore >> savedLives >> savedScore >> savedLevel;
    inputFile.close();
    
    if (savedLevel > 0 && savedLives > 0) {
        hasSavedGame = true;
    }
}
```

**Writing Save File**:
```cpp
ofstream outputFile(saveFile);
if (outputFile.is_open()) {
    outputFile << highScore << " " << lives << " " << score << " " << level;
    outputFile.close();
}
```

**Clearing Saved Game**:
```cpp
ofstream outputFile(saveFile);
if (outputFile.is_open()) {
    outputFile << highScore << " 0 0 0";
    outputFile.close();
    hasSavedGame = false;
}
```

---

## 🔊 Audio System

### Background Music

**File**: `bg-music.mp3`
**Behavior**:
- Loops continuously
- Plays on main menu, instructions, game over, and victory screens
- Stops during active gameplay
- Volume: 30% (low background level)

**Control**:
```cpp
bgMusic.setLoop(true);
bgMusic.setVolume(30);
bgMusic.play();  // On menu screens
bgMusic.stop();  // During gameplay
```

### Sound Effects

| Sound | File | Trigger |
|-------|------|---------|
| Shoot | shoot.wav | Player fires bullet |
| Explosion | explosion.wav | Enemy/meteor/boss destroyed |
| Damage | damage.mp3 | Player takes damage |
| Level Up | level-up.mp3 | Player advances to next level |
| Power-up | powerup.wav | Shield collected |
| Win | win.wav | Victory screen appears |
| Lose | lose.wav | Game over screen appears |
| Menu Click | menu-click.mp3 | Menu option selected |
| Menu Navigate | menu-navigate.wav | Menu cursor moved |

### Audio Architecture

**Loading**:
```cpp
SoundBuffer shootBuffer;
shootBuffer.loadFromFile("assets/sounds/shoot.wav");

Sound shootSound;
shootSound.setBuffer(shootBuffer);
```

**Playing**:
```cpp
shootSound.play();  // Fire and forget
```

**Buffer Lifetime**: All `SoundBuffer` objects persist for game lifetime to prevent loading/unloading overhead.

---

## 🚀 Advanced Features

### Invincibility System

**Purpose**: Prevent rapid consecutive damage
**Duration**: 2 seconds after taking damage
**Visual Feedback**: Player sprite blinks (0.1s intervals)

**Implementation**:
```cpp
bool isInvincible = false;
Clock invincibilityTimer;

// On damage taken:
isInvincible = true;
invincibilityTimer.restart();

// Visual blinking:
if (!isInvincible || ((int)(invincibilityTimer.getElapsedTime().asMilliseconds() / 100) % 2 == 0)) {
    window.draw(spaceship);
}

// Check expiration:
if (isInvincible && invincibilityTimer.getElapsedTime().asSeconds() >= 2.0f) {
    isInvincible = false;
}
```

### Explosion Effects

**Purpose**: Visual feedback for destroyed entities
**Duration**: 0.3 seconds per effect
**Max Concurrent**: 50 effects
**Sprite**: Red/green shot sprite overlay

**Effect Pool**:
```cpp
const int MAX_HIT_EFFECTS = 50;
int hitEffectRow[MAX_HIT_EFFECTS];
int hitEffectCol[MAX_HIT_EFFECTS];
float hitEffectTimer[MAX_HIT_EFFECTS];
bool hitEffectActive[MAX_HIT_EFFECTS];
```

**Creation**:
```cpp
void createExplosionEffect(int row, int col, ...) {
    for (int i = 0; i < maxEffects; i++) {
        if (!hitEffectActive[i]) {
            hitEffectRow[i] = row;
            hitEffectCol[i] = col;
            hitEffectTimer[i] = 0.0f;
            hitEffectActive[i] = true;
            break;  // Use first available slot
        }
    }
}
```

### Dynamic Difficulty Scaling

#### Enemy Speed Formula
```cpp
float enemyMoveSpeed = 0.7f - ((level - 1) * 0.12f);
if (enemyMoveSpeed < 0.333f) {
    enemyMoveSpeed = 0.333f;  // Cap minimum
}
```

**Level Speed Table**:
| Level | Move Interval | Moves Per Second |
|-------|---------------|------------------|
| 1 | 0.70s | 1.43 |
| 2 | 0.58s | 1.72 |
| 3 | 0.46s | 2.17 |
| 4 | 0.34s | 2.94 |
| 5 | 0.33s | 3.03 |

#### Spawn Rate Scaling
```cpp
float baseTime = 2.0f - (level * 0.35f);
float variance = 2.5f - (level * 0.35f);
if (baseTime < 0.5f) baseTime = 0.5f;
if (variance < 1.0f) variance = 1.0f;
nextEnemySpawnTime = baseTime + (rand() % (int)variance);
```

### Boss Mechanics

#### Movement Pattern
- Moves straight down like regular enemies
- Same speed scaling as enemies
- Can collide with player directly

#### Firing System
```cpp
bossMoveCounter++;  // Increment on each movement

float firingInterval;
if (level == 3) firingInterval = 4;       // Fire every 4 moves
else if (level == 4) firingInterval = 3;  // Fire every 3 moves
else firingInterval = 2;                  // Fire every 2 moves (Level 5)

if (bossMoveCounter >= firingInterval) {
    // Spawn boss bullet below boss
    bossMoveCounter = 0;
}
```

#### Boss Bullet Behavior
- Moves very fast (0.15s per cell)
- Travels straight down
- Can be destroyed by player bullets
- Penetrates through meteors and enemies

### Shield Power-up System

**Separate from Grid**: Uses independent arrays to avoid grid collision
**Collection**: Automatic when player overlaps
**Effect**: Prevents next damage instance
**Visual**: Blue shield overlay on player sprite
**Spawn Logic**:
```cpp
if (level >= 3 && shieldPowerupSpawnClock.getElapsedTime().asSeconds() >= nextShieldPowerupSpawnTime) {
    // Find empty slot
    for (int i = 0; i < MAX_SHIELD_POWERUPS; i++) {
        if (!shieldPowerupActive[i]) {
            shieldPowerupRow[i] = 0;
            shieldPowerupCol[i] = rand() % COLS;
            shieldPowerupActive[i] = true;
            break;
        }
    }
}
```

---

## ⚙️ Configuration

### Gameplay Constants

```cpp
// Grid dimensions
const int ROWS = 23;
const int COLS = 15;
const int CELL_SIZE = 40;
const int MARGIN = 40;

// Game balance
const int MAX_LEVEL = 5;
const float INVINCIBILITY_DURATION = 2.0f;
const float HIT_EFFECT_DURATION = 0.3f;

// Power-ups
const int MAX_SHIELD_POWERUPS = 5;
const int MAX_HIT_EFFECTS = 50;

// Timing
Time moveCooldown = milliseconds(100);       // Player movement delay
Time bulletFireCooldown = milliseconds(300); // Shooting cooldown
Time menuCooldown = milliseconds(200);       // Menu navigation delay
```

### Modifying Difficulty

**Make Game Easier**:
- Increase starting lives: `int lives = 5;`
- Decrease kills needed per level: `int killsNeeded = level * 5;`
- Increase invincibility duration: `const float INVINCIBILITY_DURATION = 3.0f;`
- Slow down enemies: Increase base move speeds

**Make Game Harder**:
- Decrease starting lives: `int lives = 1;`
- Increase kills needed: `int killsNeeded = level * 15;`
- Decrease invincibility: `const float INVINCIBILITY_DURATION = 1.0f;`
- Speed up enemies: Decrease base move speeds
- Increase boss firing rates

### Window Configuration

```cpp
const int windowWidth = COLS * CELL_SIZE + MARGIN * 2 + 500;
const int windowHeight = ROWS * CELL_SIZE + MARGIN * 2;
RenderWindow window(VideoMode(windowWidth, windowHeight), "Space Shooter");
window.setFramerateLimit(60);
```

**Default**: 1100x960 pixels, 60 FPS

---

## 🐛 Troubleshooting

### Common Issues

#### Game Won't Launch

**Problem**: Executable doesn't start or crashes immediately
**Solutions**:
1. Check SFML installation:
   ```bash
   ldconfig -p | grep sfml  # Linux
   ```
2. Verify assets folder is in the same directory as executable
3. Check console for error messages
4. Ensure OpenGL drivers are up to date

#### No Sound/Music

**Problem**: Game runs but no audio plays
**Solutions**:
1. Verify audio files exist in `assets/sounds/`
2. Check system audio settings and volume
3. Ensure audio device is connected
4. On Linux, install audio backend:
   ```bash
   sudo apt install libopenal1
   ```

#### Missing Textures

**Problem**: Black rectangles or missing sprites
**Solutions**:
1. Verify all image files exist in `assets/images/`
2. Check file permissions
3. Ensure images are valid PNG format
4. Check console for "Failed to load" messages

#### Build Errors

**Problem**: CMake or compilation fails
**Solutions**:
1. Verify SFML version (2.5 or higher required)
2. Check CMake version (3.10+ required)
3. Ensure C++17 compiler support
4. Clean build directory and rebuild:
   ```bash
   rm -rf build
   mkdir build && cd build
   cmake .. && cmake --build .
   ```

#### Frame Rate Issues

**Problem**: Game runs too slow or stutters
**Solutions**:
1. Update graphics drivers
2. Close background applications
3. Check CPU usage
4. Ensure hardware meets minimum requirements

#### Save File Issues

**Problem**: Can't save or load games
**Solutions**:
1. Check write permissions in game directory
2. Delete corrupted `save-file.txt` and restart
3. Manually create `save-file.txt` with content: `0 0 0 0`

### Error Messages

| Error | Meaning | Solution |
|-------|---------|----------|
| "Failed to load [asset]" | Missing asset file | Verify assets folder structure |
| "Failed to load font" | Font file missing | Check `assets/fonts/font.ttf` exists |
| Segmentation fault | Memory access error | Rebuild with debug symbols, check array bounds |
| "No Saved Game Exists!" | No valid save data | Start a new game or create save manually |

### Performance Optimization

If experiencing performance issues:

1. **Reduce Visual Effects**: Decrease `MAX_HIT_EFFECTS` constant
2. **Lower Frame Rate**: Change `window.setFramerateLimit(30);`
3. **Disable Music**: Comment out `bgMusic.play()` calls
4. **Simplify Rendering**: Reduce entity count per level

---

## 🤝 Contributing

Contributions are welcome! Whether it's bug fixes, new features, or documentation improvements, your help is appreciated.

### How to Contribute

1. **Fork the Repository**
   ```bash
   git clone https://github.com/Abdurrafay19/space_shooter.git
   ```

2. **Create a Feature Branch**
   ```bash
   git checkout -b feature/your-feature-name
   ```

3. **Make Your Changes**
   - Follow existing code style
   - Add comments for complex logic
   - Test thoroughly

4. **Commit Your Changes**
   ```bash
   git commit -m "Add: Brief description of changes"
   ```

5. **Push to Your Fork**
   ```bash
   git push origin feature/your-feature-name
   ```

6. **Open a Pull Request**
   - Describe your changes
   - Reference any related issues
   - Include screenshots if UI-related

### Contribution Guidelines

- **Code Style**: Follow existing formatting and naming conventions
- **Comments**: Document functions and complex logic
- **Testing**: Ensure changes don't break existing functionality
- **Assets**: Provide attribution for any new assets
- **Documentation**: Update README if adding features

### Bug Reports

Found a bug? Please open an issue with:
- Detailed description
- Steps to reproduce
- Expected vs actual behavior
- System information (OS, compiler, SFML version)
- Console output or error messages

### Feature Requests

Have an idea? Open an issue with:
- Clear description of the feature
- Use cases and benefits
- Potential implementation approach (optional)

---

## 📄 License

This project is licensed under the **MIT License**.

```
MIT License

Copyright (c) 2025 Abdurrafay19

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```

### Third-Party Assets

This project uses the following third-party resources:

**SFML (Simple and Fast Multimedia Library)**
- License: zlib/png license
- Website: https://www.sfml-dev.org/

**Font and Graphics Assets**
- Sourced from: opengameart.org

---

## 👨‍💻 Credits

### Development
- **Project Type**: Programming Fundamentals Course Project
- **Framework**: SFML 2.5
- **Language**: C++17

### Contact
- **GitHub**: [@Abdurrafay19](https://github.com/Abdurrafay19)
- **Repository**: [space_shooter](https://github.com/Abdurrafay19/space_shooter)
- **Issues**: [Report bugs or request features](https://github.com/Abdurrafay19/space_shooter/issues)

---

## 📊 Project Statistics

- **Total Lines of Code**: ~1,730 lines
- **Language**: C++ (100%)
- **Dependencies**: SFML 2.5, CMake 3.10+
- **Asset Count**: 13 images, 10 sounds, 1 font
- **Game States**: 7
- **Entity Types**: 6
- **Total Levels**: 5

---

## 🎓 Educational Value

This project demonstrates proficiency in:

### Programming Concepts
- **Object-Oriented Design**: Sprite management, state machines
- **Data Structures**: 2D arrays, parallel arrays
- **Control Flow**: Complex state management with switch/if-else
- **File I/O**: Save system implementation
- **Random Number Generation**: Procedural spawning
- **Timing and Clocks**: Event scheduling and cooldowns

### Game Development
- **Game Loop Architecture**: Update-render cycle
- **Collision Detection**: Grid-based collision systems
- **State Management**: Multiple game states
- **Difficulty Scaling**: Progressive challenge increase
- **Audio Integration**: Music and sound effect management
- **Asset Loading**: Resource management

### Software Engineering
- **Build Systems**: CMake configuration
- **Version Control**: Git workflow
- **Code Organization**: Modular function design
- **Documentation**: Comprehensive README
- **Cross-platform Development**: Linux/Windows/macOS support
- **Debugging**: Error handling and logging

---

## 🎮 Gameplay Tips

### Beginner Tips
1. **Stay Mobile**: Keep moving to avoid getting trapped
2. **Shoot Constantly**: Maintain bullet pressure on enemies
3. **Watch Boss Bullets**: Green lasers are your biggest threat
4. **Collect Shields**: Always grab shield power-ups when available
5. **Use Invincibility**: Take calculated risks during invincibility frames

### Advanced Strategies
1. **Positioning**: Center yourself between enemy columns for maximum coverage
2. **Bullet Management**: Time shots to hit multiple stacked enemies
3. **Boss Priority**: Focus fire on bosses before they accumulate
4. **Shield Timing**: Save shields for boss encounters
5. **Pattern Recognition**: Learn spawn patterns to predict danger zones

### Speedrun Tips
1. **Optimize Movement**: Minimize unnecessary horizontal movement
2. **Shoot Early**: Fire before enemies fully appear
3. **Risk Management**: Use aggressive positioning in early levels
4. **Level 5 Strategy**: Stay centered, prioritize bosses
5. **Save Shield**: Keep shield active through critical moments

---

## 📚 Additional Resources

### SFML Learning
- [Official SFML Tutorial](https://www.sfml-dev.org/tutorials/2.5/)
- [SFML Documentation](https://www.sfml-dev.org/documentation/2.5.1/)
- [SFML Game Development Book](https://www.packtpub.com/game-development/sfml-game-development)

### C++ Resources
- [C++ Reference](https://en.cppreference.com/)
- [Learn C++](https://www.learncpp.com/)
- [C++ Game Development Patterns](https://gameprogrammingpatterns.com/)

### Game Development
- [Game Programming Patterns](https://gameprogrammingpatterns.com/)
- [Gamasutra Articles](https://www.gamasutra.com/)
- [Indie Game Development Resources](https://www.reddit.com/r/gamedev/)

---

<div align="center">

## 🌟 Star this repository if you found it helpful!

**Made with ❤️ and C++**

[![GitHub stars](https://img.shields.io/github/stars/Abdurrafay19/space_shooter?style=social)](https://github.com/Abdurrafay19/space_shooter/stargazers)
[![GitHub forks](https://img.shields.io/github/forks/Abdurrafay19/space_shooter?style=social)](https://github.com/Abdurrafay19/space_shooter/network/members)

**[⬆ Back to Top](#-space-shooter-game)**

</div>
