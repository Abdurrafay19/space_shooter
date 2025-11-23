// Include SFML libraries for graphics and audio functionality
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
// Include standard C++ libraries for input/output, file handling, and random numbers
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <ctime>
// Use standard and SFML namespaces to simplify code syntax
using namespace std;
using namespace sf;

// Grid Setup: Defines the dimensions and layout of the game grid
const int ROWS = 23;
const int COLS = 15;
const int CELL_SIZE = 40;
const int MARGIN = 40;                                               // Margin around the grid for UI spacing
const float BULLET_OFFSET_X = (CELL_SIZE - CELL_SIZE * 0.3f) / 2.0f; // Center bullets horizontally
const float SHIELD_OFFSET = CELL_SIZE * -0.15f;                      // Center shield overlay

// Game States: Constants representing different screens/states of the game
const int STATE_MENU = 0;
const int STATE_PLAYING = 1;
const int STATE_INSTRUCTIONS = 2;
const int STATE_GAME_OVER = 3;
const int STATE_LEVEL_UP = 4;
const int STATE_VICTORY = 5;
const int STATE_PAUSED = 6;

// Main function: Entry point of the program where game execution begins
int main()
{

    // Random Number Generator Setup: Initialize random number generation using current time
    // This ensures different random sequences each time the game runs
    srand(static_cast<unsigned int>(time(0)));

    // Window Setup: Calculate the total window dimensions
    // Width includes: game grid + margins + extra space for UI panel on the right
    const int windowWidth = COLS * CELL_SIZE + MARGIN * 2 + 500; // Extra 500px for side panel (score, lives, etc.)
    // Height includes: game grid + top and bottom margins
    const int windowHeight = ROWS * CELL_SIZE + MARGIN * 2;
    // Create the game window with calculated dimensions and set the title
    RenderWindow window(VideoMode(windowWidth, windowHeight), "Space Shooter");
    // Limit frame rate to 60 FPS to ensure smooth and consistent gameplay
    window.setFramerateLimit(60);

    // Save File Handling: Manage persistent game data (high scores and saved games)
    // Initialize variables to store loaded data from save file
    int highScore = 0;        // Best score ever achieved by the player
    int savedLives = 0;       // Lives remaining in saved game
    int savedScore = 0;       // Score achieved in saved game
    int savedLevel = 0;       // Level reached in saved game
    bool hasSavedGame = false; // Flag indicating if a valid saved game exists
    char saveFile[] = "save-file.txt"; // Path to the save file

    // Attempt to open and read the save file
    ifstream inputFile(saveFile);
    if (inputFile.is_open())
    {
        // File format: highScore lives score level (space-separated values)
        // Read all four values from the file
        inputFile >> highScore >> savedLives >> savedScore >> savedLevel;
        inputFile.close();
        // Validate if saved game is usable (must have valid level and lives)
        if (savedLevel > 0 && savedLives > 0)
        {
            hasSavedGame = true; // Mark that a valid saved game exists
        }
    }
    else
    {
        // If save file doesn't exist, create it with default values
        // Format: "0 0 0 0" means no high score and no saved game
        ofstream createFile(saveFile);
        if (createFile.is_open())
        {
            createFile << "0 0 0 0"; // Initialize with zeros
            createFile.close();
        }
    }

    // Game Variables Setup: Initialize all core game state variables
    int currentState = STATE_MENU;             // Start at main menu screen
    int selectedMenuItem = 0;                  // Currently highlighted menu option (0 = first item)
    int lives = 3;                             // Player starts with 3 lives
    int score = 0;                             // Current game score (points earned from destroying entities)
    int killCount = 0;                         // Number of enemies/bosses destroyed in current level
    int level = 1;                             // Current difficulty level (starts at 1)
    const int MAX_LEVEL = 5;                   // Final level number - winning the game requires completing level 5
    bool isInvincible = false;                 // Temporary invincibility flag (activated after taking damage)
    Clock invincibilityTimer;                  // Timer to track how long invincibility has been active
    const float INVINCIBILITY_DURATION = 2.0f; // Invincibility lasts 2 seconds after being hit

    // Level Up Effect Setup: Variables to create animated blinking text during level transitions
    Clock levelUpTimer;            // Tracks total time spent on level up screen (2 seconds)
    bool levelUpBlinkState = true; // Controls text visibility - alternates between true/false for blinking
    Clock levelUpBlinkClock;       // Controls blink timing (toggles every 0.3 seconds)

    // Boss Mechanics Setup: Counter to control boss firing frequency
    // Bosses fire bullets every few movements based on current level difficulty
    int bossMoveCounter = 0;

    // Grid System: 2D array representing the game board - this is the core game state
    // Each cell can contain one entity (or be empty). Cell values represent:
    // 0 = Empty space (nothing there)
    // 1 = Player's spaceship
    // 2 = Meteor (obstacle that gives points when destroyed)
    // 3 = Player's bullet (moving upward)
    // 4 = Enemy UFO (moving downward)
    // 5 = Boss enemy ship (tougher enemy, appears at level 3+)
    // 6 = Boss bullet (enemy projectile moving downward)
    // Note: Shield powerups and active shields don't occupy grid cells (tracked separately)
    int grid[ROWS][COLS] = {0}; // Initialize entire grid to empty

    // Shield Powerup System: Separate tracking for shield collectibles (independent from main grid)
    // This prevents powerups from interfering with game entity collision detection
    const int MAX_SHIELD_POWERUPS = 5;         // Maximum number of simultaneous shield powerups on screen
    int shieldPowerupRow[MAX_SHIELD_POWERUPS] = {-1, -1, -1, -1, -1};      // Row position of each powerup
    int shieldPowerupCol[MAX_SHIELD_POWERUPS] = {-1, -1, -1, -1, -1};      // Column position of each powerup
    bool shieldPowerupActive[MAX_SHIELD_POWERUPS] = {false, false, false, false, false}; // Whether each slot is in use
    int shieldPowerupDirection[MAX_SHIELD_POWERUPS] = {0, 0, 0, 0, 0};     // Movement direction (currently unused, all move down)

    // Shield System: Track whether player currently has an active shield
    // When true, the shield will absorb the next hit and then deactivate
    bool hasShield = false;

    // Hit Effect System: Manage visual explosion effects when entities are destroyed
    // These arrays track multiple simultaneous explosion animations
    const int MAX_HIT_EFFECTS = 50;            // Maximum simultaneous explosion effects
    int hitEffectRow[MAX_HIT_EFFECTS] = {0};   // Row position of each effect
    int hitEffectCol[MAX_HIT_EFFECTS] = {0};   // Column position of each effect
    float hitEffectTimer[MAX_HIT_EFFECTS] = {0.0f}; // How long each effect has been displayed
    bool hitEffectActive[MAX_HIT_EFFECTS] = {false}; // Whether each effect slot is currently in use
    const float HIT_EFFECT_DURATION = 0.3f;    // Each explosion displays for 0.3 seconds before disappearing

    // Spaceship Initialization: Set up player's spaceship at starting position
    int spaceshipCol = COLS / 2;      // Place spaceship in center column (middle of bottom row)
    grid[ROWS - 1][spaceshipCol] = 1; // Mark the bottom-center grid cell as containing the spaceship
    // Load spaceship texture from file
    Texture spaceshipTexture;
    if (!spaceshipTexture.loadFromFile("assets/images/player.png"))
    {
        // If texture fails to load, print error and exit program
        cerr << "Failed to load spaceship texture" << endl;
        return -1;
    }
    // Create sprite object for the spaceship
    Sprite spaceship;
    spaceship.setTexture(spaceshipTexture);
    // Scale sprite to exactly fit within one grid cell
    // Calculate scale factors: CELL_SIZE divided by original texture dimensions
    spaceship.setScale(
        static_cast<float>(CELL_SIZE) / spaceshipTexture.getSize().x,
        static_cast<float>(CELL_SIZE) / spaceshipTexture.getSize().y);

    // Life Icon Setup: Small heart/ship icons displayed in UI to show remaining lives
    Texture lifeTexture;
    if (!lifeTexture.loadFromFile("assets/images/life.png"))
    {
        // Exit if life icon texture cannot be loaded
        cerr << "Failed to load life texture" << endl;
        return -1;
    }
    Sprite lifeIcon;
    lifeIcon.setTexture(lifeTexture);
    // Scale life icon to small size (24x24 pixels) to fit nicely next to UI text
    // This makes the icon size consistent with text height
    lifeIcon.setScale(
        24.0f / lifeTexture.getSize().x,
        24.0f / lifeTexture.getSize().y);

    // Shield PowerUp and Shield Setup: Load graphics for shield system
    // First, load the active shield overlay (displayed over spaceship when shield is active)
    Texture shieldTexture;
    if (!shieldTexture.loadFromFile("assets/images/shield.png"))
    {
        cerr << "Failed to load shield texture" << endl;
        return -1;
    }
    Sprite shieldIcon;
    shieldIcon.setTexture(shieldTexture);

    // Second, load the shield powerup collectible (falls from top of screen)
    Texture shieldPowerUpTexture;
    if (!shieldPowerUpTexture.loadFromFile("assets/images/shield-powerup.png"))
    {
        cerr << "Failed to load shield power-up texture" << endl;
        return -1;
    }
    Sprite shieldPowerUp;
    shieldPowerUp.setTexture(shieldPowerUpTexture);
    // Scale powerup to fit exactly within one grid cell
    shieldPowerUp.setScale(
        static_cast<float>(CELL_SIZE) / shieldPowerUpTexture.getSize().x,
        static_cast<float>(CELL_SIZE) / shieldPowerUpTexture.getSize().y);

    // Scale active shield overlay to be 30% larger than spaceship for visibility
    // This creates a protective bubble effect around the player
    shieldIcon.setScale(
        static_cast<float>(CELL_SIZE * 1.3f) / shieldTexture.getSize().x,
        static_cast<float>(CELL_SIZE * 1.3f) / shieldTexture.getSize().y);

    // Game Background Setup: Load and prepare the playing area background image
    Texture bgTexture;
    if (!bgTexture.loadFromFile("assets/images/backgroundColor.png"))
    {
        cerr << "Failed to load background texture" << endl;
        return -1;
    }
    Sprite background;
    background.setTexture(bgTexture);
    // Scale background to exactly cover the entire game grid (all rows and columns)
    // This stretches or shrinks the background image to fit perfectly
    background.setScale(
        static_cast<float>(COLS * CELL_SIZE) / bgTexture.getSize().x,
        static_cast<float>(ROWS * CELL_SIZE) / bgTexture.getSize().y);
    // Position background at top-left of game grid (accounting for margin)
    background.setPosition(MARGIN, MARGIN);

    // Game Border Setup: Create a decorative border around the game grid
    // Size matches the grid dimensions exactly
    RectangleShape gameBox(Vector2f(COLS * CELL_SIZE, ROWS * CELL_SIZE));
    gameBox.setFillColor(Color::Transparent);  // No fill - just an outline
    gameBox.setOutlineThickness(5);             // 5-pixel thick border
    gameBox.setOutlineColor(Color::Black);      // Black colored border
    gameBox.setPosition(MARGIN, MARGIN);        // Position at top-left of grid

    // Meteor Entity Setup: Load meteor obstacle graphics
    // Meteors are obstacles that fall from the top and can be destroyed for points
    Texture meteorTexture;
    if (!meteorTexture.loadFromFile("assets/images/meteorSmall.png"))
    {
        cerr << "Failed to load meteor texture" << endl;
        return -1;
    }
    Sprite meteor;
    meteor.setTexture(meteorTexture);
    // Scale meteor to fit one grid cell
    meteor.setScale(
        static_cast<float>(CELL_SIZE) / meteorTexture.getSize().x,
        static_cast<float>(CELL_SIZE) / meteorTexture.getSize().y);

    /// Enemy Entities Setup: Load graphics for all enemy types

    // Standard Enemy Setup: Regular UFO enemies (appear at all levels)
    // These move downward and must be destroyed to progress
    Texture enemyTexture;
    if (!enemyTexture.loadFromFile("assets/images/enemyUFO.png"))
    {
        cerr << "Failed to load enemy texture" << endl;
        return -1;
    }
    Sprite enemy;
    enemy.setTexture(enemyTexture);
    // Scale enemy UFO to fit one grid cell
    enemy.setScale(
        static_cast<float>(CELL_SIZE) / enemyTexture.getSize().x,
        static_cast<float>(CELL_SIZE) / enemyTexture.getSize().y);

    // Boss Enemy Setup: Tougher enemies that appear starting at level 3
    // Bosses are worth more points and can shoot bullets at the player
    Texture bossEnemyTexture;
    if (!bossEnemyTexture.loadFromFile("assets/images/enemyShip.png"))
    {
        cerr << "Failed to load boss enemy texture" << endl;
        return -1;
    }
    Sprite bossEnemy;
    bossEnemy.setTexture(bossEnemyTexture);
    // Scale boss to fit one grid cell
    bossEnemy.setScale(
        static_cast<float>(CELL_SIZE) / bossEnemyTexture.getSize().x,
        static_cast<float>(CELL_SIZE) / bossEnemyTexture.getSize().y);

    // Player Bullet Setup: Projectiles fired by the player
    // Bullets move upward and destroy enemies/meteors on contact
    Texture bulletTexture;
    if (!bulletTexture.loadFromFile("assets/images/laserRed.png"))
    {
        cerr << "Failed to load bullet texture" << endl;
        return -1;
    }
    Sprite bullet;
    bullet.setTexture(bulletTexture);
    // Scale bullet to be thin and elongated (narrower than full cell)
    // Width: 30% of cell width, Height: 80% of cell height
    bullet.setScale(
        static_cast<float>(CELL_SIZE * 0.3f) / bulletTexture.getSize().x,
        static_cast<float>(CELL_SIZE * 0.8f) / bulletTexture.getSize().y);

    // Bullet Impact Effect Setup: Visual explosion effect when bullets hit targets
    // Displays briefly when player bullets destroy enemies, meteors, or collide with boss bullets
    Texture bulletHitTexture;
    if (!bulletHitTexture.loadFromFile("assets/images/laserRedShot.png"))
    {
        cerr << "Failed to load bullet hit texture" << endl;
        return -1;
    }
    Sprite bulletHit;
    bulletHit.setTexture(bulletHitTexture);
    // Scale impact effect to fill entire cell for maximum visual impact
    bulletHit.setScale(
        static_cast<float>(CELL_SIZE) / bulletHitTexture.getSize().x,
        static_cast<float>(CELL_SIZE) / bulletHitTexture.getSize().y);

    // Boss Bullet Setup: Enemy projectiles fired by boss enemies
    // These bullets move downward toward the player and must be avoided
    Texture bossBulletTexture;
    if (!bossBulletTexture.loadFromFile("assets/images/laserGreen.png"))
    {
        cerr << "Failed to load boss bullet texture" << endl;
        return -1;
    }
    Sprite bossBullet;
    bossBullet.setTexture(bossBulletTexture);
    // Scale boss bullet similar to player bullet (thin and elongated)
    // Colored green to differentiate from player's red bullets
    bossBullet.setScale(
        static_cast<float>(CELL_SIZE * 0.3f) / bossBulletTexture.getSize().x,
        static_cast<float>(CELL_SIZE * 0.8f) / bossBulletTexture.getSize().y);

    // Boss Bullet Impact Effect Setup: Explosion effect for boss bullet collisions
    // Displayed when boss bullets hit the player or are destroyed by player bullets
    Texture bossBulletHitTexture;
    if (!bossBulletHitTexture.loadFromFile("assets/images/laserGreenShot.png"))
    {
        cerr << "Failed to load boss bullet hit texture" << endl;
        return -1;
    }
    Sprite bossBulletHit;
    bossBulletHit.setTexture(bossBulletHitTexture);
    // Scale to fill entire cell (matches other impact effects)
    bossBulletHit.setScale(
        static_cast<float>(CELL_SIZE) / bossBulletHitTexture.getSize().x,
        static_cast<float>(CELL_SIZE) / bossBulletHitTexture.getSize().y);

    // Main Menu Background Setup: Starfield background for menu screens
    // Used in main menu, game over, victory, and instructions screens
    Texture menuBgTexture;
    if (!menuBgTexture.loadFromFile("assets/images/starBackground.png"))
    {
        cerr << "Failed to load menu background texture" << endl;
        return -1;
    }
    Sprite menuBackground;
    menuBackground.setTexture(menuBgTexture);
    // Scale background to cover entire window (larger than game grid)
    menuBackground.setScale(
        static_cast<float>(windowWidth) / menuBgTexture.getSize().x,
        static_cast<float>(windowHeight) / menuBgTexture.getSize().y);
    // Position at window origin (0,0)
    menuBackground.setPosition(0, 0);

    // Font Loading: Load the custom font used for all text in the game
    // This font will be used for menus, UI elements, and all on-screen text
    Font font;
    if (!font.loadFromFile("assets/fonts/font.ttf"))
    {
        cerr << "Failed to load font" << endl;
        return -1;
    }

    // Sound Effects Setup: Load and configure background music
    // Music plays continuously during menus (stops during gameplay)
    Music bgMusic;
    if (!bgMusic.openFromFile("assets/sounds/bg-music.mp3"))
    {
        cerr << "Failed to load background music" << endl;
        return -1;
    }
    bgMusic.setLoop(true);  // Repeat music endlessly
    bgMusic.setVolume(30);  // Set volume to 30% (not too loud)
    bgMusic.play();         // Start playing immediately

    // Sound Buffers: Load all sound effect files into memory
    // Buffers store the raw audio data that Sound objects will play
    SoundBuffer shootBuffer, explosionBuffer, damageBuffer, levelUpBuffer;
    SoundBuffer menuClickBuffer, menuNavBuffer, winBuffer, loseBuffer;

    // Load shooting sound (played when player fires bullets)
    if (!shootBuffer.loadFromFile("assets/sounds/shoot.wav"))
    {
        cerr << "Failed to load shoot sound" << endl;
        return -1;
    }
    // Load explosion sound (played when enemies/meteors are destroyed)
    if (!explosionBuffer.loadFromFile("assets/sounds/explosion.wav"))
    {
        cerr << "Failed to load explosion sound" << endl;
        return -1;
    }
    // Load damage sound (played when player is hit)
    if (!damageBuffer.loadFromFile("assets/sounds/damage.mp3"))
    {
        cerr << "Failed to load damage sound" << endl;
        return -1;
    }
    // Load level up sound (played when advancing to next level or collecting shield)
    if (!levelUpBuffer.loadFromFile("assets/sounds/level-up.mp3"))
    {
        cerr << "Failed to load level up sound" << endl;
        return -1;
    }
    // Load menu click sound (played when selecting menu items)
    if (!menuClickBuffer.loadFromFile("assets/sounds/menu-click.mp3"))
    {
        cerr << "Failed to load menu click sound" << endl;
        return -1;
    }
    // Load menu navigation sound (played when moving between menu options)
    if (!menuNavBuffer.loadFromFile("assets/sounds/menu-navigate.wav"))
    {
        cerr << "Failed to load menu navigate sound" << endl;
        return -1;
    }
    // Load victory sound (played when completing level 5)
    if (!winBuffer.loadFromFile("assets/sounds/win.wav"))
    {
        cerr << "Failed to load win sound" << endl;
        return -1;
    }
    // Load game over sound (played when all lives are lost)
    if (!loseBuffer.loadFromFile("assets/sounds/lose.wav"))
    {
        cerr << "Failed to load lose sound" << endl;
        return -1;
    }

    // Sound Objects: Create playable sound objects and link them to buffers
    // These objects can be played multiple times using their associated buffers
    Sound shootSound, explosionSound, damageSound, levelUpSound;
    Sound menuClickSound, menuNavSound, winSound, loseSound;

    // Associate each sound with its corresponding buffer
    shootSound.setBuffer(shootBuffer);
    explosionSound.setBuffer(explosionBuffer);
    damageSound.setBuffer(damageBuffer);
    levelUpSound.setBuffer(levelUpBuffer);
    menuClickSound.setBuffer(menuClickBuffer);
    menuNavSound.setBuffer(menuNavBuffer);
    winSound.setBuffer(winBuffer);
    loseSound.setBuffer(loseBuffer);

    // Main Menu Title Setup: Large title text displayed at top of main menu
    Text menuTitle("SPACE SHOOTER", font, 40);  // Text content, font, and size (40pt)
    menuTitle.setFillColor(Color::Yellow);       // Make title stand out with yellow color
    // Center title horizontally on screen, position at Y=100 pixels from top
    // Calculate X position by: (window width / 2) - (text width / 2)
    menuTitle.setPosition(windowWidth / 2 - menuTitle.getLocalBounds().width / 2.0f, 100);

    // Main Menu Items Setup: Create array of menu options
    Text menuItems[4];  // Array holds 4 menu options
    // Define text for each menu option
    const char menuTexts[4][20] = {"Start Game", "Load Saved Game", "Instructions", "Exit"};
    // Configure each menu item
    for (int i = 0; i < 4; i++)
    {
        menuItems[i].setFont(font);              // Apply game font
        menuItems[i].setString(menuTexts[i]);    // Set text content
        menuItems[i].setCharacterSize(28);       // Slightly smaller than title (28pt)
        menuItems[i].setFillColor(Color::White); // Default white (changes to yellow when selected)
        // Center each menu item horizontally, space them vertically
        // Y position: 260 + (index * 56) creates even spacing between items
        menuItems[i].setPosition(windowWidth / 2 - menuItems[i].getLocalBounds().width / 2.0f, 260 + i * 56);
    }

    // High Score Display for Main Menu: Shows best score below title
    Text menuHighScoreText("High Score: 0", font, 24);  // Initial text (updated dynamically)
    menuHighScoreText.setFillColor(Color::Yellow);       // Yellow to match title importance
    // Center horizontally, position between title and menu items
    menuHighScoreText.setPosition(windowWidth / 2 - menuHighScoreText.getLocalBounds().width / 2.0f, 180);

    // Menu Navigation Instructions: Help text at bottom of screen
    Text menuInstructions("Use UP/DOWN or W/S to navigate  |  ENTER to select", font, 18);
    menuInstructions.setFillColor(Color(150, 150, 150)); // Subtle gray color (less distracting)
    // Center horizontally, position near bottom of window
    menuInstructions.setPosition(windowWidth / 2 - menuInstructions.getLocalBounds().width / 2.0f, windowHeight - 80);

    /// In-Game UI Elements Setup: Text displayed in right panel during gameplay

    // Game Title: Displayed at top of right panel during gameplay
    Text title("Space  Shooter", font, 28);
    title.setFillColor(Color::Yellow);  // Yellow for visual prominence
    // Position in right panel: grid width + margins + 20px padding
    title.setPosition(MARGIN + COLS * CELL_SIZE + 20, MARGIN);

    // Lives Display Text: Label for life icons (icons drawn separately)
    Text livesText("Lives:", font, 20);
    livesText.setFillColor(Color::White);
    // Position below title in right panel
    livesText.setPosition(MARGIN + COLS * CELL_SIZE + 20, MARGIN + 150);

    // Score Display Text: Shows current points earned
    Text scoreText("Score: 0", font, 20);  // Updated dynamically during gameplay
    scoreText.setFillColor(Color::White);
    // Position below lives in right panel
    scoreText.setPosition(MARGIN + COLS * CELL_SIZE + 20, MARGIN + 200);

    // Enemies Killed Display Text: Shows progress toward level completion
    Text killsText("Kills: 0/10", font, 20);  // Format: current/required kills
    killsText.setFillColor(Color::White);
    // Position below score in right panel
    killsText.setPosition(MARGIN + COLS * CELL_SIZE + 20, MARGIN + 230);

    // Level Display Text: Shows current level number
    Text levelText("Level: 1", font, 20);  // Updated when level changes
    levelText.setFillColor(Color::White);
    // Position below kills in right panel
    levelText.setPosition(MARGIN + COLS * CELL_SIZE + 20, MARGIN + 280);

    // High Score Display: Shows best score during gameplay (for comparison)
    Text highScoreText("High Score: 0", font, 20);
    highScoreText.setFillColor(Color::Yellow);  // Yellow to stand out
    // Position below level in right panel
    highScoreText.setPosition(MARGIN + COLS * CELL_SIZE + 20, MARGIN + 330);

    // Game Over Screen Setup: Elements displayed when player loses all lives
    Text gameOverTitle("GAME OVER", font, 40);
    gameOverTitle.setFillColor(Color::Red);  // Red color emphasizes failure
    // Center title at top of screen
    gameOverTitle.setPosition(windowWidth / 2 - gameOverTitle.getLocalBounds().width / 2.0f, 100);

    // Game Over Score Display: Shows final score achieved in lost game
    Text gameOverScore("Final Score: 0", font, 28);  // Updated with actual score
    gameOverScore.setFillColor(Color::Yellow);
    // Center below title
    gameOverScore.setPosition(windowWidth / 2 - gameOverScore.getLocalBounds().width / 2.0f, 200);

    // Game Over Menu Items: Options after losing
    Text gameOverItems[2];  // Only 2 options: Restart or Main Menu
    const char gameOverTexts[2][20] = {"Restart", "Main Menu"};
    for (int i = 0; i < 2; i++)
    {
        gameOverItems[i].setFont(font);
        gameOverItems[i].setString(gameOverTexts[i]);
        gameOverItems[i].setCharacterSize(28);
        gameOverItems[i].setFillColor(Color::White);  // Changes to yellow when selected
        // Center horizontally, space vertically below score
        gameOverItems[i].setPosition(windowWidth / 2 - gameOverItems[i].getLocalBounds().width / 2.0f, 300 + i * 56);
    }

    // Game Over Navigation Instructions: Help text at bottom
    Text gameOverInstructions("Use UP/DOWN or W/S to navigate  |  ENTER to select", font, 18);
    gameOverInstructions.setFillColor(Color(150, 150, 150));  // Subtle gray
    // Center at bottom of screen
    gameOverInstructions.setPosition(windowWidth / 2 - gameOverInstructions.getLocalBounds().width / 2.0f, windowHeight - 80);

    // Level Up Screen Setup: Displayed briefly (2 seconds) when advancing levels
    // Positioned in center of game grid (not full window)
    Text levelUpText("LEVEL UP!", font, 40);
    levelUpText.setFillColor(Color::Green);  // Green indicates success/progress
    // Calculate center point of the game grid
    float gridCenterX = MARGIN + (COLS * CELL_SIZE) / 2.0f;
    float gridCenterY = MARGIN + (ROWS * CELL_SIZE) / 2.0f;
    // Center text in middle of grid
    levelUpText.setPosition(gridCenterX - levelUpText.getLocalBounds().width / 2.0f, gridCenterY - levelUpText.getLocalBounds().height / 2.0f - 10);

    // Pause Screen Setup: Overlay displayed when player presses P
    Text pauseTitle("PAUSED", font, 40);
    pauseTitle.setFillColor(Color::Cyan);  // Cyan to differentiate from other states
    // Position above grid center
    pauseTitle.setPosition(gridCenterX - pauseTitle.getLocalBounds().width / 2.0f, gridCenterY - 200);

    // Pause Menu Items: Three options when paused
    Text pauseItems[3];
    const char pauseTexts[3][20] = {"Resume", "Restart", "Save & Quit"};
    for (int i = 0; i < 3; i++)
    {
        pauseItems[i].setFont(font);
        pauseItems[i].setString(pauseTexts[i]);
        pauseItems[i].setCharacterSize(28);
        pauseItems[i].setFillColor(Color::White);  // Changes to yellow when selected
        // Center horizontally in grid, space vertically
        pauseItems[i].setPosition(gridCenterX - pauseItems[i].getLocalBounds().width / 2.0f, gridCenterY - 50 + i * 56);
    }

    // Victory Screen Setup: Displayed when completing level 5
    Text victoryTitle("VICTORY!", font, 40);
    victoryTitle.setFillColor(Color::Yellow);  // Yellow for celebration
    // Center at top of full window
    victoryTitle.setPosition(windowWidth / 2 - victoryTitle.getLocalBounds().width / 2.0f, 100);

    // Victory Score Display: Shows final score after winning
    Text victoryScore("Final Score: 0", font, 28);
    victoryScore.setFillColor(Color::White);
    // Center below victory title
    victoryScore.setPosition(windowWidth / 2 - victoryScore.getLocalBounds().width / 2.0f, 200);

    // Victory Menu Items: Options after winning
    Text victoryItems[2];
    const char victoryTexts[2][20] = {"Restart", "Main Menu"};
    for (int i = 0; i < 2; i++)
    {
        victoryItems[i].setFont(font);
        victoryItems[i].setString(victoryTexts[i]);
        victoryItems[i].setCharacterSize(28);
        victoryItems[i].setFillColor(Color::White);
        // Center horizontally, space vertically
        victoryItems[i].setPosition(windowWidth / 2 - victoryItems[i].getLocalBounds().width / 2.0f, 300 + i * 56);
    }

    // Victory Navigation Instructions
    Text victoryInstructions("Use UP/DOWN or W/S to navigate  |  ENTER to select", font, 18);
    victoryInstructions.setFillColor(Color(150, 150, 150));
    // Center at bottom of screen
    victoryInstructions.setPosition(windowWidth / 2 - victoryInstructions.getLocalBounds().width / 2.0f, windowHeight - 80);

    // Instructions Screen Setup: Detailed help screen explaining game mechanics
    // Main title at top
    Text instructionsTitle("HOW TO PLAY", font, 40);
    instructionsTitle.setFillColor(Color::Yellow);  // Yellow for visibility
    // Center horizontally, position near top
    instructionsTitle.setPosition(windowWidth / 2 - instructionsTitle.getLocalBounds().width / 2.0f, 40);

    // Control Instructions Section: Explains keyboard controls
    Text controlsTitle("CONTROLS", font, 24);  // Section header
    controlsTitle.setFillColor(Color::Cyan);    // Cyan to differentiate sections
    // Left-aligned, positioned below main title
    controlsTitle.setPosition(50, 100);

    // Movement controls explanation
    Text moveText("Move Left/Right: A/D or Arrow Keys", font, 18);
    moveText.setFillColor(Color::White);
    // Indented under CONTROLS section
    moveText.setPosition(50, 140);

    // Shooting controls explanation
    Text shootText("Shoot: SPACEBAR", font, 18);
    shootText.setFillColor(Color::White);
    // Below movement text
    shootText.setPosition(50, 170);

    // Pause controls explanation
    Text pauseText("Pause: P", font, 18);
    pauseText.setFillColor(Color::White);
    // Below shooting text
    pauseText.setPosition(50, 200);

    // Entities Explanation Section: Describes all game objects with sprites
    Text entitiesTitle("ENTITIES", font, 24);  // Section header
    entitiesTitle.setFillColor(Color::Cyan);
    // Below CONTROLS section
    entitiesTitle.setPosition(50, 250);

    // Player spaceship description (sprite shown at X=60)
    Text playerDesc("Your Ship", font, 18);
    playerDesc.setFillColor(Color::White);
    // Indented to leave room for sprite display
    playerDesc.setPosition(120, 290);

    // Meteor description with point value
    Text meteorDesc("Meteor - 1 Point (Avoid collision!)", font, 18);
    meteorDesc.setFillColor(Color::White);
    meteorDesc.setPosition(120, 330);

    // Enemy UFO description with point value
    Text enemyDesc("Enemy - 3 Points (Avoid collision!)", font, 18);
    enemyDesc.setFillColor(Color::White);
    enemyDesc.setPosition(120, 370);

    // Boss enemy description with level requirement
    Text bossDesc("Boss - 5 Points (Level 3+) (Avoid collision!)", font, 18);
    bossDesc.setFillColor(Color::White);
    bossDesc.setPosition(120, 410);

    // Player bullet description
    Text bulletDesc("Your Bullet", font, 18);
    bulletDesc.setFillColor(Color::White);
    bulletDesc.setPosition(120, 450);

    // Boss bullet description (warning to avoid)
    Text bossBulletDesc("Boss Bullet - Avoid!", font, 18);
    bossBulletDesc.setFillColor(Color::White);
    bossBulletDesc.setPosition(120, 490);

    // Life icon description (explains UI element)
    Text lifeDesc("Life Icon - Indicates remaining lives", font, 18);
    lifeDesc.setFillColor(Color::White);
    lifeDesc.setPosition(120, 530);

    // Shield powerup description with level requirement
    Text shieldPowerupDesc("Shield Powerup - Absorbs 1 Hit (Level 3+)", font, 18);
    shieldPowerupDesc.setFillColor(Color::White);
    shieldPowerupDesc.setPosition(120, 570);

    // Game Systems Section: Explains core game mechanics
    Text systemsTitle("GAME SYSTEMS", font, 24);  // Section header
    systemsTitle.setFillColor(Color::Cyan);
    // Below ENTITIES section
    systemsTitle.setPosition(50, 620);

    // Lives system explanation
    Text livesDesc("Lives: You start with 3 lives. Lose one when hit any enemy.", font, 18);
    livesDesc.setFillColor(Color::White);
    // Under GAME SYSTEMS header
    livesDesc.setPosition(50, 660);

    // Level progression explanation
    Text levelsDesc("Levels: Destroy 10 enemies/bosses per level to advance.", font, 18);
    levelsDesc.setFillColor(Color::White);
    // Below lives description
    levelsDesc.setPosition(50, 690);

    // High score save system explanation
    Text highScoreDesc("High Score: Your best score is saved automatically.", font, 18);
    highScoreDesc.setFillColor(Color::White);
    // Below levels description
    highScoreDesc.setPosition(50, 720);

    // Objective Section: States win/loss conditions
    Text objectiveTitle("OBJECTIVE", font, 24);  // Section header
    objectiveTitle.setFillColor(Color::Cyan);
    // Below GAME SYSTEMS section
    objectiveTitle.setPosition(50, 770);

    // Primary objective: destroy enemies
    Text objective1("- Destroy enemies and bosses", font, 18);
    objective1.setFillColor(Color::White);
    // Under OBJECTIVE header
    objective1.setPosition(50, 810);

    // Loss condition: don't lose all lives
    Text objective2("- Do not lose all your lives", font, 18);
    objective2.setFillColor(Color::White);
    // Below first objective
    objective2.setPosition(50, 840);

    // Victory condition: complete level 5
    Text objective3("- Complete Level 5 to win!", font, 18);
    objective3.setFillColor(Color::White);
    // Below second objective
    objective3.setPosition(50, 870);

    // Empty text placeholder (not currently used)
    Text objective4("", font, 18);
    objective4.setFillColor(Color::White);
    objective4.setPosition(50, 900);

    // Instructions to return to menu
    Text instructionsBack("Press ESC or BACKSPACE to return to menu", font, 18);
    instructionsBack.setFillColor(Color(150, 150, 150));  // Subtle gray
    // Centered at bottom of screen
    instructionsBack.setPosition(windowWidth / 2 - instructionsBack.getLocalBounds().width / 2.0f, windowHeight - 80);

    // Game Timing Clocks Setup: Manage all time-based game events
    // Clocks track elapsed time and control when actions occur

    // Movement Cooldown: Prevents player movement from being too sensitive
    Clock moveClock;                       // Tracks time since last movement
    Time moveCooldown = milliseconds(100); // Require 100ms between movements

    // Meteor Spawning: Controls when new meteors appear at top of screen
    Clock meteorSpawnClock;                        // Tracks time since last meteor spawn
    Clock meteorMoveClock;                         // Tracks time since meteors last moved
    float nextSpawnTime = 1.0f + (rand() % 3);    // First meteor spawns in 1-3 seconds

    // Enemy Spawning: Controls when new enemies appear
    Clock enemySpawnClock;                             // Tracks time since last enemy spawn
    Clock enemyMoveClock;                              // Tracks time since enemies last moved
    float nextEnemySpawnTime = 2.0f + (rand() % 4);   // First enemy spawns in 2-5 seconds

    // Boss Spawning: Controls when boss enemies appear (only at level 3+)
    Clock bossSpawnClock;                           // Tracks time since last boss spawn
    Clock bossMoveClock;                            // Tracks time since bosses last moved
    Clock bossBulletMoveClock;                      // Tracks time since boss bullets last moved
    float nextBossSpawnTime = 8.0f + (rand() % 5); // First boss spawns in 8-12 seconds

    // Shield Powerup Spawning: Controls when shield collectibles appear (only at level 3+)
    Clock shieldPowerupSpawnClock;                       // Tracks time since last powerup spawn
    Clock shieldPowerupMoveClock;                        // Tracks time since powerups last moved
    float nextShieldPowerupSpawnTime = 15.0f + (rand() % 10); // First powerup in 15-25 seconds

    // Bullet Firing: Controls player shooting rate
    Clock bulletMoveClock;                     // Tracks time since bullets last moved
    Clock bulletFireClock;                     // Tracks time since player last fired
    Time bulletFireCooldown = milliseconds(300); // Player can fire every 0.3 seconds

    // Hit Effect Timer: Controls explosion animation duration
    Clock hitEffectClock;  // Tracks time to update explosion effects

    // Menu Navigation Cooldown: Prevents menu selections from being too sensitive
    Clock menuClock;                       // Tracks time since last menu action
    Time menuCooldown = milliseconds(200); // Require 200ms between menu actions

    // Main Game Loop: This loop runs continuously until the player closes the window
    // Each iteration = one frame of the game (60 times per second)
    while (window.isOpen())
    {

        // Event Polling: Check for system events (window close, etc.)
        // This must be done every frame to keep the window responsive
        Event event;
        while (window.pollEvent(event))  // Process all pending events
        {
            // If user clicks the X button or uses system close command
            if (event.type == Event::Closed)
                window.close();  // Gracefully close the game window
        }

        // State Machine: Execute logic based on current game state
        // The game operates in different states (menu, playing, paused, etc.)
        if (currentState == STATE_MENU)
        {
            // Main Menu State: Handle menu navigation and option selection
            // Only process input if enough time has passed (prevents double-inputs)
            if (menuClock.getElapsedTime() >= menuCooldown)
            {
                bool menuAction = false;  // Track if any input was received this frame

                // Navigate menu UP: Move to previous menu item (with wraparound)
                if (Keyboard::isKeyPressed(Keyboard::Up) || Keyboard::isKeyPressed(Keyboard::W))
                {
                    // Modulo arithmetic for wraparound: (current - 1 + total) % total
                    selectedMenuItem = (selectedMenuItem - 1 + 4) % 4;
                    menuNavSound.play();  // Play navigation sound feedback
                    menuAction = true;
                }
                // Navigate menu DOWN: Move to next menu item (with wraparound)
                else if (Keyboard::isKeyPressed(Keyboard::Down) || Keyboard::isKeyPressed(Keyboard::S))
                {
                    selectedMenuItem = (selectedMenuItem + 1) % 4;  // Wraparound to first item
                    menuNavSound.play();
                    menuAction = true;
                }
                // Select current menu item with Enter key
                else if (Keyboard::isKeyPressed(Keyboard::Enter))
                {
                    menuClickSound.play();  // Play selection confirmation sound
                    if (selectedMenuItem == 0)
                    { // Option 0: Start New Game
                        bgMusic.stop();  // Stop menu music (no music during gameplay)
                        currentState = STATE_PLAYING;  // Switch to gameplay state
                        // Reset all game state variables to starting values
                        lives = 3;           // Start with 3 lives
                        score = 0;           // Start with 0 score
                        killCount = 0;       // No enemies killed yet
                        level = 1;           // Start at level 1
                        bossMoveCounter = 0; // Reset boss firing counter
                        hasShield = false;   // No shield at start
                        // Clear the entire game grid (remove all entities)
                        for (int r = 0; r < ROWS; r++)
                        {
                            for (int c = 0; c < COLS; c++)
                            {
                                grid[r][c] = 0;  // Set each cell to empty
                            }
                        }
                        // Deactivate all shield powerups
                        for (int i = 0; i < MAX_SHIELD_POWERUPS; i++)
                        {
                            shieldPowerupActive[i] = false;
                        }
                        // Place spaceship at starting position (bottom center)
                        spaceshipCol = COLS / 2;                // Center column
                        grid[ROWS - 1][spaceshipCol] = 1;       // Bottom row
                        // Restart all timing clocks for fresh game start
                        meteorSpawnClock.restart();
                        meteorMoveClock.restart();
                        enemySpawnClock.restart();
                        enemyMoveClock.restart();
                        bossSpawnClock.restart();
                        bossMoveClock.restart();
                        bossBulletMoveClock.restart();
                        bulletMoveClock.restart();
                        shieldPowerupSpawnClock.restart();
                        shieldPowerupMoveClock.restart();
                    }
                    else if (selectedMenuItem == 1)
                    { // Option 1: Load Saved Game
                        if (hasSavedGame)  // Only proceed if a valid save exists
                        {
                            bgMusic.stop();  // Stop menu music
                            currentState = STATE_PLAYING;  // Switch to gameplay
                            // Restore saved game state from loaded variables
                            lives = savedLives;    // Restore saved lives count
                            score = savedScore;    // Restore saved score
                            killCount = 0;         // Always reset kills (level progress)
                            level = savedLevel;    // Restore saved level
                            bossMoveCounter = 0;   // Reset boss mechanics
                            hasShield = false;     // No shield when loading
                            // Clear the grid for fresh start at saved level
                            for (int r = 0; r < ROWS; r++)
                            {
                                for (int c = 0; c < COLS; c++)
                                {
                                    grid[r][c] = 0;  // Empty all cells
                                }
                            }
                            // Clear any shield powerups
                            for (int i = 0; i < MAX_SHIELD_POWERUPS; i++)
                            {
                                shieldPowerupActive[i] = false;
                            }
                            // Place spaceship at starting position
                            spaceshipCol = COLS / 2;
                            grid[ROWS - 1][spaceshipCol] = 1;
                            // Restart all timing clocks
                            meteorSpawnClock.restart();
                            meteorMoveClock.restart();
                            enemySpawnClock.restart();
                            enemyMoveClock.restart();
                            bossSpawnClock.restart();
                            bossMoveClock.restart();
                            bossBulletMoveClock.restart();
                            bulletMoveClock.restart();
                            shieldPowerupSpawnClock.restart();
                            shieldPowerupMoveClock.restart();
                        }
                        else
                        {
                            // No saved game exists - do nothing (could show message)
                        }
                    }
                    else if (selectedMenuItem == 2)
                    { // Option 2: View Instructions
                        currentState = STATE_INSTRUCTIONS;  // Switch to instructions screen
                    }
                    else if (selectedMenuItem == 3)
                    { // Option 3: Exit Game
                        bgMusic.stop();  // Stop music before closing
                        window.close();  // Close the game window and exit program
                    }
                    menuAction = true;  // Mark that a menu action occurred
                }

                // If any input was received, restart cooldown timer
                if (menuAction)
                {
                    menuClock.restart();  // Prevents rapid repeated inputs
                }
            }

            // Visual Feedback: Update menu item colors based on selection
            // Highlighted item turns yellow, others remain white
            for (int i = 0; i < 4; i++)
            {
                if (i == selectedMenuItem)
                {
                    menuItems[i].setFillColor(Color::Yellow);  // Highlight selected
                }
                else
                {
                    menuItems[i].setFillColor(Color::White);  // Default color
                }
            }
        }  // End of STATE_MENU
        // Game Over State: Player has lost all lives
        else if (currentState == STATE_GAME_OVER)
        {
            // Game over menu navigation with cooldown (prevents accidental double-selections)
            if (menuClock.getElapsedTime() >= menuCooldown)
            {
                bool menuAction = false;  // Track if input received

                // Navigate UP through game over options (only 2 options)
                if (Keyboard::isKeyPressed(Keyboard::Up) || Keyboard::isKeyPressed(Keyboard::W))
                {
                    // Wraparound between 2 items: (current - 1 + 2) % 2
                    selectedMenuItem = (selectedMenuItem - 1 + 2) % 2;
                    menuNavSound.play();  // Audio feedback
                    menuAction = true;
                }
                // Navigate DOWN through game over options
                else if (Keyboard::isKeyPressed(Keyboard::Down) || Keyboard::isKeyPressed(Keyboard::S))
                {
                    selectedMenuItem = (selectedMenuItem + 1) % 2;  // Wraparound
                    menuNavSound.play();
                    menuAction = true;
                }
                // Select game over menu item with Enter
                else if (Keyboard::isKeyPressed(Keyboard::Enter))
                {
                    menuClickSound.play();  // Confirmation sound
                    if (selectedMenuItem == 0)
                    { // Option 0: Restart Game (start fresh from level 1)
                        currentState = STATE_PLAYING;
                        // Reset all game state to initial values
                        lives = 3;           // Restore starting lives
                        score = 0;           // Reset score to zero
                        killCount = 0;       // Reset kill counter
                        level = 1;           // Return to level 1
                        bossMoveCounter = 0; // Reset boss mechanics
                        hasShield = false;   // Remove any shield
                        // Clear the entire game grid
                        for (int r = 0; r < ROWS; r++)
                        {
                            for (int c = 0; c < COLS; c++)
                            {
                                grid[r][c] = 0;  // Empty each cell
                            }
                        }
                        // Reset spaceship to starting position
                        spaceshipCol = COLS / 2;             // Center column
                        grid[ROWS - 1][spaceshipCol] = 1;    // Bottom row
                        // Restart all game timing clocks
                        meteorSpawnClock.restart();
                        meteorMoveClock.restart();
                        enemySpawnClock.restart();
                        enemyMoveClock.restart();
                        bossSpawnClock.restart();
                        bossMoveClock.restart();
                        bossBulletMoveClock.restart();
                        bulletMoveClock.restart();
                    }
                    else if (selectedMenuItem == 1)
                    { // Option 1: Return to Main Menu
                        // Restart background music if not already playing
                        if (bgMusic.getStatus() != Music::Playing)
                        {
                            bgMusic.play();
                        }
                        currentState = STATE_MENU;  // Switch to menu state
                        selectedMenuItem = 0;        // Reset to first menu item
                    }
                    menuAction = true;
                }

                // Reset cooldown timer if input was received
                if (menuAction)
                {
                    menuClock.restart();
                }
            }

            // Visual Feedback: Highlight selected game over option
            for (int i = 0; i < 2; i++)
            {
                if (i == selectedMenuItem)
                {
                    gameOverItems[i].setFillColor(Color::Yellow);  // Highlight selected
                }
                else
                {
                    gameOverItems[i].setFillColor(Color::White);  // Default white
                }
            }
        }  // End of STATE_GAME_OVER
        // Instructions Screen State: Display help information
        else if (currentState == STATE_INSTRUCTIONS)
        {
            // Wait for player to press ESC or Backspace to return to menu
            if (menuClock.getElapsedTime() >= menuCooldown)
            {
                // Check for exit input
                if (Keyboard::isKeyPressed(Keyboard::Escape) || Keyboard::isKeyPressed(Keyboard::BackSpace))
                {
                    menuClickSound.play();       // Play sound feedback
                    currentState = STATE_MENU;   // Return to main menu
                    selectedMenuItem = 0;         // Reset selection to first item
                    menuClock.restart();          // Reset cooldown
                }
            }
        }  // End of STATE_INSTRUCTIONS
        // Gameplay State: Main game logic - this is where the action happens!
        else if (currentState == STATE_PLAYING)
        {
            // Pause Game Check: Allow player to pause at any time
            if (menuClock.getElapsedTime() >= menuCooldown)
            {
                // Press P to pause the game
                if (Keyboard::isKeyPressed(Keyboard::P))
                {
                    currentState = STATE_PAUSED;  // Switch to pause state
                    selectedMenuItem = 0;          // Default to first pause option
                    menuClock.restart();           // Reset input cooldown
                }
            }

            // Player Movement Logic: Handle horizontal spaceship movement
            // Only allow movement if cooldown has elapsed (prevents too-fast movement)
            if (moveClock.getElapsedTime() >= moveCooldown)
            {
                bool moved = false;  // Track if movement occurred this frame
                // Move Left: A key or Left Arrow
                if ((Keyboard::isKeyPressed(Keyboard::Left) || Keyboard::isKeyPressed(Keyboard::A)) && spaceshipCol > 0)
                {
                    grid[ROWS - 1][spaceshipCol] = 0; // Clear current position in grid
                    spaceshipCol--;                    // Decrease column (move left)
                    grid[ROWS - 1][spaceshipCol] = 1; // Mark new position in grid
                    moved = true;
                }
                // Move Right: D key or Right Arrow
                else if ((Keyboard::isKeyPressed(Keyboard::Right) || Keyboard::isKeyPressed(Keyboard::D)) && spaceshipCol < COLS - 1)
                {
                    grid[ROWS - 1][spaceshipCol] = 0; // Clear current position
                    spaceshipCol++;                    // Increase column (move right)
                    grid[ROWS - 1][spaceshipCol] = 1; // Mark new position
                    moved = true;
                }

                // If movement occurred, restart cooldown timer
                if (moved)
                {
                    moveClock.restart();
                }
            }

            // Player Shooting Logic: Fire bullets upward with SPACEBAR
            // Only fire if cooldown has elapsed (prevents spam)
            if (Keyboard::isKeyPressed(Keyboard::Space) && bulletFireClock.getElapsedTime() >= bulletFireCooldown)
            {
                // Spawn bullet in row directly above spaceship
                int bulletRow = ROWS - 2;  // One row above bottom
                // Only spawn if that cell is empty (no collision)
                if (bulletRow >= 0 && grid[bulletRow][spaceshipCol] == 0)
                {
                    grid[bulletRow][spaceshipCol] = 3; // Grid value 3 = player bullet
                    shootSound.play();                  // Play shooting sound effect
                }
                bulletFireCooldown = milliseconds(300);  // Set cooldown to 0.3 seconds
                bulletFireClock.restart();               // Start cooldown timer
            }

            // Meteor Spawning Logic: Randomly spawn meteors at top of screen
            // Check if enough time has passed since last meteor spawn
            if (meteorSpawnClock.getElapsedTime().asSeconds() >= nextSpawnTime)
            {
                int randomCol = rand() % COLS;  // Choose random column
                // Only spawn meteor if top row at that column is empty
                if (grid[0][randomCol] == 0)
                {
                    grid[0][randomCol] = 2; // Grid value 2 = meteor
                }
                meteorSpawnClock.restart();             // Reset spawn timer
                nextSpawnTime = 1.0f + (rand() % 3);   // Random next spawn: 1-3 seconds
            }

            // Enemy Spawning Logic: Spawn regular enemies at increasing frequency
            // Spawn rate increases with level (higher levels = more frequent enemies)
            if (enemySpawnClock.getElapsedTime().asSeconds() >= nextEnemySpawnTime)
            {
                int randomCol = rand() % COLS;  // Random column for spawn
                // Only spawn if top row cell is empty (prevent overlap)
                if (grid[0][randomCol] == 0)
                {
                    grid[0][randomCol] = 4; // Grid value 4 = enemy
                }
                enemySpawnClock.restart();  // Reset spawn timer
                // Calculate next spawn time with level-based difficulty scaling
                // Higher levels = shorter wait time between spawns
                float baseTime = 2.0f - (level * 0.35f);  // Decreases with level (more aggressive)
                float variance = 2.5f - (level * 0.35f);  // Random variation (tighter)
                // Ensure minimum spawn time (don't go below 0.5 seconds)
                if (baseTime < 0.5f)
                    baseTime = 0.5f;
                if (variance < 1.0f)
                    variance = 1.0f;
                nextEnemySpawnTime = baseTime + (rand() % (int)variance);
            }

            // Boss Spawning Logic: Spawn boss enemies (only at level 3 and above)
            // Bosses are tougher enemies that can shoot back
            if (level >= 3 && bossSpawnClock.getElapsedTime().asSeconds() >= nextBossSpawnTime)
            {
                int randomCol = rand() % COLS;  // Random column for boss spawn
                // Only spawn if top row cell is empty
                if (grid[0][randomCol] == 0)
                {
                    grid[0][randomCol] = 5; // Grid value 5 = boss
                }
                bossSpawnClock.restart();  // Reset boss spawn timer
                // Calculate next boss spawn time with level scaling
                // Higher levels = more frequent boss spawns
                float bossBaseTime = 10.0f - ((level - 3) * 1.5f);  // Decreases with level
                float bossVariance = 4.0f;  // Random variation
                // Ensure minimum time (don't spawn too frequently)
                if (bossBaseTime < 5.0f)
                    bossBaseTime = 5.0f;
                nextBossSpawnTime = bossBaseTime + (rand() % (int)bossVariance);
            }

            // Shield Powerup Spawning Logic: Rare collectible that grants protection
            // Only spawns at level 3+ (adds strategic element to higher levels)
            if (level >= 3 && shieldPowerupSpawnClock.getElapsedTime().asSeconds() >= nextShieldPowerupSpawnTime)
            {
                // Find an available slot in powerup array (max 5 simultaneous powerups)
                for (int i = 0; i < MAX_SHIELD_POWERUPS; i++)
                {
                    if (!shieldPowerupActive[i])  // Found empty slot
                    {
                        int randomCol = rand() % COLS;  // Random spawn column
                        shieldPowerupRow[i] = 0;        // Spawn at top row
                        shieldPowerupCol[i] = randomCol;
                        shieldPowerupActive[i] = true;  // Activate this powerup
                        shieldPowerupDirection[i] = 0;  // Direction: 0 = down (not used currently)
                        break;  // Only spawn one powerup at a time
                    }
                }
                shieldPowerupSpawnClock.restart();  // Reset spawn timer
                // Calculate next spawn time based on level (rarer at lower levels)
                float shieldBaseTime;
                float shieldVariance;
                if (level < 5)
                {
                    // Levels 3-4: Rare spawn (20-35 second intervals)
                    shieldBaseTime = 20.0f;
                    shieldVariance = 15.0f;
                }
                else
                {
                    // Level 5: More frequent (12-20 second intervals)
                    shieldBaseTime = 12.0f;
                    shieldVariance = 8.0f;
                }
                nextShieldPowerupSpawnTime = shieldBaseTime + (rand() % (int)shieldVariance);
            }

            // Meteor Movement Logic: Move all meteors downward at level-adjusted speed
            // Speed increases with level to make game progressively harder
            float meteorMoveSpeed = 0.7f - ((level - 1) * 0.12f);  // Base speed decreases with level (faster)
            if (meteorMoveSpeed < 0.333f)  // Cap minimum speed (max difficulty)
                meteorMoveSpeed = 0.333f;
            // Check if enough time has passed to move meteors
            if (meteorMoveClock.getElapsedTime().asSeconds() >= meteorMoveSpeed)
            {
                // Iterate from bottom to top to avoid processing same meteor twice in one frame
                // (If we went top-to-bottom, a meteor could move multiple times)
                for (int r = ROWS - 1; r >= 0; r--)
                {
                    for (int c = 0; c < COLS; c++)
                    {
                        if (grid[r][c] == 2)
                        { // Found a meteor
                            // Check if meteor reached the bottom edge
                            if (r == ROWS - 1)
                            {
                                grid[r][c] = 0; // Remove meteor (goes off screen)
                            }
                            else
                            {
                                grid[r][c] = 0; // Clear current position
                                // Attempt to move meteor down one row
                                // Can move if next cell is empty or has another meteor
                                if (grid[r + 1][c] == 0 || grid[r + 1][c] == 2)
                                {
                                    grid[r + 1][c] = 2;  // Place meteor in new position
                                }
                                // Collision with Spaceship: Meteor hits player
                                else if (grid[r + 1][c] == 1)
                                {
                                    // Check if player has active shield
                                    if (hasShield)
                                    {
                                        hasShield = false; // Shield absorbs hit and deactivates
                                        isInvincible = true;  // Activate invincibility after shield breaks
                                        invincibilityTimer.restart();  // Start invincibility timer
                                        damageSound.play();  // Play damage sound for shield break
                                    }
                                    else if (!isInvincible)  // Only damage if not in invincibility period
                                    {
                                        lives--;  // Lose one life
                                        damageSound.play();  // Play damage sound
                                        isInvincible = true;  // Activate temporary invincibility
                                        invincibilityTimer.restart();  // Start invincibility timer
                                        // Check if player has lost all lives
                                        if (lives <= 0)
                                        {
                                            // Game Over: Save high score and clear saved game
                                            if (score > highScore)  // New high score achieved
                                            {
                                                highScore = score;  // Update high score
                                                ofstream outputFile(saveFile);
                                                if (outputFile.is_open())
                                                {
                                                    // Save format: "highScore 0 0 0" (clear saved game data)
                                                    outputFile << highScore << " 0 0 0";
                                                    outputFile.close();
                                                    hasSavedGame = false;  // No saved game now
                                                }
                                            }
                                            else  // Didn't beat high score
                                            {
                                                ofstream outputFile(saveFile);
                                                if (outputFile.is_open())
                                                {
                                                    // Keep existing high score, clear saved game
                                                    outputFile << highScore << " 0 0 0";
                                                    outputFile.close();
                                                    hasSavedGame = false;
                                                }
                                            }
                                            loseSound.play();  // Play game over sound
                                            currentState = STATE_GAME_OVER;  // Switch to game over screen
                                            selectedMenuItem = 0;  // Default to first option
                                        }
                                    }
                                    grid[r + 1][c] = 0; // Remove meteor from grid
                                }
                                // Collision with Player Bullet: Meteor destroyed for points
                                else if (grid[r + 1][c] == 3)
                                {
                                    int meteorPoints = 1 + (rand() % 2); // Random 1-2 points per meteor
                                    score += meteorPoints;  // Add points to score
                                    explosionSound.play();  // Play explosion sound
                                    grid[r + 1][c] = 0; // Destroy both meteor and bullet
                                    // Create visual explosion effect at collision location
                                    for (int i = 0; i < MAX_HIT_EFFECTS; i++)
                                    {
                                        if (!hitEffectActive[i])  // Find available effect slot
                                        {
                                            hitEffectRow[i] = r + 1;     // Position of explosion
                                            hitEffectCol[i] = c;
                                            hitEffectTimer[i] = 0.0f;    // Start effect timer
                                            hitEffectActive[i] = true;   // Activate effect
                                            break;  // Only need one effect per collision
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                meteorMoveClock.restart();
            }

            // Shield Powerup Movement Logic: Move powerups down (tracked separately from main grid)
            // Powerups move slower than other entities (0.5 seconds per cell)
            if (shieldPowerupMoveClock.getElapsedTime().asSeconds() >= 0.5f)
            {
                // Process all active shield powerups
                for (int i = 0; i < MAX_SHIELD_POWERUPS; i++)
                {
                    if (shieldPowerupActive[i])  // Skip inactive slots
                    {
                        // Check if powerup has reached the bottom edge
                        if (shieldPowerupRow[i] >= ROWS - 1)
                        {
                            shieldPowerupActive[i] = false;  // Deactivate (fell off screen)
                            continue;  // Skip to next powerup
                        }

                        // Check collision with spaceship BEFORE moving
                        // (Powerup position might already overlap spaceship)
                        if (grid[shieldPowerupRow[i]][shieldPowerupCol[i]] == 1)
                        {
                            if (!hasShield) {  // Only collect if player doesn't have shield
                                hasShield = true;  // Activate player's shield
                                levelUpSound.play(); // Play collection sound (reuses level up sound)
                            }
                            shieldPowerupActive[i] = false;  // Remove powerup
                            continue;  // Skip movement for this powerup
                        }

                        // Move powerup down one row (straight down, no zigzag)
                        shieldPowerupRow[i]++;

                        // Check collision with spaceship AFTER moving
                        // (Player might now be at powerup's new position)
                        if (grid[shieldPowerupRow[i]][shieldPowerupCol[i]] == 1)
                        {
                            if (!hasShield) {  // Only collect if player doesn't have shield
                                hasShield = true;  // Grant shield to player
                                levelUpSound.play(); // Sound feedback
                            }
                            shieldPowerupActive[i] = false;  // Remove collected powerup
                        }
                    }
                }
                shieldPowerupMoveClock.restart();  // Reset movement timer
            }

            // Enemy Movement Logic: Move all enemies downward with level-based speed
            // Enemies move faster at higher levels (same speed curve as meteors)
            float enemyMoveSpeed = 0.7f - ((level - 1) * 0.12f);  // Calculate speed based on level (faster)
            // Only move enemies if enough time has passed
            if (enemyMoveClock.getElapsedTime().asSeconds() >= enemyMoveSpeed)
            {
                // Iterate from bottom to top (prevents processing same enemy twice)
                for (int r = ROWS - 1; r >= 0; r--)
                {
                    for (int c = 0; c < COLS; c++)
                    {
                        if (grid[r][c] == 4)
                        { // Found an enemy UFO
                            // Check if enemy reached bottom edge (escaped)
                            if (r == ROWS - 1)
                            {
                                grid[r][c] = 0; // Remove enemy from grid
                                lives--;        // Player loses life as penalty for letting enemy escape
                                damageSound.play();  // Play damage sound
                                isInvincible = true;  // Grant temporary invincibility
                                invincibilityTimer.restart();  // Start invincibility timer
                                // Check if player has lost all lives
                                if (lives <= 0)
                                {
                                    // Game Over: Save high score and clear saved game
                                    if (score > highScore)  // Check for new high score
                                    {
                                        highScore = score;  // Update high score
                                        ofstream outputFile(saveFile);
                                        if (outputFile.is_open())
                                        {
                                            // Save new high score, clear saved game data
                                            outputFile << highScore << " 0 0 0";
                                            outputFile.close();
                                            hasSavedGame = false;
                                        }
                                    }
                                    else  // Didn't beat high score
                                    {
                                        ofstream outputFile(saveFile);
                                        if (outputFile.is_open())
                                        {
                                            // Keep existing high score
                                            outputFile << highScore << " 0 0 0";
                                            outputFile.close();
                                            hasSavedGame = false;
                                        }
                                    }
                                    loseSound.play();  // Play game over sound
                                    currentState = STATE_GAME_OVER;  // Switch to game over screen
                                    selectedMenuItem = 0;  // Default selection
                                }
                            }
                            else  // Enemy not at bottom - continue moving
                            {
                                grid[r][c] = 0; // Clear current enemy position
                                // Move down if next cell is empty or has another enemy
                                if (grid[r + 1][c] == 0 || grid[r + 1][c] == 4)
                                {
                                    grid[r + 1][c] = 4;  // Place enemy in new position
                                }
                                // Collision with Spaceship: Enemy crashes into player
                                else if (grid[r + 1][c] == 1)
                                {
                                    // Check if player has active shield
                                    if (hasShield)
                                    {
                                        hasShield = false; // Shield absorbs the hit and deactivates
                                        isInvincible = true;  // Activate invincibility after shield breaks
                                        invincibilityTimer.restart();  // Start invincibility timer
                                        explosionSound.play();  // Play explosion sound
                                    }
                                    else if (!isInvincible)  // Only damage if not in invincibility period
                                    {
                                        lives--;  // Lose one life
                                        damageSound.play();  // Play damage sound
                                        isInvincible = true;  // Activate invincibility
                                        invincibilityTimer.restart();  // Start invincibility timer
                                        // Check if player has lost all lives
                                        if (lives <= 0)
                                        {
                                            // Game Over: Save high score
                                            if (score > highScore)  // New high score
                                            {
                                                highScore = score;
                                                ofstream outputFile(saveFile);
                                                if (outputFile.is_open())
                                                {
                                                    // Save new high score, clear saved game
                                                    outputFile << highScore << " 0 0 0 0";
                                                    outputFile.close();
                                                    hasSavedGame = false;
                                                }
                                            }
                                            else  // Didn't beat high score
                                            {
                                                ofstream outputFile(saveFile);
                                                if (outputFile.is_open())
                                                {
                                                    // Keep existing high score
                                                    outputFile << highScore << " 0 0 0 0";
                                                    outputFile.close();
                                                    hasSavedGame = false;
                                                }
                                            }
                                            loseSound.play();  // Game over sound
                                            currentState = STATE_GAME_OVER;  // Switch to game over screen
                                            selectedMenuItem = 0;
                                        }
                                    }
                                    grid[r + 1][c] = 0; // Remove enemy from grid
                                }
                                // Collision with Player Bullet: Enemy destroyed
                                else if (grid[r + 1][c] == 3)
                                {
                                    score += 3;  // Award 3 points for destroying enemy
                                    killCount++; // Increment kill counter (used for level progression)
                                    explosionSound.play();  // Play explosion sound
                                    grid[r + 1][c] = 0; // Destroy both enemy and bullet
                                    // Create visual explosion effect
                                    for (int i = 0; i < MAX_HIT_EFFECTS; i++)
                                    {
                                        if (!hitEffectActive[i])  // Find available effect slot
                                        {
                                            hitEffectRow[i] = r + 1;     // Explosion position
                                            hitEffectCol[i] = c;
                                            hitEffectTimer[i] = 0.0f;    // Start effect timer
                                            hitEffectActive[i] = true;   // Activate effect
                                            break;
                                        }
                                    }
                                    // Check for Level Up: Need 10 kills per level (10, 20, 30, 40, 50)
                                    int killsNeeded = level * 10;  // Calculate kills required for next level
                                    if (level < MAX_LEVEL && killCount >= killsNeeded)  // Ready to level up  // Ready to level up
                                    {
                                        level++;  // Advance to next level
                                        levelUpSound.play();  // Play level up sound
                                        killCount = 0;       // Reset kill counter for new level
                                        bossMoveCounter = 0; // Reset boss firing mechanics
                                        // Clear all entities from grid (except spaceship)
                                        // This gives player a brief respite before next level
                                        for (int r = 0; r < ROWS; r++)
                                        {
                                            for (int c = 0; c < COLS; c++)
                                            {
                                                // Remove meteors(2), bullets(3), enemies(4), bosses(5), boss bullets(6)
                                                if (grid[r][c] >= 2 && grid[r][c] <= 6)
                                                {
                                                    grid[r][c] = 0;  // Clear entity
                                                }
                                            }
                                        }
                                        // Reset spaceship to center starting position
                                        grid[ROWS - 1][spaceshipCol] = 0;  // Clear old position
                                        spaceshipCol = COLS / 2;            // Move to center
                                        grid[ROWS - 1][spaceshipCol] = 1;  // Mark new position
                                        // Transition to Level Up State (brief 2-second display)
                                        currentState = STATE_LEVEL_UP;
                                        levelUpTimer.restart();       // Start level up display timer
                                        levelUpBlinkClock.restart();  // Start blink animation
                                    }
                                    else if (level >= MAX_LEVEL && killCount >= killsNeeded)  // Victory!
                                    {
                                        // Victory Condition: Player completed level 5 with required kills
                                        // Save high score and clear saved game
                                        if (score > highScore)  // New high score achieved
                                        {
                                            highScore = score;
                                            ofstream outputFile(saveFile);
                                            if (outputFile.is_open())
                                            {
                                                // Save new high score, no saved game
                                                outputFile << highScore << " 0 0 0";
                                                outputFile.close();
                                                hasSavedGame = false;
                                            }
                                        }
                                        else  // Didn't beat high score
                                        {
                                            ofstream outputFile(saveFile);
                                            if (outputFile.is_open())
                                            {
                                                // Keep existing high score
                                                outputFile << highScore << " 0 0 0";
                                                outputFile.close();
                                                hasSavedGame = false;
                                            }
                                        }
                                        winSound.play();  // Play victory sound
                                        currentState = STATE_VICTORY;  // Switch to victory screen
                                        selectedMenuItem = 0;  // Default menu selection
                                    }
                                }
                            }
                        }
                    }
                }
                enemyMoveClock.restart();  // Reset enemy movement timer
            }

            // Boss Movement Logic: Move all bosses downward with level-based speed
            // Bosses move slightly slower than regular enemies but can shoot
            float bossMoveSpeed = 0.8f - ((level - 3) * 0.1f);  // Speed increases from level 3 onward
            if (bossMoveSpeed < 0.5f)  // Cap minimum speed (maximum difficulty)
                bossMoveSpeed = 0.5f;
            // Only move bosses if enough time has passed
            if (bossMoveClock.getElapsedTime().asSeconds() >= bossMoveSpeed)
            {
                // Iterate from bottom to top (prevents double-processing)
                for (int r = ROWS - 1; r >= 0; r--)
                {
                    for (int c = 0; c < COLS; c++)
                    {
                        if (grid[r][c] == 5)
                        { // Found a boss enemy
                            // Check if boss reached bottom edge (escaped)
                            if (r == ROWS - 1)
                            {
                                grid[r][c] = 0; // Remove boss from grid
                                lives--;        // Player loses life as penalty
                                damageSound.play();  // Play damage sound
                                isInvincible = true;  // Grant invincibility
                                invincibilityTimer.restart();  // Start invincibility timer
                                // Check if player has lost all lives
                                if (lives <= 0)
                                {
                                    // Update high score if current score is higher
                                    if (score > highScore)
                                    {
                                        highScore = score;
                                        ofstream outputFile(saveFile);
                                        if (outputFile.is_open())
                                        {
                                            outputFile << highScore << " 0 0 0";
                                            outputFile.close();
                                            hasSavedGame = false;
                                        }
                                    }
                                    else
                                    {
                                        ofstream outputFile(saveFile);
                                        if (outputFile.is_open())
                                        {
                                            outputFile << highScore << " 0 0 0";
                                            outputFile.close();
                                            hasSavedGame = false;
                                        }
                                    }
                                    loseSound.play();
                                    currentState = STATE_GAME_OVER;
                                    selectedMenuItem = 0;
                                }
                            }
                            else  // Boss not at bottom - continue moving
                            {
                                int nextRow = r + 1;  // Calculate next row
                                int nextCell = grid[nextRow][c];  // Check what's in next cell

                                grid[r][c] = 0; // Clear boss from current position

                                // Move down if next cell allows it
                                // Boss can move through empty(0), other bosses(5), boss bullets(6), meteors(2), enemies(4)
                                if (nextCell == 0 || nextCell == 5 || nextCell == 6 || nextCell == 2 || nextCell == 4)
                                {
                                    grid[nextRow][c] = 5;  // Place boss in new position
                                }
                                // Collision with Spaceship: Boss crashes into player
                                else if (grid[r + 1][c] == 1)
                                {
                                    // Check if player has active shield
                                    if (hasShield)
                                    {
                                        hasShield = false; // Shield absorbs hit and deactivates
                                        isInvincible = true;  // Activate invincibility after shield breaks
                                        invincibilityTimer.restart();  // Start invincibility timer
                                        explosionSound.play();  // Play explosion sound
                                    }
                                    else if (!isInvincible)  // Only damage if not invincible
                                    {
                                        lives--;  // Lose one life
                                        damageSound.play();  // Play damage sound
                                        isInvincible = true;  // Activate invincibility
                                        invincibilityTimer.restart();  // Start timer
                                        // Check if player has lost all lives
                                        if (lives <= 0)
                                        {
                                            // Update high score if current score is higher
                                            if (score > highScore)
                                            {
                                                highScore = score;
                                                ofstream outputFile(saveFile);
                                                if (outputFile.is_open())
                                                {
                                                    outputFile << highScore << " 0 0 0 0";
                                                    outputFile.close();
                                                    hasSavedGame = false;
                                                }
                                            }
                                            else
                                            {
                                                ofstream outputFile(saveFile);
                                                if (outputFile.is_open())
                                                {
                                                    outputFile << highScore << " 0 0 0 0";
                                                    outputFile.close();
                                                    hasSavedGame = false;
                                                }
                                            }
                                            loseSound.play();
                                            currentState = STATE_GAME_OVER;
                                            selectedMenuItem = 0;
                                        }
                                    }
                                    grid[r + 1][c] = 0; // Remove boss
                                }
                                // Collision with Player Bullet: Boss destroyed (worth more points)
                                else if (grid[r + 1][c] == 3)
                                {
                                    score += 5;  // Award 5 points for destroying boss (more than enemy)
                                    killCount++; // Increment kill counter
                                    explosionSound.play();  // Play explosion sound
                                    grid[r + 1][c] = 0; // Destroy both boss and bullet
                                    // Create visual explosion effect
                                    for (int i = 0; i < MAX_HIT_EFFECTS; i++)
                                    {
                                        if (!hitEffectActive[i])  // Find available effect slot
                                        {
                                            hitEffectRow[i] = r + 1;     // Explosion position
                                            hitEffectCol[i] = c;
                                            hitEffectTimer[i] = 0.0f;    // Start timer
                                            hitEffectActive[i] = true;   // Activate effect
                                            break;
                                        }
                                    }
                                    // Check for Level Up (need 10 kills per level)
                                    int killsNeeded = level * 10;
                                    if (level < MAX_LEVEL && killCount >= killsNeeded)
                                    {
                                        level++;  // Advance to next level
                                        levelUpSound.play();  // Play level up sound
                                        killCount = 0;         // Reset kill counter
                                        bossMoveCounter = 0;   // Reset boss mechanics
                                        // Clear all entities from grid
                                        for (int r = 0; r < ROWS; r++)
                                        {
                                            for (int c = 0; c < COLS; c++)
                                            {
                                                // Remove all entities except spaceship
                                                if (grid[r][c] >= 2 && grid[r][c] <= 6)
                                                {
                                                    grid[r][c] = 0;
                                                }
                                            }
                                        }
                                        // Reset spaceship to center
                                        grid[ROWS - 1][spaceshipCol] = 0;
                                        spaceshipCol = COLS / 2;
                                        grid[ROWS - 1][spaceshipCol] = 1;
                                        // Transition to Level Up screen
                                        currentState = STATE_LEVEL_UP;
                                        levelUpTimer.restart();
                                        levelUpBlinkClock.restart();
                                    }
                                    else if (level >= MAX_LEVEL && killCount >= killsNeeded)  // Victory!
                                    {
                                        // Victory Condition: Completed level 5
                                        winSound.play();  // Play victory sound
                                        currentState = STATE_VICTORY;  // Switch to victory screen
                                        selectedMenuItem = 0;
                                    }
                                }
                            }
                        }
                    }
                }

                // Boss Firing Logic: Bosses periodically shoot bullets downward
                // Firing frequency increases with level (more dangerous at higher levels)
                bossMoveCounter++;  // Increment movement counter

                // Determine how often bosses fire based on current level
                float firingInterval;
                if (level == 3)
                {
                    firingInterval = 5; // Level 3: Fire every 5 movements (slowest)
                }
                else if (level == 4)
                {
                    firingInterval = 4; // Level 4: Fire every 4 movements (moderate)
                }
                else
                {                       // Level 5
                    firingInterval = 3; // Level 5: Fire every 3 movements (fastest)
                }

                // Execute Boss Firing: If counter reaches interval, all bosses fire
                if (bossMoveCounter >= firingInterval)
                {
                    // Scan grid for all bosses and make them fire
                    for (int r = 0; r < ROWS; r++)
                    {
                        for (int c = 0; c < COLS; c++)
                        {
                            if (grid[r][c] == 5)
                            { // Found a boss
                                // Fire bullet downward if possible
                                if (r < ROWS - 1)  // Not at bottom row
                                {
                                    int bulletRow = r + 1;  // Row below boss
                                    // Only fire if cell below is empty
                                    if (bulletRow < ROWS && grid[bulletRow][c] == 0)
                                    {
                                        grid[bulletRow][c] = 6; // Grid value 6 = boss bullet
                                    }
                                }
                            }
                        }
                    }
                    bossMoveCounter = 0; // Reset counter for next firing cycle
                }

                bossMoveClock.restart();  // Reset boss movement timer
            }

            // Boss Bullet Movement Logic: Move boss bullets downward very fast
            // Boss bullets move much faster than other entities (constant fast speed)
            float bossBulletSpeed = 0.15f; // Move every 0.15 seconds (very fast, regardless of level)
            if (bossBulletMoveClock.getElapsedTime().asSeconds() >= bossBulletSpeed)
            {
                // Iterate from bottom to top (prevents double-processing)
                for (int r = ROWS - 1; r >= 0; r--)
                {
                    for (int c = 0; c < COLS; c++)
                    {
                        if (grid[r][c] == 6)
                        { // Found a boss bullet (enemy projectile)
                            // Check if bullet reached bottom edge
                            if (r == ROWS - 1)
                            {
                                grid[r][c] = 0; // Remove bullet (goes off screen)
                            }
                            else  // Bullet not at bottom - continue moving
                            {
                                grid[r][c] = 0; // Clear current position
                                // Collision with Spaceship: Boss bullet hits player
                                if (grid[r + 1][c] == 1)
                                {
                                    // Check if player has active shield
                                    if (hasShield)
                                    {
                                        hasShield = false; // Shield absorbs hit and deactivates
                                        isInvincible = true;  // Activate invincibility after shield breaks
                                        invincibilityTimer.restart();  // Start invincibility timer
                                        explosionSound.play();  // Play explosion sound
                                    }
                                    else if (!isInvincible)  // Only damage if not invincible
                                    {
                                        lives--;  // Lose one life
                                        damageSound.play();  // Play damage sound
                                        isInvincible = true;  // Activate invincibility
                                        invincibilityTimer.restart();  // Start timer
                                        // Check if player has lost all lives
                                        if (lives <= 0)
                                        {
                                            // Game Over: Save high score
                                            if (score > highScore)  // New high score
                                            {
                                                highScore = score;
                                                ofstream outputFile(saveFile);
                                                if (outputFile.is_open())
                                                {
                                                    // Save new high score, clear saved game
                                                    outputFile << highScore << " 0 0 0 0";
                                                    outputFile.close();
                                                    hasSavedGame = false;
                                                }
                                            }
                                            else  // Didn't beat high score
                                            {
                                                ofstream outputFile(saveFile);
                                                if (outputFile.is_open())
                                                {
                                                    // Keep existing high score
                                                    outputFile << highScore << " 0 0 0 0";
                                                    outputFile.close();
                                                    hasSavedGame = false;
                                                }
                                            }
                                            loseSound.play();  // Game over sound
                                            currentState = STATE_GAME_OVER;  // Switch to game over screen
                                            selectedMenuItem = 0;
                                        }
                                    }
                                    // Create visual hit effect at collision location
                                    for (int i = 0; i < MAX_HIT_EFFECTS; i++)
                                    {
                                        if (!hitEffectActive[i])  // Find available effect slot
                                        {
                                            hitEffectRow[i] = r + 1;     // Effect position
                                            hitEffectCol[i] = c;
                                            hitEffectTimer[i] = 0.0f;    // Start timer
                                            hitEffectActive[i] = true;   // Activate effect
                                            break;
                                        }
                                    }
                                }
                                // Boss bullet passes through meteors and enemies (doesn't destroy them)
                                // This makes boss bullets more dangerous (harder to block)
                                else if (grid[r + 1][c] == 2 || grid[r + 1][c] == 4)
                                {
                                    grid[r + 1][c] = 6; // Bullet continues through, entity remains
                                }
                                // Move to empty space or stack with another boss bullet
                                else if (grid[r + 1][c] == 0 || grid[r + 1][c] == 6)
                                {
                                    grid[r + 1][c] = 6;  // Move bullet down
                                }
                            }
                        }
                    }
                }
                bossBulletMoveClock.restart();  // Reset bullet movement timer
            }

            // Player Bullet Movement Logic: Move player bullets upward very fast
            // Bullets move much faster than enemies (0.05 seconds per cell)
            if (bulletMoveClock.getElapsedTime().asSeconds() >= 0.05f)
            {
                // Iterate from top to bottom (prevents processing same bullet twice)
                // Since bullets move UP, we process top rows first
                for (int r = 0; r < ROWS; r++)
                {
                    for (int c = 0; c < COLS; c++)
                    {
                        if (grid[r][c] == 3)
                        { // Found a player bullet
                            // Check if bullet reached top edge
                            if (r == 0)
                            {
                                grid[r][c] = 0; // Remove bullet (goes off screen)
                            }
                            else  // Bullet not at top - continue moving upward
                            {
                                grid[r][c] = 0; // Clear current position
                                // Move up if next cell allows it
                                // Can move through empty(0), other bullets(3), or shield powerup(7)
                                if (grid[r - 1][c] == 0 || grid[r - 1][c] == 3 || grid[r - 1][c] == 7)
                                {
                                    // Don't overwrite shield powerup (let bullet pass through)
                                    if (grid[r - 1][c] != 7)
                                        grid[r - 1][c] = 3;  // Move bullet up
                                }
                                // Collision with Boss Bullet: Bullets destroy each other
                                else if (grid[r - 1][c] == 6)
                                {
                                    explosionSound.play();  // Play explosion sound
                                    grid[r - 1][c] = 0; // Destroy both bullets
                                    // Create visual explosion effect
                                    for (int i = 0; i < MAX_HIT_EFFECTS; i++)
                                    {
                                        if (!hitEffectActive[i])
                                        {
                                            hitEffectRow[i] = r - 1;
                                            hitEffectCol[i] = c;
                                            hitEffectTimer[i] = 0.0f;
                                            hitEffectActive[i] = true;
                                            break;
                                        }
                                    }
                                }
                                // Collision with Meteor: Player bullet destroys meteor for points
                                else if (grid[r - 1][c] == 2)
                                {
                                    int meteorPoints = 1 + (rand() % 2); // Random 1-2 points per meteor
                                    score += meteorPoints;  // Add points to score
                                    explosionSound.play();  // Play explosion sound
                                    grid[r - 1][c] = 0; // Destroy both meteor and bullet
                                    // Create visual explosion effect
                                    for (int i = 0; i < MAX_HIT_EFFECTS; i++)
                                    {
                                        if (!hitEffectActive[i])  // Find available effect slot
                                        {
                                            hitEffectRow[i] = r - 1;     // Explosion position
                                            hitEffectCol[i] = c;
                                            hitEffectTimer[i] = 0.0f;    // Start timer
                                            hitEffectActive[i] = true;   // Activate effect
                                            break;
                                        }
                                    }
                                }
                                // Collision with Enemy: Player bullet destroys enemy
                                else if (grid[r - 1][c] == 4)
                                {
                                    score += 3;  // Award 3 points for destroying enemy
                                    killCount++; // Increment kill counter (progress toward level up)
                                    explosionSound.play();  // Play explosion sound
                                    grid[r - 1][c] = 0; // Destroy both enemy and bullet
                                    // Create visual explosion effect
                                    for (int i = 0; i < MAX_HIT_EFFECTS; i++)
                                    {
                                        if (!hitEffectActive[i])  // Find available effect slot
                                        {
                                            hitEffectRow[i] = r - 1;     // Explosion position
                                            hitEffectCol[i] = c;
                                            hitEffectTimer[i] = 0.0f;    // Start timer
                                            hitEffectActive[i] = true;   // Activate effect
                                            break;
                                        }
                                    }
                                    // Check for Level Up: Need 10 kills per level
                                    int killsNeeded = level * 10;  // Calculate required kills
                                    if (level < MAX_LEVEL && killCount >= killsNeeded)  // Ready to level up  // Ready to level up
                                    {
                                        level++;  // Advance to next level
                                        levelUpSound.play();  // Play level up sound
                                        killCount = 0;         // Reset kill counter for new level
                                        bossMoveCounter = 0;   // Reset boss mechanics
                                        // Clear all entities from grid (give player a break)
                                        for (int r = 0; r < ROWS; r++)
                                        {
                                            for (int c = 0; c < COLS; c++)
                                            {
                                                // Remove all entities except spaceship
                                                if (grid[r][c] >= 2 && grid[r][c] <= 6)
                                                {
                                                    grid[r][c] = 0;  // Clear entity
                                                }
                                            }
                                        }
                                        // Reset spaceship to center position
                                        grid[ROWS - 1][spaceshipCol] = 0;  // Clear old position
                                        spaceshipCol = COLS / 2;            // Move to center
                                        grid[ROWS - 1][spaceshipCol] = 1;  // Mark new position
                                        // Transition to Level Up screen (brief 2-second display)
                                        currentState = STATE_LEVEL_UP;
                                        levelUpTimer.restart();       // Start display timer
                                        levelUpBlinkClock.restart();  // Start blink animation
                                    }
                                    else if (level >= MAX_LEVEL && killCount >= killsNeeded)  // Victory!
                                    {
                                        // Victory Condition: Completed level 5 with required kills
                                        // Save high score and clear saved game
                                        if (score > highScore)  // New high score
                                        {
                                            highScore = score;
                                            ofstream outputFile(saveFile);
                                            if (outputFile.is_open())
                                            {
                                                // Save new high score, clear saved game
                                                outputFile << highScore << " 0 0 0 0";
                                                outputFile.close();
                                                hasSavedGame = false;
                                            }
                                        }
                                        else  // Didn't beat high score
                                        {
                                            ofstream outputFile(saveFile);
                                            if (outputFile.is_open())
                                            {
                                                // Keep existing high score, clear saved game
                                                outputFile << highScore << " 0 0 0 0";
                                                outputFile.close();
                                                hasSavedGame = false;
                                            }
                                        }
                                        winSound.play();  // Play victory sound
                                        currentState = STATE_VICTORY;  // Switch to victory screen
                                        selectedMenuItem = 0;  // Default menu selection
                                    }
                                }
                                // Collision with Boss: Player bullet destroys boss (worth more points)
                                else if (grid[r - 1][c] == 5)
                                {
                                    score += 5;  // Award 5 points for destroying boss (more valuable)
                                    killCount++; // Increment kill counter
                                    explosionSound.play();  // Play explosion sound
                                    grid[r - 1][c] = 0; // Destroy both boss and bullet
                                    // Create visual explosion effect
                                    for (int i = 0; i < MAX_HIT_EFFECTS; i++)
                                    {
                                        if (!hitEffectActive[i])  // Find available effect slot
                                        {
                                            hitEffectRow[i] = r - 1;     // Explosion position
                                            hitEffectCol[i] = c;
                                            hitEffectTimer[i] = 0.0f;    // Start timer
                                            hitEffectActive[i] = true;   // Activate effect
                                            break;
                                        }
                                    }
                                    // Check for Level Up: Need 10 kills per level
                                    int killsNeeded = level * 10;  // Calculate required kills
                                    if (level < MAX_LEVEL && killCount >= killsNeeded)  // Ready to level up  // Ready to level up
                                    {
                                        level++;  // Advance to next level
                                        levelUpSound.play();  // Play level up sound
                                        killCount = 0;         // Reset kill counter
                                        bossMoveCounter = 0;   // Reset boss mechanics
                                        // Clear all entities from grid
                                        for (int r = 0; r < ROWS; r++)
                                        {
                                            for (int c = 0; c < COLS; c++)
                                            {
                                                // Remove all entities except spaceship
                                                if (grid[r][c] >= 2 && grid[r][c] <= 6)
                                                {
                                                    grid[r][c] = 0;  // Clear entity
                                                }
                                            }
                                        }
                                        // Reset spaceship to center
                                        grid[ROWS - 1][spaceshipCol] = 0;
                                        spaceshipCol = COLS / 2;
                                        grid[ROWS - 1][spaceshipCol] = 1;
                                        // Transition to Level Up screen
                                        currentState = STATE_LEVEL_UP;
                                        levelUpTimer.restart();
                                        levelUpBlinkClock.restart();
                                    }
                                    else if (level >= MAX_LEVEL && killCount >= killsNeeded)  // Victory!
                                    {
                                        // Victory Condition: Completed level 5
                                        // Save high score and clear saved game
                                        if (score > highScore)  // New high score
                                        {
                                            highScore = score;
                                            ofstream outputFile(saveFile);
                                            if (outputFile.is_open())
                                            {
                                                // Save new high score, no saved game
                                                outputFile << highScore << " 0 0 0";
                                                outputFile.close();
                                                hasSavedGame = false;
                                            }
                                        }
                                        else  // Didn't beat high score
                                        {
                                            ofstream outputFile(saveFile);
                                            if (outputFile.is_open())
                                            {
                                                // Keep existing high score
                                                outputFile << highScore << " 0 0 0";
                                                outputFile.close();
                                                hasSavedGame = false;
                                            }
                                        }
                                        winSound.play();  // Play victory sound
                                        currentState = STATE_VICTORY;  // Switch to victory screen
                                        selectedMenuItem = 0;  // Default selection
                                    }
                                }
                            }
                        }
                    }
                }
                bulletMoveClock.restart();  // Reset bullet movement timer
            }

            // Update Hit Effects: Manage explosion animation timers
            float deltaTime = hitEffectClock.getElapsedTime().asSeconds();  // Time since last update
            for (int i = 0; i < MAX_HIT_EFFECTS; i++)  // Check all 50 possible effects
            {
                if (hitEffectActive[i])  // If this explosion is currently visible
                {
                    hitEffectTimer[i] += deltaTime;  // Increment timer
                    if (hitEffectTimer[i] >= HIT_EFFECT_DURATION)  // Lasted 0.3 seconds
                    {
                        hitEffectActive[i] = false; // Remove explosion from display
                    }
                }
            }
            hitEffectClock.restart();  // Reset clock for next frame

            // Update Invincibility Status: Player is invincible for 1 second after taking damage
            if (isInvincible && invincibilityTimer.getElapsedTime().asSeconds() >= INVINCIBILITY_DURATION)  // 1 second passed
            {
                isInvincible = false;  // Remove invincibility protection
            }

        } // End of PLAYING state

        // Level Up State Logic: Brief 2-second display before returning to gameplay
        else if (currentState == STATE_LEVEL_UP)
        {
            // Create blinking animation for "LEVEL UP" text (toggles every 0.3 seconds)
            if (levelUpBlinkClock.getElapsedTime().asSeconds() >= 0.3f)
            {
                levelUpBlinkState = !levelUpBlinkState;  // Toggle visibility
                levelUpBlinkClock.restart();  // Reset blink timer
            }

            // Automatically return to gameplay after 2 seconds
            if (levelUpTimer.getElapsedTime().asSeconds() >= 2.0f)
            {
                currentState = STATE_PLAYING;  // Resume game
                // Restart all game clocks (fresh start for new level)
                meteorSpawnClock.restart();
                meteorMoveClock.restart();
                enemySpawnClock.restart();
                enemyMoveClock.restart();
                bossSpawnClock.restart();
                bossMoveClock.restart();
                bossBulletMoveClock.restart();
                shieldPowerupSpawnClock.restart();
                shieldPowerupMoveClock.restart();
            }
        }

        // Victory State Logic: Player completed level 5, show victory screen
        else if (currentState == STATE_VICTORY)
        {
            // Victory menu navigation: Two options (Restart Game, Main Menu)
            if (menuClock.getElapsedTime() >= menuCooldown)  // Cooldown prevents accidental inputs
            {
                bool menuAction = false;  // Track if user pressed a key

                // Move selection up (wrap around from top to bottom)
                if (Keyboard::isKeyPressed(Keyboard::Up) || Keyboard::isKeyPressed(Keyboard::W))
                {
                    selectedMenuItem = (selectedMenuItem - 1 + 2) % 2;  // Cycle through 2 options
                    menuNavSound.play();  // Play navigation sound
                    menuAction = true;
                }
                // Move selection down (wrap around from bottom to top)
                else if (Keyboard::isKeyPressed(Keyboard::Down) || Keyboard::isKeyPressed(Keyboard::S))
                {
                    selectedMenuItem = (selectedMenuItem + 1) % 2;  // Cycle through 2 options
                    menuNavSound.play();  // Play navigation sound
                    menuAction = true;
                }
                // Confirm selection with Enter key
                else if (Keyboard::isKeyPressed(Keyboard::Enter))
                {
                    menuClickSound.play();  // Play selection sound
                    if (selectedMenuItem == 0)  // Option 1: Restart Game (play again from level 1)
                    {
                        currentState = STATE_PLAYING;  // Start playing
                        // Reset all game variables to initial state
                        lives = 3;
                        score = 0;
                        killCount = 0;
                        level = 1;
                        bossMoveCounter = 0;
                        isInvincible = false;
                        hasShield = false;
                        // Clear entire grid
                        for (int r = 0; r < ROWS; r++)
                        {
                            for (int c = 0; c < COLS; c++)
                            {
                                grid[r][c] = 0;  // Empty cell
                            }
                        }
                        // Place spaceship in center of bottom row
                        spaceshipCol = COLS / 2;
                        grid[ROWS - 1][spaceshipCol] = 1;
                        // Restart all game clocks
                        meteorSpawnClock.restart();
                        meteorMoveClock.restart();
                        enemySpawnClock.restart();
                        enemyMoveClock.restart();
                        bossSpawnClock.restart();
                        bossMoveClock.restart();
                        bossBulletMoveClock.restart();
                        bulletMoveClock.restart();
                        shieldPowerupSpawnClock.restart();
                        shieldPowerupMoveClock.restart();
                    }
                    else if (selectedMenuItem == 1)  // Option 2: Return to Main Menu
                    {
                        if (bgMusic.getStatus() != Music::Playing)  // Resume music if stopped
                        {
                            bgMusic.play();
                        }
                        currentState = STATE_MENU;  // Return to menu
                        selectedMenuItem = 0;  // Reset menu selection
                    }
                    menuAction = true;
                }

                if (menuAction)
                {
                    menuClock.restart();
                }
            }
        }

        // Pause State Logic: Game is paused, player can resume, restart, or save & quit
        else if (currentState == STATE_PAUSED)
        {
            // Pause menu navigation: Three options (Resume, Restart Level, Save & Quit)
            if (menuClock.getElapsedTime() >= menuCooldown)  // Prevent accidental inputs
            {
                bool menuAction = false;  // Track key presses

                // Move selection up (wrap around from top to bottom)
                if (Keyboard::isKeyPressed(Keyboard::Up) || Keyboard::isKeyPressed(Keyboard::W))
                {
                    selectedMenuItem = (selectedMenuItem - 1 + 3) % 3;  // Cycle through 3 options
                    menuNavSound.play();  // Play navigation sound
                    menuAction = true;
                }
                // Move selection down (wrap around from bottom to top)
                else if (Keyboard::isKeyPressed(Keyboard::Down) || Keyboard::isKeyPressed(Keyboard::S))
                {
                    selectedMenuItem = (selectedMenuItem + 1) % 3;  // Cycle through 3 options
                    menuNavSound.play();  // Play navigation sound
                    menuAction = true;
                }
                // Confirm selection with Enter key
                else if (Keyboard::isKeyPressed(Keyboard::Enter))
                {
                    menuClickSound.play();  // Play selection sound
                    if (selectedMenuItem == 0)  // Option 1: Resume Game (continue where left off)
                    {
                        currentState = STATE_PLAYING;  // Return to gameplay immediately
                    }
                    else if (selectedMenuItem == 1)  // Option 2: Restart Level (keep level/lives, reset score)
                    {
                        currentState = STATE_PLAYING;  // Start playing
                        // Reset score and kill counter (keep current level and lives)
                        score = 0;
                        killCount = 0;
                        bossMoveCounter = 0;
                        isInvincible = false;
                        // Clear all entities from grid
                        for (int r = 0; r < ROWS; r++)
                        {
                            for (int c = 0; c < COLS; c++)
                            {
                                grid[r][c] = 0;  // Empty cell
                            }
                        }
                        // Place spaceship in center
                        spaceshipCol = COLS / 2;
                        grid[ROWS - 1][spaceshipCol] = 1;
                        // Restart all game clocks
                        meteorSpawnClock.restart();
                        meteorMoveClock.restart();
                        enemySpawnClock.restart();
                        enemyMoveClock.restart();
                        bossSpawnClock.restart();
                        bossMoveClock.restart();
                        bossBulletMoveClock.restart();
                        bulletMoveClock.restart();
                        shieldPowerupSpawnClock.restart();
                        shieldPowerupMoveClock.restart();
                    }
                    else if (selectedMenuItem == 2)  // Option 3: Save & Quit to Main Menu
                    {
                        // Save current game state to file (format: "highScore lives score level")
                        ofstream outputFile(saveFile);
                        if (outputFile.is_open())
                        {
                            outputFile << highScore << " " << lives << " " << score << " " << level;
                            outputFile.close();
                            // Update saved game tracking variables
                            hasSavedGame = true;
                            savedLives = lives;
                            savedScore = score;
                            savedLevel = level;
                        }
                        // Resume background music if stopped
                        if (bgMusic.getStatus() != Music::Playing)
                        {
                            bgMusic.play();
                        }
                        currentState = STATE_MENU;  // Return to main menu
                        selectedMenuItem = 0;  // Reset selection
                    }
                    menuAction = true;
                }
                // Quick resume with P key (same key used to pause)
                else if (Keyboard::isKeyPressed(Keyboard::P))
                {
                    currentState = STATE_PLAYING;  // Resume gameplay
                    menuAction = true;
                }

                if (menuAction)
                {
                    menuClock.restart();
                }
            }
        }

        // ========== RENDERING SECTION: Draw all visuals to the window ==========
        window.clear(Color(40, 40, 40)); // Clear screen with dark gray background

        // Main Menu Screen: Show title, options, and high score
        if (currentState == STATE_MENU)
        {
            // Draw background image and title
            window.draw(menuBackground);  // Background image
            window.draw(menuTitle);       // "SPACE SHOOTER" title

            // Update and display high score (centered on screen)
            char menuHighScoreBuffer[50];
            sprintf(menuHighScoreBuffer, "High Score: %d", highScore);  // Format score
            menuHighScoreText.setString(menuHighScoreBuffer);
            // Center horizontally at y=180
            menuHighScoreText.setPosition(windowWidth / 2 - menuHighScoreText.getLocalBounds().width / 2.0f, 180);
            window.draw(menuHighScoreText);

            // Draw all menu options (4 options: New Game, Continue, Instructions, Exit)
            for (int i = 0; i < 4; i++)
            {
                // Highlight selected option in yellow, others in white
                if (i == selectedMenuItem)
                {
                    menuItems[i].setFillColor(Color::Yellow);  // Selected
                }
                else
                {
                    menuItems[i].setFillColor(Color::White);   // Not selected
                }
                window.draw(menuItems[i]);  // Draw option text
            }
            window.draw(menuInstructions);  // Draw "Press Enter to select" text
        }
        // Instructions Screen: Show controls, entity descriptions, and game objectives
        else if (currentState == STATE_INSTRUCTIONS)
        {
            // Draw background and title
            window.draw(menuBackground);      // Background image
            window.draw(instructionsTitle);   // "HOW TO PLAY" title

            // Section 1: Controls (keyboard inputs)
            window.draw(controlsTitle);  // "CONTROLS:" header
            window.draw(moveText);       // Movement keys (Left/Right/A/D)
            window.draw(shootText);      // Shoot key (Space)
            window.draw(pauseText);      // Pause key (P)

            // Section 2: Entity Descriptions (show each sprite with explanation)
            window.draw(entitiesTitle);  // "ENTITIES:" header

            // Player spaceship sprite and description
            spaceship.setPosition(60, 285);
            window.draw(spaceship);
            window.draw(playerDesc);  // "Player - Your spaceship"

            // Meteor sprite and description
            meteor.setPosition(60, 325);
            window.draw(meteor);
            window.draw(meteorDesc);  // "Meteor - Worth 1-2 points"

            // Enemy sprite and description
            enemy.setPosition(60, 365);
            window.draw(enemy);
            window.draw(enemyDesc);  // "Enemy - Worth 3 points"

            // Boss sprite and description
            bossEnemy.setPosition(60, 405);
            window.draw(bossEnemy);
            window.draw(bossDesc);  // "Boss - Worth 5 points, shoots bullets"

            // Player bullet sprite and description
            bullet.setPosition(60 + BULLET_OFFSET_X, 445);
            window.draw(bullet);
            window.draw(bulletDesc);  // "Player Bullet - Your projectile"

            // Boss bullet sprite and description
            bossBullet.setPosition(60 + BULLET_OFFSET_X, 485);
            window.draw(bossBullet);
            window.draw(bossBulletDesc);  // "Boss Bullet - Avoid or shoot"

            // Life icon and description
            lifeIcon.setPosition(60 + 8, 525);
            window.draw(lifeIcon);
            window.draw(lifeDesc);  // "Life - You have 3 lives"

            // Shield powerup sprite and description
            shieldPowerUp.setPosition(60, 565);
            window.draw(shieldPowerUp);
            window.draw(shieldPowerupDesc);  // "Shield - Protects from 1 hit"

            // Section 3: Game Systems (lives, levels, high score)
            window.draw(systemsTitle);   // "GAME SYSTEMS:" header
            window.draw(livesDesc);      // Lives explanation
            window.draw(levelsDesc);     // Level progression explanation
            window.draw(highScoreDesc);  // High score saving explanation

            // Section 4: Objectives (win conditions)
            window.draw(objectiveTitle);  // "OBJECTIVES:" header
            window.draw(objective1);      // Destroy enemies
            window.draw(objective2);      // Survive
            window.draw(objective3);      // Level up every 10 kills
            window.draw(objective4);      // Beat level 5 to win

            // Navigation hint
            window.draw(instructionsBack);  // "Press ESC to return"
        }
        // Gameplay Screen: Draw all game entities, UI, and effects
        else if (currentState == STATE_PLAYING)
        {
            // Draw background and game border
            window.draw(background);  // Space background image
            window.draw(gameBox);     // White border around play area

            // Draw all entities from the grid (23 rows x 15 columns)
            for (int r = 0; r < ROWS; r++)
            {
                for (int c = 0; c < COLS; c++)
                {
                    // Value 1 = Player Spaceship (bottom row, controlled by player)
                    if (grid[r][c] == 1)
                    {
                        spaceship.setPosition(MARGIN + c * CELL_SIZE, MARGIN + r * CELL_SIZE);
                        // Blink effect during invincibility (flash every 100ms)
                        if (!isInvincible || ((int)(invincibilityTimer.getElapsedTime().asMilliseconds() / 100) % 2 == 0))
                        {
                            window.draw(spaceship);  // Draw if visible this frame
                        }
                    }
                    // Value 2 = Meteor (falls down, destroys on contact)
                    else if (grid[r][c] == 2)
                    {
                        meteor.setPosition(MARGIN + c * CELL_SIZE, MARGIN + r * CELL_SIZE);
                        window.draw(meteor);
                    }
                    // Value 3 = Player Bullet (shoots upward)
                    else if (grid[r][c] == 3)
                    {
                        bullet.setPosition(MARGIN + c * CELL_SIZE + BULLET_OFFSET_X, MARGIN + r * CELL_SIZE);
                        window.draw(bullet);  // Offset for visual centering
                    }
                    // Value 4 = Enemy (moves down, worth 3 points)
                    else if (grid[r][c] == 4)
                    {
                        enemy.setPosition(MARGIN + c * CELL_SIZE, MARGIN + r * CELL_SIZE);
                        window.draw(enemy);
                    }
                    // Value 5 = Boss (moves left/right, shoots bullets, worth 5 points)
                    else if (grid[r][c] == 5)
                    {
                        bossEnemy.setPosition(MARGIN + c * CELL_SIZE, MARGIN + r * CELL_SIZE);
                        window.draw(bossEnemy);
                    }
                    // Value 6 = Boss Bullet (shoots downward from boss)
                    else if (grid[r][c] == 6)
                    {
                        bossBullet.setPosition(MARGIN + c * CELL_SIZE + BULLET_OFFSET_X, MARGIN + r * CELL_SIZE);
                        window.draw(bossBullet);  // Offset for visual centering
                    }
                }
            }

            // Draw shield powerups from separate tracking array (not in grid)
            // Maximum 5 shield powerups can be active simultaneously
            for (int i = 0; i < MAX_SHIELD_POWERUPS; i++)
            {
                if (shieldPowerupActive[i])  // If this powerup exists
                {
                    // Position shield sprite at its grid location
                    shieldPowerUp.setPosition(MARGIN + shieldPowerupCol[i] * CELL_SIZE, MARGIN + shieldPowerupRow[i] * CELL_SIZE);
                    window.draw(shieldPowerUp);
                }
            }

            // Draw Shield icon over spaceship if player has shield protection
            if (hasShield)  // Player collected a shield powerup
            {
                // Center shield icon over spaceship position (bottom row)
                shieldIcon.setPosition(MARGIN + spaceshipCol * CELL_SIZE + SHIELD_OFFSET, MARGIN + (ROWS - 1) * CELL_SIZE + SHIELD_OFFSET);
                window.draw(shieldIcon);  // Visual indicator of protection
            }

            // Draw all active explosion effects (max 50 simultaneous)
            for (int i = 0; i < MAX_HIT_EFFECTS; i++)
            {
                if (hitEffectActive[i])  // If explosion is visible
                {
                    // Position explosion sprite at collision location
                    bulletHit.setPosition(MARGIN + hitEffectCol[i] * CELL_SIZE, MARGIN + hitEffectRow[i] * CELL_SIZE);
                    window.draw(bulletHit);  // Draw explosion (lasts 0.3 seconds)
                }
            }

            // UI Section: Lives display (text + icons)
            livesText.setString("Lives:");  // Label

            // Draw individual life icons (heart sprites) for remaining lives
            float lifeIconStartX = livesText.getPosition().x + livesText.getLocalBounds().width + 10;  // Start after "Lives:" text
            float lifeIconY = livesText.getPosition().y + (livesText.getLocalBounds().height / 2.0f) - 12;  // Vertically center

            // Draw one icon for each remaining life
            for (int i = 0; i < lives; i++)
            {
                lifeIcon.setPosition(lifeIconStartX + (i * 28), lifeIconY);  // Space icons 28px apart
                window.draw(lifeIcon);
            }

            // UI Section: Current score (updates constantly)
            char scoreBuffer[20];
            sprintf(scoreBuffer, "Score: %d", score);  // Format score
            scoreText.setString(scoreBuffer);

            // UI Section: Kill counter and progress (e.g., "Kills: 7/10")
            char killsBuffer[50];
            sprintf(killsBuffer, "Kills: %d/%d", killCount, level * 10);  // Show progress toward level up
            killsText.setString(killsBuffer);

            // UI Section: Current level (1-5)
            char levelBuffer[20];
            sprintf(levelBuffer, "Level: %d", level);
            levelText.setString(levelBuffer);

            // UI Section: High score (best score ever achieved)
            char highScoreBuffer[50];
            sprintf(highScoreBuffer, "High Score: %d", highScore);
            highScoreText.setString(highScoreBuffer);

            // Draw all UI text elements to screen
            window.draw(title);          // "SPACE SHOOTER" title
            window.draw(livesText);      // Lives label
            window.draw(scoreText);      // Current score
            window.draw(killsText);      // Kill progress
            window.draw(levelText);      // Current level
            window.draw(highScoreText);  // High score
        }
        // Level Up Screen: Brief 2-second display when advancing to next level
        else if (currentState == STATE_LEVEL_UP)
        {
            // Draw background and border
            window.draw(background);  // Space background
            window.draw(gameBox);     // Game border

            // Draw player's spaceship (centered, all other entities cleared)
            spaceship.setPosition(MARGIN + spaceshipCol * CELL_SIZE, MARGIN + (ROWS - 1) * CELL_SIZE);
            window.draw(spaceship);

            // Draw blinking "LEVEL UP!" text (toggles every 0.3 seconds)
            if (levelUpBlinkState)  // Only visible when blink state is true
            {
                window.draw(levelUpText);  // Large centered text
            }

            // Update and show new level number
            char levelBuffer[20];
            sprintf(levelBuffer, "Level: %d", level);  // Show advanced level
            levelText.setString(levelBuffer);

            // Update and show reset kill counter (starts at 0 for new level)
            char killsBuffer[50];
            sprintf(killsBuffer, "Kills: %d/%d", killCount, level * 10);  // "0/10", "0/20", etc.
            killsText.setString(killsBuffer);

            // Draw UI elements (same as gameplay screen)
            window.draw(title);
            window.draw(livesText);
            window.draw(scoreText);
            window.draw(killsText);
            window.draw(levelText);
        }
        // Pause Screen: Show frozen game state with darkened overlay and menu
        else if (currentState == STATE_PAUSED)
        {
            // Draw background and border (same as gameplay)
            window.draw(background);
            window.draw(gameBox);

            // Draw frozen game state (all entities remain visible but unmoving)
            for (int r = 0; r < ROWS; r++)
            {
                for (int c = 0; c < COLS; c++)
                {
                    // Draw each entity type at its current position
                    if (grid[r][c] == 1)  // Spaceship
                    {
                        spaceship.setPosition(MARGIN + c * CELL_SIZE, MARGIN + r * CELL_SIZE);
                        window.draw(spaceship);
                    }
                    else if (grid[r][c] == 2)  // Meteor
                    {
                        meteor.setPosition(MARGIN + c * CELL_SIZE, MARGIN + r * CELL_SIZE);
                        window.draw(meteor);
                    }
                    else if (grid[r][c] == 3)  // Player Bullet
                    {
                        bullet.setPosition(MARGIN + c * CELL_SIZE + BULLET_OFFSET_X, MARGIN + r * CELL_SIZE);
                        window.draw(bullet);
                    }
                    else if (grid[r][c] == 4)  // Enemy
                    {
                        enemy.setPosition(MARGIN + c * CELL_SIZE, MARGIN + r * CELL_SIZE);
                        window.draw(enemy);
                    }
                    else if (grid[r][c] == 5)  // Boss
                    {
                        bossEnemy.setPosition(MARGIN + c * CELL_SIZE, MARGIN + r * CELL_SIZE);
                        window.draw(bossEnemy);
                    }
                    else if (grid[r][c] == 6)  // Boss Bullet
                    {
                        bossBullet.setPosition(MARGIN + c * CELL_SIZE + BULLET_OFFSET_X, MARGIN + r * CELL_SIZE);
                        window.draw(bossBullet);
                    }
                }
            }

            // Create semi-transparent dark overlay to dim background game state
            RectangleShape overlay(Vector2f(COLS * CELL_SIZE, ROWS * CELL_SIZE));  // Cover play area
            overlay.setPosition(MARGIN, MARGIN);  // Position over game grid
            overlay.setFillColor(Color(0, 0, 0, 150)); // Semi-transparent black (alpha=150)
            window.draw(overlay);  // Darken background to highlight menu

            // Draw pause menu on top of overlay
            window.draw(pauseTitle);  // "PAUSED" title text
            // Draw 3 menu options (Resume, Restart Level, Save & Quit)
            for (int i = 0; i < 3; i++)
            {
                // Highlight selected option in yellow, others in white
                if (i == selectedMenuItem)
                {
                    pauseItems[i].setFillColor(Color::Yellow);  // Selected
                }
                else
                {
                    pauseItems[i].setFillColor(Color::White);   // Not selected
                }
                window.draw(pauseItems[i]);  // Draw option text
            }
        }
        // Victory Screen: Player completed level 5 and won the game
        else if (currentState == STATE_VICTORY)
        {
            // Draw background and victory title
            window.draw(menuBackground);  // Background image
            window.draw(victoryTitle);    // "VICTORY!" title

            // Update and display final score (centered on screen)
            char victoryScoreBuffer[50];
            sprintf(victoryScoreBuffer, "Final Score: %d", score);  // Format final score
            victoryScore.setString(victoryScoreBuffer);
            // Center horizontally at y=200
            victoryScore.setPosition(windowWidth / 2 - victoryScore.getLocalBounds().width / 2.0f, 200);
            window.draw(victoryScore);

            // Draw victory menu options (2 options: Restart Game, Main Menu)
            for (int i = 0; i < 2; i++)
            {
                // Highlight selected option in yellow, others in white
                if (i == selectedMenuItem)
                {
                    victoryItems[i].setFillColor(Color::Yellow);  // Selected
                }
                else
                {
                    victoryItems[i].setFillColor(Color::White);   // Not selected
                }
                window.draw(victoryItems[i]);  // Draw option text
            }

            window.draw(victoryInstructions);  // "Press Enter to select" text
        }
        // Game Over Screen: Player lost all 3 lives
        else if (currentState == STATE_GAME_OVER)
        {
            // Draw background and game over title
            window.draw(menuBackground);  // Background image
            window.draw(gameOverTitle);   // "GAME OVER" title

            // Update and display final score (centered on screen)
            char gameOverScoreBuffer[50];
            sprintf(gameOverScoreBuffer, "Final Score: %d", score);  // Format final score
            gameOverScore.setString(gameOverScoreBuffer);
            // Center horizontally at y=200
            gameOverScore.setPosition(windowWidth / 2 - gameOverScore.getLocalBounds().width / 2.0f, 200);
            window.draw(gameOverScore);

            // Draw game over menu options (2 options: Restart Game, Main Menu)
            for (int i = 0; i < 2; i++)
            {
                // Highlight selected option in yellow, others in white
                if (i == selectedMenuItem)
                {
                    gameOverItems[i].setFillColor(Color::Yellow);  // Selected
                }
                else
                {
                    gameOverItems[i].setFillColor(Color::White);   // Not selected
                }
                window.draw(gameOverItems[i]);  // Draw option text
            }

            window.draw(gameOverInstructions);  // "Press Enter to select" text
        }

        window.display();  // Display everything drawn this frame to the screen
    }  // End of main game loop (continues until window is closed)

    return 0;  // Program exits successfully
}  // End of main function
