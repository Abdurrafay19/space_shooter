#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <ctime>
using namespace std;
using namespace sf;

// Grid Setup: Defines the dimensions and layout of the game grid
const int ROWS = 23;
const int COLS = 15;
const int CELL_SIZE = 40;
const int MARGIN = 40; // Margin around the grid for UI spacing
const float BULLET_OFFSET_X = (CELL_SIZE - CELL_SIZE * 0.3f) / 2.0f; // Center bullets horizontally
const float SHIELD_OFFSET = CELL_SIZE * -0.15f; // Center shield overlay

// Game States: Constants representing different screens/states of the game
const int STATE_MENU = 0;
const int STATE_PLAYING = 1;
const int STATE_INSTRUCTIONS = 2;
const int STATE_GAME_OVER = 3;
const int STATE_LEVEL_UP = 4;
const int STATE_VICTORY = 5;
const int STATE_PAUSED = 6;

int main()
{

    // Random Number Generator Setup: Seeds the random number generator with the current time
    srand(static_cast<unsigned int>(time(0)));

    // Window Setup: Calculates window size based on grid dimensions and creates the SFML window
    const int windowWidth = COLS * CELL_SIZE + MARGIN * 2 + 500; // Extra width for side panel (score, lives, etc.)
    const int windowHeight = ROWS * CELL_SIZE + MARGIN * 2;
    RenderWindow window(VideoMode(windowWidth, windowHeight), "Space Shooter");
    window.setFramerateLimit(60); // Limit frame rate to 60 FPS for smooth gameplay

    // Save File Handling: Load high score and saved game state
    int highScore = 0;
    int savedLives = 0;
    int savedScore = 0;
    int savedLevel = 0;
    bool hasSavedGame = false;
    string saveFile = "save-file.txt";
    
    ifstream inputFile(saveFile);
    if (inputFile.is_open())
    {
        // File format: highScore lives score level
        inputFile >> highScore >> savedLives >> savedScore >> savedLevel;
        inputFile.close();
        if (savedLevel > 0 && savedLives > 0)
        {
            hasSavedGame = true;
        }
    }
    else
    {
        // Create the file with initial values: highScore=0, no saved game
        ofstream createFile(saveFile);
        if (createFile.is_open())
        {
            createFile << "0 0 0 0";
            createFile.close();
        }
    }

    // Game Variables Setup: Initialize core game state variables
    int currentState = STATE_MENU;
    int selectedMenuItem = 0; // Index of the currently selected menu item
    int lives = 3;            // Player's remaining lives
    int score = 0;            // Current score (bonus from meteors)
    int killCount = 0;        // Count of enemies/bosses destroyed
    int level = 1;            // Current game level
    const int MAX_LEVEL = 5;  // Maximum level before victory
    bool isInvincible = false;                 // Flag for temporary invincibility after being hit
    Clock invincibilityTimer;                  // Timer to track invincibility duration
    const float INVINCIBILITY_DURATION = 1.0f; // Duration of invincibility in seconds

    // Level Up Effect Setup: Variables for the blinking text effect during level transitions
    Clock levelUpTimer;
    bool levelUpBlinkState = true; // Toggles visibility of level up text
    Clock levelUpBlinkClock;

    // Boss Mechanics Setup: Tracks boss movements to determine when to fire
    int bossMoveCounter = 0;

    // Grid System: 2D array representing the game board
    // 0 = Empty, 1 = Spaceship, 2 = Meteor, 3 = Bullet, 4 = Enemy, 5 = Boss, 6 = Boss Bullet
    // Note: Shield (when active) is rendered visually but doesn't occupy grid space
    // Shield powerups use separate array to avoid interference with other entities
    int grid[ROWS][COLS] = {0};
    
    // Separate grid for shield powerups (independent from main grid)
    const int MAX_SHIELD_POWERUPS = 5;
    int shieldPowerupRow[MAX_SHIELD_POWERUPS] = {-1, -1, -1, -1, -1};
    int shieldPowerupCol[MAX_SHIELD_POWERUPS] = {-1, -1, -1, -1, -1};
    bool shieldPowerupActive[MAX_SHIELD_POWERUPS] = {false, false, false, false, false};
    int shieldPowerupDirection[MAX_SHIELD_POWERUPS] = {0, 0, 0, 0, 0}; // -1=left, 0=down, 1=right

    // Shield System: Track if player has active shield
    bool hasShield = false;

    // Hit Effect System: Arrays to manage visual effects when entities are destroyed
    const int MAX_HIT_EFFECTS = 50;
    int hitEffectRow[MAX_HIT_EFFECTS] = {0};
    int hitEffectCol[MAX_HIT_EFFECTS] = {0};
    float hitEffectTimer[MAX_HIT_EFFECTS] = {0.0f};
    bool hitEffectActive[MAX_HIT_EFFECTS] = {false};
    const float HIT_EFFECT_DURATION = 0.3f; // Duration of the hit explosion effect

    // Spaceship Initialization: Place player at the bottom center
    int spaceshipCol = COLS / 2;      // Start column
    grid[ROWS - 1][spaceshipCol] = 1; // Mark grid position as spaceship
    Texture spaceshipTexture;
    if (!spaceshipTexture.loadFromFile("assets/images/player.png"))
    {
        cerr << "Failed to load spaceship texture" << endl;
        return -1;
    }
    Sprite spaceship;
    spaceship.setTexture(spaceshipTexture);
    // Scale sprite to fit within a single grid cell
    spaceship.setScale(
        static_cast<float>(CELL_SIZE) / spaceshipTexture.getSize().x,
        static_cast<float>(CELL_SIZE) / spaceshipTexture.getSize().y);

    // Life Icon Setup: Used for displaying remaining lives in the UI
    Texture lifeTexture;
    if (!lifeTexture.loadFromFile("assets/images/life.png"))
    {
        cerr << "Failed to load life texture" << endl;
        return -1;
    }
    Sprite lifeIcon;
    lifeIcon.setTexture(lifeTexture);
    // Scale life icon to be small (approx 24x24 pixels) to match UI text size
    lifeIcon.setScale(
        24.0f / lifeTexture.getSize().x,
        24.0f / lifeTexture.getSize().y);


    // Shield PowerUp and shield Setup: Visual representation of the shield power-up
    Texture shieldTexture;
    if (!shieldTexture.loadFromFile("assets/images/shield.png"))
    {
        cerr << "Failed to load shield texture" << endl;
        return -1;
    }  
    Sprite shieldIcon;
    shieldIcon.setTexture(shieldTexture);
    
    Texture shieldPowerUpTexture;
    if (!shieldPowerUpTexture.loadFromFile("assets/images/shield-powerup.png"))
    {
        cerr << "Failed to load shield power-up texture" << endl;
        return -1;
    }
    Sprite shieldPowerUp;
    shieldPowerUp.setTexture(shieldPowerUpTexture);
    shieldPowerUp.setScale(
        static_cast<float>(CELL_SIZE) / shieldPowerUpTexture.getSize().x,
        static_cast<float>(CELL_SIZE) / shieldPowerUpTexture.getSize().y);

    // Scale shield to be bigger than spaceship (1.3x)
    shieldIcon.setScale(
        static_cast<float>(CELL_SIZE * 1.3f) / shieldTexture.getSize().x,
        static_cast<float>(CELL_SIZE * 1.3f) / shieldTexture.getSize().y);

    // Game Background Setup: Background image for the playing area
    Texture bgTexture;
    if (!bgTexture.loadFromFile("assets/images/backgroundColor.png"))
    {
        cerr << "Failed to load background texture" << endl;
        return -1;
    }
    Sprite background;
    background.setTexture(bgTexture);
    // Scale background to cover the entire grid area
    background.setScale(
        static_cast<float>(COLS * CELL_SIZE) / bgTexture.getSize().x,
        static_cast<float>(ROWS * CELL_SIZE) / bgTexture.getSize().y);
    background.setPosition(MARGIN, MARGIN);

    // Game Border Setup: A black outline around the playing grid
    RectangleShape gameBox(Vector2f(COLS * CELL_SIZE, ROWS * CELL_SIZE));
    gameBox.setFillColor(Color::Transparent);
    gameBox.setOutlineThickness(5);
    gameBox.setOutlineColor(Color::Black);
    gameBox.setPosition(MARGIN, MARGIN);

    // Meteor Entity Setup
    Texture meteorTexture;
    if (!meteorTexture.loadFromFile("assets/images/meteorSmall.png"))
    {
        cerr << "Failed to load meteor texture" << endl;
        return -1;
    }
    Sprite meteor;
    meteor.setTexture(meteorTexture);
    meteor.setScale(
        static_cast<float>(CELL_SIZE) / meteorTexture.getSize().x,
        static_cast<float>(CELL_SIZE) / meteorTexture.getSize().y);

    /// Enemy Entities Setup

    // Standard Enemy Setup
    Texture enemyTexture;
    if (!enemyTexture.loadFromFile("assets/images/enemyUFO.png"))
    {
        cerr << "Failed to load enemy texture" << endl;
        return -1;
    }
    Sprite enemy;
    enemy.setTexture(enemyTexture);
    enemy.setScale(
        static_cast<float>(CELL_SIZE) / enemyTexture.getSize().x,
        static_cast<float>(CELL_SIZE) / enemyTexture.getSize().y);

    // Boss Enemy Setup
    Texture bossEnemyTexture;
    if (!bossEnemyTexture.loadFromFile("assets/images/enemyShip.png"))
    {
        cerr << "Failed to load boss enemy texture" << endl;
        return -1;
    }
    Sprite bossEnemy;
    bossEnemy.setTexture(bossEnemyTexture);
    bossEnemy.setScale(
        static_cast<float>(CELL_SIZE) / bossEnemyTexture.getSize().x,
        static_cast<float>(CELL_SIZE) / bossEnemyTexture.getSize().y);

    // Player Bullet Setup
    Texture bulletTexture;
    if (!bulletTexture.loadFromFile("assets/images/laserRed.png"))
    {
        cerr << "Failed to load bullet texture" << endl;
        return -1;
    }
    Sprite bullet;
    bullet.setTexture(bulletTexture);
    // Scale bullet to be narrower (30% width) and slightly shorter (80% height) than a cell
    bullet.setScale(
        static_cast<float>(CELL_SIZE * 0.3f) / bulletTexture.getSize().x,
        static_cast<float>(CELL_SIZE * 0.8f) / bulletTexture.getSize().y);

    // Bullet Impact Effect Setup
    Texture bulletHitTexture;
    if (!bulletHitTexture.loadFromFile("assets/images/laserRedShot.png"))
    {
        cerr << "Failed to load bullet hit texture" << endl;
        return -1;
    }
    Sprite bulletHit;
    bulletHit.setTexture(bulletHitTexture);
    bulletHit.setScale(
        static_cast<float>(CELL_SIZE) / bulletHitTexture.getSize().x,
        static_cast<float>(CELL_SIZE) / bulletHitTexture.getSize().y);

    // Boss Bullet Setup
    Texture bossBulletTexture;
    if (!bossBulletTexture.loadFromFile("assets/images/laserGreen.png"))
    {
        cerr << "Failed to load boss bullet texture" << endl;
        return -1;
    }
    Sprite bossBullet;
    bossBullet.setTexture(bossBulletTexture);
    bossBullet.setScale(
        static_cast<float>(CELL_SIZE * 0.3f) / bossBulletTexture.getSize().x,
        static_cast<float>(CELL_SIZE * 0.8f) / bossBulletTexture.getSize().y);

    // Boss Bullet Impact Effect Setup
    Texture bossBulletHitTexture;
    if (!bossBulletHitTexture.loadFromFile("assets/images/laserGreenShot.png"))
    {
        cerr << "Failed to load boss bullet hit texture" << endl;
        return -1;
    }
    Sprite bossBulletHit;
    bossBulletHit.setTexture(bossBulletHitTexture);
    bossBulletHit.setScale(
        static_cast<float>(CELL_SIZE) / bossBulletHitTexture.getSize().x,
        static_cast<float>(CELL_SIZE) / bossBulletHitTexture.getSize().y);

    // Main Menu Background Setup
    Texture menuBgTexture;
    if (!menuBgTexture.loadFromFile("assets/images/starBackground.png"))
    {
        cerr << "Failed to load menu background texture" << endl;
        return -1;
    }
    Sprite menuBackground;
    menuBackground.setTexture(menuBgTexture);
    // Scale menu background to cover the entire window
    menuBackground.setScale(
        static_cast<float>(windowWidth) / menuBgTexture.getSize().x,
        static_cast<float>(windowHeight) / menuBgTexture.getSize().y);
    menuBackground.setPosition(0, 0);

    // Font Loading
    Font font;
    if (!font.loadFromFile("assets/fonts/font.ttf"))
    {
        cerr << "Failed to load font" << endl;
        return -1;
    }

    // Sound Effects Setup
    Music bgMusic;
    if (!bgMusic.openFromFile("assets/sounds/bg-music.mp3"))
    {
        cerr << "Failed to load background music" << endl;
        return -1;
    }
    bgMusic.setLoop(true);
    bgMusic.setVolume(30);
    bgMusic.play();

    SoundBuffer shootBuffer, explosionBuffer, damageBuffer, levelUpBuffer;
    SoundBuffer menuClickBuffer, menuNavBuffer, winBuffer, loseBuffer;

    if (!shootBuffer.loadFromFile("assets/sounds/shoot.wav"))
    {
        cerr << "Failed to load shoot sound" << endl;
        return -1;
    }
    if (!explosionBuffer.loadFromFile("assets/sounds/explosion.wav"))
    {
        cerr << "Failed to load explosion sound" << endl;
        return -1;
    }
    if (!damageBuffer.loadFromFile("assets/sounds/damage.mp3"))
    {
        cerr << "Failed to load damage sound" << endl;
        return -1;
    }
    if (!levelUpBuffer.loadFromFile("assets/sounds/level-up.mp3"))
    {
        cerr << "Failed to load level up sound" << endl;
        return -1;
    }
    if (!menuClickBuffer.loadFromFile("assets/sounds/menu-click.mp3"))
    {
        cerr << "Failed to load menu click sound" << endl;
        return -1;
    }
    if (!menuNavBuffer.loadFromFile("assets/sounds/menu-navigate.wav"))
    {
        cerr << "Failed to load menu navigate sound" << endl;
        return -1;
    }
    if (!winBuffer.loadFromFile("assets/sounds/win.wav"))
    {
        cerr << "Failed to load win sound" << endl;
        return -1;
    }
    if (!loseBuffer.loadFromFile("assets/sounds/lose.wav"))
    {
        cerr << "Failed to load lose sound" << endl;
        return -1;
    }

    Sound shootSound, explosionSound, damageSound, levelUpSound;
    Sound menuClickSound, menuNavSound, winSound, loseSound;

    shootSound.setBuffer(shootBuffer);
    explosionSound.setBuffer(explosionBuffer);
    damageSound.setBuffer(damageBuffer);
    levelUpSound.setBuffer(levelUpBuffer);
    menuClickSound.setBuffer(menuClickBuffer);
    menuNavSound.setBuffer(menuNavBuffer);
    winSound.setBuffer(winBuffer);
    loseSound.setBuffer(loseBuffer);

    // Main Menu Title Setup
    Text menuTitle("SPACE SHOOTER", font, 40);
    menuTitle.setFillColor(Color::Yellow);
    // Center menu title horizontally
    menuTitle.setPosition(windowWidth / 2 - menuTitle.getLocalBounds().width / 2.0f, 100);

    // Main Menu Items Setup
    Text menuItems[4];
    const char menuTexts[4][20] = {"Start Game", "Load Saved Game", "Instructions", "Exit"};
    for (int i = 0; i < 4; i++)
    {
        menuItems[i].setFont(font);
        menuItems[i].setString(menuTexts[i]);
        menuItems[i].setCharacterSize(28);
        menuItems[i].setFillColor(Color::White);
        // Center each menu item horizontally under the title
        menuItems[i].setPosition(windowWidth / 2 - menuItems[i].getLocalBounds().width / 2.0f, 260 + i * 56);
    }

    // High Score Display for Main Menu
    Text menuHighScoreText("High Score: 0", font, 24);
    menuHighScoreText.setFillColor(Color::Yellow);
    menuHighScoreText.setPosition(windowWidth / 2 - menuHighScoreText.getLocalBounds().width / 2.0f, 180);

    // Menu Navigation Instructions Setup
    Text menuInstructions("Use UP/DOWN or W/S to navigate  |  ENTER to select", font, 18);
    menuInstructions.setFillColor(Color(150, 150, 150)); // Gray color
    menuInstructions.setPosition(windowWidth / 2 - menuInstructions.getLocalBounds().width / 2.0f, windowHeight - 80);

    /// In-Game UI Elements Setup

    // Game Title (displayed during gameplay)
    Text title("Space  Shooter", font, 28);
    title.setFillColor(Color::Yellow);
    // Place title to the right of the game grid
    title.setPosition(MARGIN + COLS * CELL_SIZE + 20, MARGIN);

    // Lives Display Text
    Text livesText("Lives:", font, 20);
    livesText.setFillColor(Color::White);
    livesText.setPosition(MARGIN + COLS * CELL_SIZE + 20, MARGIN + 150);

    // Score Display Text
    Text scoreText("Score: 0", font, 20);
    scoreText.setFillColor(Color::White);
    scoreText.setPosition(MARGIN + COLS * CELL_SIZE + 20, MARGIN + 200);

    // Enemies Killed Display Text
    Text killsText("Kills: 0/10", font, 20);
    killsText.setFillColor(Color::White);
    killsText.setPosition(MARGIN + COLS * CELL_SIZE + 20, MARGIN + 230);

    // Level Display Text
    Text levelText("Level: 1", font, 20);
    levelText.setFillColor(Color::White);
    levelText.setPosition(MARGIN + COLS * CELL_SIZE + 20, MARGIN + 280);

    // High Score Display (Playing Screen)
    Text highScoreText("High Score: 0", font, 20);
    highScoreText.setFillColor(Color::Yellow);
    highScoreText.setPosition(MARGIN + COLS * CELL_SIZE + 20, MARGIN + 330);

    // Game Over Screen Setup
    Text gameOverTitle("GAME OVER", font, 40);
    gameOverTitle.setFillColor(Color::Red);
    gameOverTitle.setPosition(windowWidth / 2 - gameOverTitle.getLocalBounds().width / 2.0f, 100);

    // Game Over Score Display
    Text gameOverScore("Final Score: 0", font, 28);
    gameOverScore.setFillColor(Color::Yellow);
    gameOverScore.setPosition(windowWidth / 2 - gameOverScore.getLocalBounds().width / 2.0f, 200);

    // Game Over Menu Items
    Text gameOverItems[2];
    const char gameOverTexts[2][20] = {"Restart", "Main Menu"};
    for (int i = 0; i < 2; i++)
    {
        gameOverItems[i].setFont(font);
        gameOverItems[i].setString(gameOverTexts[i]);
        gameOverItems[i].setCharacterSize(28);
        gameOverItems[i].setFillColor(Color::White);
        gameOverItems[i].setPosition(windowWidth / 2 - gameOverItems[i].getLocalBounds().width / 2.0f, 300 + i * 56);
    }

    // Game Over Navigation Instructions
    Text gameOverInstructions("Use UP/DOWN or W/S to navigate  |  ENTER to select", font, 18);
    gameOverInstructions.setFillColor(Color(150, 150, 150));
    gameOverInstructions.setPosition(windowWidth / 2 - gameOverInstructions.getLocalBounds().width / 2.0f, windowHeight - 80);

    // Level Up Screen Setup (Centered on the playing grid)
    Text levelUpText("LEVEL UP!", font, 40);
    levelUpText.setFillColor(Color::Green);
    // Calculate center of the grid area
    float gridCenterX = MARGIN + (COLS * CELL_SIZE) / 2.0f;
    float gridCenterY = MARGIN + (ROWS * CELL_SIZE) / 2.0f;
    levelUpText.setPosition(gridCenterX - levelUpText.getLocalBounds().width / 2.0f, gridCenterY - levelUpText.getLocalBounds().height / 2.0f - 10);

    // Pause Screen Setup
    Text pauseTitle("PAUSED", font, 40);
    pauseTitle.setFillColor(Color::Cyan);
    pauseTitle.setPosition(gridCenterX - pauseTitle.getLocalBounds().width / 2.0f, gridCenterY - 200);

    Text pauseItems[3];
    const char pauseTexts[3][20] = {"Resume", "Restart", "Save & Quit"};
    for (int i = 0; i < 3; i++)
    {
        pauseItems[i].setFont(font);
        pauseItems[i].setString(pauseTexts[i]);
        pauseItems[i].setCharacterSize(28);
        pauseItems[i].setFillColor(Color::White);
        pauseItems[i].setPosition(gridCenterX - pauseItems[i].getLocalBounds().width / 2.0f, gridCenterY - 50 + i * 56);
    }

    // Victory Screen Setup
    Text victoryTitle("VICTORY!", font, 40);
    victoryTitle.setFillColor(Color::Yellow);
    victoryTitle.setPosition(windowWidth / 2 - victoryTitle.getLocalBounds().width / 2.0f, 100);

    // Victory Score Display
    Text victoryScore("Final Score: 0", font, 28);
    victoryScore.setFillColor(Color::White);
    victoryScore.setPosition(windowWidth / 2 - victoryScore.getLocalBounds().width / 2.0f, 200);

    Text victoryItems[2];
    const char victoryTexts[2][20] = {"Restart", "Main Menu"};
    for (int i = 0; i < 2; i++)
    {
        victoryItems[i].setFont(font);
        victoryItems[i].setString(victoryTexts[i]);
        victoryItems[i].setCharacterSize(28);
        victoryItems[i].setFillColor(Color::White);
        victoryItems[i].setPosition(windowWidth / 2 - victoryItems[i].getLocalBounds().width / 2.0f, 300 + i * 56);
    }

    Text victoryInstructions("Use UP/DOWN or W/S to navigate  |  ENTER to select", font, 18);
    victoryInstructions.setFillColor(Color(150, 150, 150));
    victoryInstructions.setPosition(windowWidth / 2 - victoryInstructions.getLocalBounds().width / 2.0f, windowHeight - 80);

    // Instructions Screen Setup
    Text instructionsTitle("HOW TO PLAY", font, 40);
    instructionsTitle.setFillColor(Color::Yellow);
    instructionsTitle.setPosition(windowWidth / 2 - instructionsTitle.getLocalBounds().width / 2.0f, 40);

    // Control Instructions Section
    Text controlsTitle("CONTROLS", font, 24);
    controlsTitle.setFillColor(Color::Cyan);
    controlsTitle.setPosition(50, 100);

    Text moveText("Move Left/Right: A/D or Arrow Keys", font, 18);
    moveText.setFillColor(Color::White);
    moveText.setPosition(50, 140);

    Text shootText("Shoot: SPACEBAR", font, 18);
    shootText.setFillColor(Color::White);
    shootText.setPosition(50, 170);

    Text pauseText("Pause: P", font, 18);
    pauseText.setFillColor(Color::White);
    pauseText.setPosition(50, 200);

    // Entities Explanation Section
    Text entitiesTitle("ENTITIES", font, 24);
    entitiesTitle.setFillColor(Color::Cyan);
    entitiesTitle.setPosition(50, 250);

    Text playerDesc("Your Ship", font, 18);
    playerDesc.setFillColor(Color::White);
    playerDesc.setPosition(120, 290);

    Text meteorDesc("Meteor - Avoid!", font, 18);
    meteorDesc.setFillColor(Color::White);
    meteorDesc.setPosition(120, 330);

    Text enemyDesc("Enemy - 3 Points", font, 18);
    enemyDesc.setFillColor(Color::White);
    enemyDesc.setPosition(120, 370);

    Text bossDesc("Boss - 5 Points (Level 3+)", font, 18);
    bossDesc.setFillColor(Color::White);
    bossDesc.setPosition(120, 410);

    Text bulletDesc("Your Bullet", font, 18);
    bulletDesc.setFillColor(Color::White);
    bulletDesc.setPosition(120, 450);

    Text bossBulletDesc("Boss Bullet - Avoid!", font, 18);
    bossBulletDesc.setFillColor(Color::White);
    bossBulletDesc.setPosition(120, 490);

    Text lifeDesc("Life Icon", font, 18);
    lifeDesc.setFillColor(Color::White);
    lifeDesc.setPosition(120, 530);

    Text shieldPowerupDesc("Shield Powerup - Absorbs 1 Hit (Level 3+)", font, 18);
    shieldPowerupDesc.setFillColor(Color::White);
    shieldPowerupDesc.setPosition(120, 570);

    // Objective Section
    Text objectiveTitle("OBJECTIVE", font, 24);
    objectiveTitle.setFillColor(Color::Cyan);
    objectiveTitle.setPosition(50, 620);

    Text objective1("- Destroy enemies (3 pts) and bosses (5 pts)", font, 18);
    objective1.setFillColor(Color::White);
    objective1.setPosition(50, 660);

    Text objective2("- Destroy meteors for 1-2 bonus points", font, 18);
    objective2.setFillColor(Color::White);
    objective2.setPosition(50, 690);

    Text objective3("- Each level: Destroy (Level x 10) enemies/bosses", font, 18);
    objective3.setFillColor(Color::White);
    objective3.setPosition(50, 720);

    Text objective4("- Complete Level 5 to win! Don't lose 3 lives!", font, 18);
    objective4.setFillColor(Color::White);
    objective4.setPosition(50, 750);

    Text instructionsBack("Press ESC or BACKSPACE to return to menu", font, 18);
    instructionsBack.setFillColor(Color(150, 150, 150));
    instructionsBack.setPosition(windowWidth / 2 - instructionsBack.getLocalBounds().width / 2.0f, windowHeight - 80);

    // Game Timing Clocks Setup

    // Movement Cooldown: Prevents overly sensitive controls
    Clock moveClock;
    Time moveCooldown = milliseconds(100); // 100ms delay between movements

    // Meteor Spawning Logic
    Clock meteorSpawnClock;
    Clock meteorMoveClock;
    float nextSpawnTime = 1.0f + (rand() % 3); // Random interval between 1-3 seconds

    // Enemy Spawning Logic
    Clock enemySpawnClock;
    Clock enemyMoveClock;
    float nextEnemySpawnTime = 2.0f + (rand() % 4); // Random interval between 2-5 seconds

    // Boss Spawning Logic (Bosses appear after level 3)
    Clock bossSpawnClock;
    Clock bossMoveClock;
    Clock bossBulletMoveClock;
    float nextBossSpawnTime = 8.0f + (rand() % 5); // Random interval between 8-12 seconds

    // Shield Powerup Spawning Logic (Spawns after level 3)
    Clock shieldPowerupSpawnClock;
    Clock shieldPowerupMoveClock;
    float nextShieldPowerupSpawnTime = 15.0f + (rand() % 10); // Random interval between 15-25 seconds

    // Bullet Firing Logic
    Clock bulletMoveClock;
    Clock bulletFireClock;
    Time bulletFireCooldown = milliseconds(300); // Fire rate limit: 0.3 seconds

    // Hit Effect Timer
    Clock hitEffectClock;

    // Menu Navigation Cooldown
    Clock menuClock;
    Time menuCooldown = milliseconds(200);

    // Main Game Loop: Runs until the window is closed
    while (window.isOpen())
    {

        // Event Polling: Handle window close events
        Event event;
        while (window.pollEvent(event))
        {
            if (event.type == Event::Closed)
                window.close();
        }

        // State Machine: Handle logic based on current game state
        if (currentState == STATE_MENU)
        {
            // Menu Logic: Handle navigation and selection
            if (menuClock.getElapsedTime() >= menuCooldown)
            {
                bool menuAction = false;

                // Navigate menu up/down
                if (Keyboard::isKeyPressed(Keyboard::Up) || Keyboard::isKeyPressed(Keyboard::W))
                {
                    selectedMenuItem = (selectedMenuItem - 1 + 4) % 4;
                    menuNavSound.play();
                    menuAction = true;
                }
                else if (Keyboard::isKeyPressed(Keyboard::Down) || Keyboard::isKeyPressed(Keyboard::S))
                {
                    selectedMenuItem = (selectedMenuItem + 1) % 4;
                    menuNavSound.play();
                    menuAction = true;
                }
                // Select menu item
                else if (Keyboard::isKeyPressed(Keyboard::Enter))
                {
                    menuClickSound.play();
                    if (selectedMenuItem == 0)
                    { // Start Game
                        bgMusic.stop();
                        currentState = STATE_PLAYING;
                        // Reset game state variables for a new game
                        lives = 3;
                        score = 0;
                        killCount = 0;
                        level = 1;
                        bossMoveCounter = 0;
                        hasShield = false;
                        // Clear the grid
                        for (int r = 0; r < ROWS; r++)
                        {
                            for (int c = 0; c < COLS; c++)
                            {
                                grid[r][c] = 0;
                            }
                        }
                        // Clear shield powerups
                        for (int i = 0; i < MAX_SHIELD_POWERUPS; i++)
                        {
                            shieldPowerupActive[i] = false;
                        }
                        // Reset spaceship position
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
                    else if (selectedMenuItem == 1)
                    { // Load Saved Game
                        if (hasSavedGame)
                        {
                            bgMusic.stop();
                            currentState = STATE_PLAYING;
                            // Restore saved game state
                            lives = savedLives;
                            score = savedScore;
                            killCount = 0; // Always start kills from 0
                            level = savedLevel;
                            bossMoveCounter = 0;
                            hasShield = false;
                            // Clear the grid
                            for (int r = 0; r < ROWS; r++)
                            {
                                for (int c = 0; c < COLS; c++)
                                {
                                    grid[r][c] = 0;
                                }
                            }
                            // Clear shield powerups
                            for (int i = 0; i < MAX_SHIELD_POWERUPS; i++)
                            {
                                shieldPowerupActive[i] = false;
                            }
                            // Reset spaceship position
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
                        else
                        {
                        }
                    }
                    else if (selectedMenuItem == 2)
                    { // Instructions
                        currentState = STATE_INSTRUCTIONS;
                    }
                    else if (selectedMenuItem == 3)
                    { // Exit
                        bgMusic.stop();
                        window.close();
                    }
                    menuAction = true;
                }

                if (menuAction)
                {
                    menuClock.restart();
                }
            }

            // Update menu item colors to highlight selection
            for (int i = 0; i < 4; i++)
            {
                if (i == selectedMenuItem)
                {
                    menuItems[i].setFillColor(Color::Yellow);
                }
                else
                {
                    menuItems[i].setFillColor(Color::White);
                }
            }
        }
        // Game Over State Logic
        else if (currentState == STATE_GAME_OVER)
        {
            // Game over menu navigation with cooldown
            if (menuClock.getElapsedTime() >= menuCooldown)
            {
                bool menuAction = false;

                // Navigate game over menu up/down
                if (Keyboard::isKeyPressed(Keyboard::Up) || Keyboard::isKeyPressed(Keyboard::W))
                {
                    selectedMenuItem = (selectedMenuItem - 1 + 2) % 2;
                    menuNavSound.play();
                    menuAction = true;
                }
                else if (Keyboard::isKeyPressed(Keyboard::Down) || Keyboard::isKeyPressed(Keyboard::S))
                {
                    selectedMenuItem = (selectedMenuItem + 1) % 2;
                    menuNavSound.play();
                    menuAction = true;
                }
                // Select game over menu item
                else if (Keyboard::isKeyPressed(Keyboard::Enter))
                {
                    menuClickSound.play();
                    if (selectedMenuItem == 0)
                    { // Restart Game
                        currentState = STATE_PLAYING;
                        // Reset game state
                        lives = 3;
                        score = 0;
                        killCount = 0;
                        level = 1;
                        bossMoveCounter = 0;
                        hasShield = false;
                        for (int r = 0; r < ROWS; r++)
                        {
                            for (int c = 0; c < COLS; c++)
                            {
                                grid[r][c] = 0;
                            }
                        }
                        spaceshipCol = COLS / 2;
                        grid[ROWS - 1][spaceshipCol] = 1;
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
                    { // Return to Main Menu
                        if (bgMusic.getStatus() != Music::Playing)
                        {
                            bgMusic.play();
                        }
                        currentState = STATE_MENU;
                        selectedMenuItem = 0;
                    }
                    menuAction = true;
                }

                if (menuAction)
                {
                    menuClock.restart();
                }
            }

            // Update game over item colors based on selection
            for (int i = 0; i < 2; i++)
            {
                if (i == selectedMenuItem)
                {
                    gameOverItems[i].setFillColor(Color::Yellow);
                }
                else
                {
                    gameOverItems[i].setFillColor(Color::White);
                }
            }
        }
        // Instructions Screen Logic
        else if (currentState == STATE_INSTRUCTIONS)
        {
            // Check for input to return to menu
            if (menuClock.getElapsedTime() >= menuCooldown)
            {
                if (Keyboard::isKeyPressed(Keyboard::Escape) || Keyboard::isKeyPressed(Keyboard::BackSpace))
                {
                    menuClickSound.play();
                    currentState = STATE_MENU;
                    selectedMenuItem = 0;
                    menuClock.restart();
                }
            }
        }
        // Gameplay State Logic: Main game mechanics happen here
        else if (currentState == STATE_PLAYING)
        {
            // Check for pause input
            if (menuClock.getElapsedTime() >= menuCooldown)
            {
                if (Keyboard::isKeyPressed(Keyboard::P))
                {
                    currentState = STATE_PAUSED;
                    selectedMenuItem = 0;
                    menuClock.restart();
                }
            }

            // Player Movement Logic: Handle Left/Right input
            if (moveClock.getElapsedTime() >= moveCooldown)
            {
                bool moved = false;
                // Move Left
                if ((Keyboard::isKeyPressed(Keyboard::Left) || Keyboard::isKeyPressed(Keyboard::A)) && spaceshipCol > 0)
                {
                    grid[ROWS - 1][spaceshipCol] = 0; // Clear old position
                    spaceshipCol--;
                    grid[ROWS - 1][spaceshipCol] = 1; // Set new position
                    moved = true;
                }
                // Move Right
                else if ((Keyboard::isKeyPressed(Keyboard::Right) || Keyboard::isKeyPressed(Keyboard::D)) && spaceshipCol < COLS - 1)
                {
                    grid[ROWS - 1][spaceshipCol] = 0; // Clear old position
                    spaceshipCol++;
                    grid[ROWS - 1][spaceshipCol] = 1; // Set new position
                    moved = true;
                }

                if (moved)
                {
                    moveClock.restart();
                }
            }

            // Player Shooting Logic
            if (Keyboard::isKeyPressed(Keyboard::Space) && bulletFireClock.getElapsedTime() >= bulletFireCooldown)
            {
                // Fire bullet from position immediately above spaceship
                int bulletRow = ROWS - 2;
                if (bulletRow >= 0 && grid[bulletRow][spaceshipCol] == 0)
                {
                    grid[bulletRow][spaceshipCol] = 3; // 3 represents bullet
                    shootSound.play();
                }
                bulletFireCooldown = milliseconds(300);
                bulletFireClock.restart();
            }

            // Meteor Spawning Logic
            if (meteorSpawnClock.getElapsedTime().asSeconds() >= nextSpawnTime)
            {
                int randomCol = rand() % COLS;
                // Only spawn if the top row at that column is empty
                if (grid[0][randomCol] == 0)
                {
                    grid[0][randomCol] = 2; // 2 represents meteor
                }
                meteorSpawnClock.restart();
                nextSpawnTime = 1.0f + (rand() % 3); // Next spawn in 1-3 seconds
            }

            // Enemy Spawning Logic (Spawn rate increases with level)
            if (enemySpawnClock.getElapsedTime().asSeconds() >= nextEnemySpawnTime)
            {
                int randomCol = rand() % COLS;
                // Only spawn if the top row at that column is empty
                if (grid[0][randomCol] == 0)
                {
                    grid[0][randomCol] = 4; // 4 represents enemy
                }
                enemySpawnClock.restart();
                // Calculate next spawn time based on level difficulty
                float baseTime = 2.5f - (level * 0.4f);
                float variance = 3.0f - (level * 0.4f);
                if (baseTime < 0.5f)
                    baseTime = 0.5f;
                if (variance < 1.0f)
                    variance = 1.0f;
                nextEnemySpawnTime = baseTime + (rand() % (int)variance);
            }

            // Boss Spawning Logic (Only spawns at Level 3 and above)
            if (level >= 3 && bossSpawnClock.getElapsedTime().asSeconds() >= nextBossSpawnTime)
            {
                int randomCol = rand() % COLS;
                // Only spawn if the top row at that column is empty
                if (grid[0][randomCol] == 0)
                {
                    grid[0][randomCol] = 5; // 5 represents boss
                }
                bossSpawnClock.restart();
                // Calculate next boss spawn time based on level
                float bossBaseTime = 10.0f - ((level - 3) * 1.5f);
                float bossVariance = 4.0f;
                if (bossBaseTime < 5.0f)
                    bossBaseTime = 5.0f;
                nextBossSpawnTime = bossBaseTime + (rand() % (int)bossVariance);
            }

            // Shield Powerup Spawning Logic (Only spawns at Level 3 and above, rare spawn)
            if (level >= 3 && shieldPowerupSpawnClock.getElapsedTime().asSeconds() >= nextShieldPowerupSpawnTime)
            {
                // Find an inactive slot for new powerup
                for (int i = 0; i < MAX_SHIELD_POWERUPS; i++)
                {
                    if (!shieldPowerupActive[i])
                    {
                        int randomCol = rand() % COLS;
                        shieldPowerupRow[i] = 0;
                        shieldPowerupCol[i] = randomCol;
                        shieldPowerupActive[i] = true;
                        shieldPowerupDirection[i] = 0; // Start moving down
                        break;
                    }
                }
                shieldPowerupSpawnClock.restart();
                // Calculate next shield powerup spawn time (rarer at level 3-4, less rare at level 5)
                float shieldBaseTime;
                float shieldVariance;
                if (level < 5)
                {
                    shieldBaseTime = 20.0f; // Rare spawn at levels 3-4
                    shieldVariance = 15.0f;
                }
                else
                {
                    shieldBaseTime = 12.0f; // Less rare at level 5
                    shieldVariance = 8.0f;
                }
                nextShieldPowerupSpawnTime = shieldBaseTime + (rand() % (int)shieldVariance);
            }

            // Meteor Movement Logic (Speed scales with level, same as enemies)
            float meteorMoveSpeed = 0.833f - ((level - 1) * 0.1f);
            if (meteorMoveSpeed < 0.333f)
                meteorMoveSpeed = 0.333f;
            if (meteorMoveClock.getElapsedTime().asSeconds() >= meteorMoveSpeed)
            {
                // Iterate from bottom to top to avoid overwriting entities as they move down
                for (int r = ROWS - 1; r >= 0; r--)
                {
                    for (int c = 0; c < COLS; c++)
                    {
                        if (grid[r][c] == 2)
                        { // Found a meteor
                            // Check if meteor reached the bottom of the grid
                            if (r == ROWS - 1)
                            {
                                grid[r][c] = 0; // Remove meteor
                            }
                            else
                            {
                                grid[r][c] = 0; // Clear current position
                                // Move down if next cell is empty or contains another meteor
                                if (grid[r + 1][c] == 0 || grid[r + 1][c] == 2)
                                {
                                    grid[r + 1][c] = 2;
                                }
                                // Collision with Spaceship
                                else if (grid[r + 1][c] == 1)
                                {
                                    // Check if player has shield
                                    if (hasShield)
                                    {
                                        hasShield = false; // Shield absorbs the hit
                                    }
                                    else if (!isInvincible)
                                    {
                                        lives--;
                                        damageSound.play();
                                        isInvincible = true;
                                        invincibilityTimer.restart();
                                        // Check for Game Over
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
                                    grid[r + 1][c] = 0; // Remove meteor
                                }
                                // Collision with Player Bullet
                                else if (grid[r + 1][c] == 3)
                                {
                                    int meteorPoints = 1 + (rand() % 2); // Random 1 or 2 points
                                    score += meteorPoints;
                                    explosionSound.play();
                                    grid[r + 1][c] = 0; // Destroy both meteor and bullet
                                    // Trigger hit effect
                                    for (int i = 0; i < MAX_HIT_EFFECTS; i++)
                                    {
                                        if (!hitEffectActive[i])
                                        {
                                            hitEffectRow[i] = r + 1;
                                            hitEffectCol[i] = c;
                                            hitEffectTimer[i] = 0.0f;
                                            hitEffectActive[i] = true;
                                            break;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                meteorMoveClock.restart();
            }

            // Shield Powerup Movement Logic (Random zigzag movement, separate from main grid)
            if (shieldPowerupMoveClock.getElapsedTime().asSeconds() >= 0.5f)
            {
                for (int i = 0; i < MAX_SHIELD_POWERUPS; i++)
                {
                    if (shieldPowerupActive[i])
                    {
                        // Check if powerup reached the bottom
                        if (shieldPowerupRow[i] >= ROWS - 1)
                        {
                            shieldPowerupActive[i] = false;
                            continue;
                        }

                        // Check collision with spaceship BEFORE moving
                        if (grid[shieldPowerupRow[i]][shieldPowerupCol[i]] == 1)
                        {
                            hasShield = true;
                            levelUpSound.play(); // Use level up sound for powerup collection
                            shieldPowerupActive[i] = false;
                            continue;
                        }

                        // Move down
                        shieldPowerupRow[i]++;

                        // Random horizontal movement (zigzag)
                        int randomMove = rand() % 3; // 0=left, 1=stay, 2=right
                        if (randomMove == 0 && shieldPowerupCol[i] > 0)
                        {
                            shieldPowerupCol[i]--; // Move left
                            shieldPowerupDirection[i] = -1;
                        }
                        else if (randomMove == 2 && shieldPowerupCol[i] < COLS - 1)
                        {
                            shieldPowerupCol[i]++; // Move right
                            shieldPowerupDirection[i] = 1;
                        }
                        else
                        {
                            shieldPowerupDirection[i] = 0; // Stay in same column
                        }

                        // Check collision with spaceship AFTER moving
                        if (grid[shieldPowerupRow[i]][shieldPowerupCol[i]] == 1)
                        {
                            hasShield = true;
                            levelUpSound.play(); // Use level up sound for powerup collection
                            shieldPowerupActive[i] = false;
                        }
                    }
                }
                shieldPowerupMoveClock.restart();
            }

            // Enemy Movement Logic (Speed increases with level)
            float enemyMoveSpeed = 0.833f - ((level - 1) * 0.1f);
            if (enemyMoveClock.getElapsedTime().asSeconds() >= enemyMoveSpeed)
            {
                // Iterate from bottom to top
                for (int r = ROWS - 1; r >= 0; r--)
                {
                    for (int c = 0; c < COLS; c++)
                    {
                        if (grid[r][c] == 4)
                        { // Found an enemy
                            // Check if enemy reached the bottom (Player loses a life)
                            if (r == ROWS - 1)
                            {
                                grid[r][c] = 0; // Remove enemy
                                lives--;        // Penalty for letting enemy escape
                                damageSound.play();
                                isInvincible = true;
                                invincibilityTimer.restart();
                                // Check for Game Over
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
                            else
                            {
                                grid[r][c] = 0; // Clear current position
                                // Move down if next cell is empty or contains another enemy
                                if (grid[r + 1][c] == 0 || grid[r + 1][c] == 4)
                                {
                                    grid[r + 1][c] = 4;
                                }
                                // Collision with Spaceship
                                else if (grid[r + 1][c] == 1)
                                {
                                    // Check if player has shield
                                    if (hasShield)
                                    {
                                        hasShield = false; // Shield absorbs the hit
                                        explosionSound.play();
                                    }
                                    else if (!isInvincible)
                                    {
                                        lives--;
                                        damageSound.play();
                                        isInvincible = true;
                                        invincibilityTimer.restart();
                                        // Check for Game Over
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
                                    grid[r + 1][c] = 0; // Remove enemy
                                }
                                // Collision with Player Bullet
                                else if (grid[r + 1][c] == 3)
                                {
                                    score += 3; // Award 3 points for enemy
                                    killCount++; // Increment kill counter
                                    explosionSound.play();
                                    grid[r + 1][c] = 0; // Destroy both
                                    // Trigger hit effect
                                    for (int i = 0; i < MAX_HIT_EFFECTS; i++)
                                    {
                                        if (!hitEffectActive[i])
                                        {
                                            hitEffectRow[i] = r + 1;
                                            hitEffectCol[i] = c;
                                            hitEffectTimer[i] = 0.0f;
                                            hitEffectActive[i] = true;
                                            break;
                                        }
                                    }
                                    // Check for Level Up Condition (based on kills, not score)
                                    int killsNeeded = level * 10;
                                    if (level < MAX_LEVEL && killCount >= killsNeeded)
                                    {
                                        level++;
                                        levelUpSound.play();
                                        killCount = 0;       // Reset kill counter for next level
                                        bossMoveCounter = 0; // Reset boss firing counter
                                        // Clear grid of all entities
                                        for (int r = 0; r < ROWS; r++)
                                        {
                                            for (int c = 0; c < COLS; c++)
                                            {
                                                if (grid[r][c] >= 2 && grid[r][c] <= 6)
                                                {
                                                    grid[r][c] = 0;
                                                }
                                            }
                                        }
                                        // Reset spaceship position
                                        grid[ROWS - 1][spaceshipCol] = 0;
                                        spaceshipCol = COLS / 2;
                                        grid[ROWS - 1][spaceshipCol] = 1;
                                        // Transition to Level Up State
                                        currentState = STATE_LEVEL_UP;
                                        levelUpTimer.restart();
                                        levelUpBlinkClock.restart();
                                    }
                                    else if (level >= MAX_LEVEL && killCount >= killsNeeded)
                                    {
                                        // Victory Condition Met
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
                                        winSound.play();
                                        currentState = STATE_VICTORY;
                                        selectedMenuItem = 0;
                                    }
                                }
                            }
                        }
                    }
                }
                enemyMoveClock.restart();
            }

            // Boss Movement Logic (Speed increases with level)
            float bossMoveSpeed = 0.8f - ((level - 3) * 0.1f);
            if (bossMoveSpeed < 0.5f)
                bossMoveSpeed = 0.5f;
            if (bossMoveClock.getElapsedTime().asSeconds() >= bossMoveSpeed)
            {
                // Iterate from bottom to top
                for (int r = ROWS - 1; r >= 0; r--)
                {
                    for (int c = 0; c < COLS; c++)
                    {
                        if (grid[r][c] == 5)
                        { // Found a boss
                            // Check if boss reached the bottom (Player loses a life)
                            if (r == ROWS - 1)
                            {
                                grid[r][c] = 0; // Remove boss
                                lives--;        // Penalty
                                damageSound.play();
                                isInvincible = true;
                                invincibilityTimer.restart();
                                // Check for Game Over
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
                            else
                            {
                                int nextRow = r + 1;
                                int nextCell = grid[nextRow][c];

                                grid[r][c] = 0; // Clear current position

                                // Move down if next cell is empty or contains other enemies/projectiles
                                if (nextCell == 0 || nextCell == 5 || nextCell == 6 || nextCell == 2 || nextCell == 4)
                                {
                                    grid[nextRow][c] = 5;
                                }
                                // Collision with Spaceship
                                else if (grid[r + 1][c] == 1)
                                {
                                    // Check if player has shield
                                    if (hasShield)
                                    {
                                        hasShield = false; // Shield absorbs the hit
                                        explosionSound.play();
                                    }
                                    else if (!isInvincible)
                                    {
                                        lives--;
                                        damageSound.play();
                                        isInvincible = true;
                                        invincibilityTimer.restart();
                                        // Check for Game Over
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
                                // Collision with Player Bullet
                                else if (grid[r + 1][c] == 3)
                                {
                                    score += 5; // Award 5 points for boss
                                    killCount++; // Increment kill counter
                                    explosionSound.play();
                                    grid[r + 1][c] = 0; // Destroy both
                                    // Trigger hit effect
                                    for (int i = 0; i < MAX_HIT_EFFECTS; i++)
                                    {
                                        if (!hitEffectActive[i])
                                        {
                                            hitEffectRow[i] = r + 1;
                                            hitEffectCol[i] = c;
                                            hitEffectTimer[i] = 0.0f;
                                            hitEffectActive[i] = true;
                                            break;
                                        }
                                    }
                                    // Check for Level Up Condition (based on kills)
                                    int killsNeeded = level * 10;
                                    if (level < MAX_LEVEL && killCount >= killsNeeded)
                                    {
                                        level++;
                                        levelUpSound.play();
                                        killCount = 0;
                                        bossMoveCounter = 0;
                                        // Clear grid
                                        for (int r = 0; r < ROWS; r++)
                                        {
                                            for (int c = 0; c < COLS; c++)
                                            {
                                                if (grid[r][c] >= 2 && grid[r][c] <= 6)
                                                {
                                                    grid[r][c] = 0;
                                                }
                                            }
                                        }
                                        // Reset spaceship
                                        grid[ROWS - 1][spaceshipCol] = 0;
                                        spaceshipCol = COLS / 2;
                                        grid[ROWS - 1][spaceshipCol] = 1;
                                        // Transition to Level Up
                                        currentState = STATE_LEVEL_UP;
                                        levelUpTimer.restart();
                                        levelUpBlinkClock.restart();
                                    }
                                    else if (level >= MAX_LEVEL && killCount >= killsNeeded)
                                    {
                                        // Victory Condition
                                        winSound.play();
                                        currentState = STATE_VICTORY;
                                        selectedMenuItem = 0;
                                    }
                                }
                            }
                        }
                    }
                }

                // Boss Firing Logic
                bossMoveCounter++;

                // Determine firing frequency based on level (slower intervals)
                float firingInterval;
                if (level == 3)
                {
                    firingInterval = 5; // Fire every 5th movement
                }
                else if (level == 4)
                {
                    firingInterval = 4; // Fire every 4th movement
                }
                else
                {                       // Level 5
                    firingInterval = 3; // Fire every 3rd movement
                }

                // Execute Boss Firing
                if (bossMoveCounter >= firingInterval)
                {
                    for (int r = 0; r < ROWS; r++)
                    {
                        for (int c = 0; c < COLS; c++)
                        {
                            if (grid[r][c] == 5)
                            { // Found a boss
                                // Fire bullet downwards
                                if (r < ROWS - 1)
                                {
                                    int bulletRow = r + 1;
                                    if (bulletRow < ROWS && grid[bulletRow][c] == 0)
                                    {
                                        grid[bulletRow][c] = 6; // 6 represents boss bullet
                                    }
                                }
                            }
                        }
                    }
                    bossMoveCounter = 0; // Reset counter
                }

                bossMoveClock.restart();
            }

            // Boss Bullet Movement Logic (Moves faster than other entities)
            float bossBulletSpeed = 0.15f; // Fast constant speed regardless of level
            if (bossBulletMoveClock.getElapsedTime().asSeconds() >= bossBulletSpeed)
            {
                // Iterate from bottom to top
                for (int r = ROWS - 1; r >= 0; r--)
                {
                    for (int c = 0; c < COLS; c++)
                    {
                        if (grid[r][c] == 6)
                        { // Found a boss bullet
                            // Check if bullet reached the bottom
                            if (r == ROWS - 1)
                            {
                                grid[r][c] = 0; // Remove bullet
                            }
                            else
                            {
                                grid[r][c] = 0; // Clear current position
                                // Collision with Spaceship
                                if (grid[r + 1][c] == 1)
                                {
                                    // Check if player has shield
                                    if (hasShield)
                                    {
                                        hasShield = false; // Shield absorbs the hit
                                        explosionSound.play();
                                    }
                                    else if (!isInvincible)
                                    {
                                        lives--;
                                        damageSound.play();
                                        isInvincible = true;
                                        invincibilityTimer.restart();
                                        // Check for Game Over
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
                                    // Trigger hit effect
                                    for (int i = 0; i < MAX_HIT_EFFECTS; i++)
                                    {
                                        if (!hitEffectActive[i])
                                        {
                                            hitEffectRow[i] = r + 1;
                                            hitEffectCol[i] = c;
                                            hitEffectTimer[i] = 0.0f;
                                            hitEffectActive[i] = true;
                                            break;
                                        }
                                    }
                                }
                                // Pass through other enemies/meteors
                                else if (grid[r + 1][c] == 2 || grid[r + 1][c] == 4)
                                {
                                    grid[r + 1][c] = 6; // Bullet continues, entity stays
                                }
                                // Move to empty space or overwrite another boss bullet
                                else if (grid[r + 1][c] == 0 || grid[r + 1][c] == 6)
                                {
                                    grid[r + 1][c] = 6;
                                }
                            }
                        }
                    }
                }
                bossBulletMoveClock.restart();
            }

            // Player Bullet Movement Logic (Moves up)
            if (bulletMoveClock.getElapsedTime().asSeconds() >= 0.05f)
            {
                // Iterate from top to bottom to avoid overwriting
                for (int r = 0; r < ROWS; r++)
                {
                    for (int c = 0; c < COLS; c++)
                    {
                        if (grid[r][c] == 3)
                        { // Found a player bullet
                            // Check if bullet reached the top
                            if (r == 0)
                            {
                                grid[r][c] = 0; // Remove bullet
                            }
                            else
                            {
                                grid[r][c] = 0; // Clear current position
                                // Move up if next cell is empty, contains another bullet, or has shield powerup (pass through)
                                if (grid[r - 1][c] == 0 || grid[r - 1][c] == 3 || grid[r - 1][c] == 7)
                                {
                                    if (grid[r - 1][c] != 7) // Only overwrite if not shield powerup
                                        grid[r - 1][c] = 3;
                                }
                                // Collision with Boss Bullet
                                else if (grid[r - 1][c] == 6)
                                {
                                    explosionSound.play();
                                    grid[r - 1][c] = 0; // Destroy both
                                    // Trigger hit effect
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
                                // Collision with Meteor
                                else if (grid[r - 1][c] == 2)
                                {
                                    int meteorPoints = 1 + (rand() % 2); // Random 1 or 2 points
                                    score += meteorPoints;
                                    explosionSound.play();
                                    grid[r - 1][c] = 0; // Destroy both
                                    // Trigger hit effect
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
                                // Collision with Enemy
                                else if (grid[r - 1][c] == 4)
                                {
                                    score += 3; // Award 3 points
                                    killCount++; // Increment kill counter
                                    explosionSound.play();
                                    grid[r - 1][c] = 0; // Destroy both
                                    // Trigger hit effect
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
                                    // Check for Level Up
                                    int killsNeeded = level * 10;
                                    if (level < MAX_LEVEL && killCount >= killsNeeded)
                                    {
                                        level++;
                                        levelUpSound.play();
                                        killCount = 0;
                                        bossMoveCounter = 0;
                                        // Clear grid
                                        for (int r = 0; r < ROWS; r++)
                                        {
                                            for (int c = 0; c < COLS; c++)
                                            {
                                                if (grid[r][c] >= 2 && grid[r][c] <= 6)
                                                {
                                                    grid[r][c] = 0;
                                                }
                                            }
                                        }
                                        // Reset spaceship
                                        grid[ROWS - 1][spaceshipCol] = 0;
                                        spaceshipCol = COLS / 2;
                                        grid[ROWS - 1][spaceshipCol] = 1;
                                        // Transition to Level Up
                                        currentState = STATE_LEVEL_UP;
                                        levelUpTimer.restart();
                                        levelUpBlinkClock.restart();
                                    }
                                    else if (level >= MAX_LEVEL && killCount >= killsNeeded)
                                    {
                                        // Victory Condition
                                        // Update high score if current score is higher
                                        if (score > highScore)
                                        {
                                            highScore = score;
                                            ofstream outputFile(saveFile);
                                            if (outputFile.is_open())
                                            {
                                                outputFile << highScore << " 0 0 0 0"; // Clear saved game
                                                outputFile.close();
                                                hasSavedGame = false;
                                            }
                                        }
                                        else
                                        {
                                            // Clear saved game on victory
                                            ofstream outputFile(saveFile);
                                            if (outputFile.is_open())
                                            {
                                                outputFile << highScore << " 0 0 0 0";
                                                outputFile.close();
                                                hasSavedGame = false;
                                            }
                                        }
                                        winSound.play();
                                        currentState = STATE_VICTORY;
                                        selectedMenuItem = 0;
                                    }
                                }
                                // Collision with Boss
                                else if (grid[r - 1][c] == 5)
                                {
                                    score += 5; // Award 5 points
                                    killCount++; // Increment kill counter
                                    explosionSound.play();
                                    grid[r - 1][c] = 0; // Destroy both
                                    // Trigger hit effect
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
                                    // Check for Level Up
                                    int killsNeeded = level * 10;
                                    if (level < MAX_LEVEL && killCount >= killsNeeded)
                                    {
                                        level++;
                                        levelUpSound.play();
                                        killCount = 0;
                                        bossMoveCounter = 0;
                                        // Clear grid
                                        for (int r = 0; r < ROWS; r++)
                                        {
                                            for (int c = 0; c < COLS; c++)
                                            {
                                                if (grid[r][c] >= 2 && grid[r][c] <= 6)
                                                {
                                                    grid[r][c] = 0;
                                                }
                                            }
                                        }
                                        // Reset spaceship
                                        grid[ROWS - 1][spaceshipCol] = 0;
                                        spaceshipCol = COLS / 2;
                                        grid[ROWS - 1][spaceshipCol] = 1;
                                        // Transition to Level Up
                                        currentState = STATE_LEVEL_UP;
                                        levelUpTimer.restart();
                                        levelUpBlinkClock.restart();
                                    }
                                    else if (level >= MAX_LEVEL && killCount >= killsNeeded)
                                    {
                                        // Victory Condition
                                        // Update high score if current score is higher
                                        if (score > highScore)
                                        {
                                            highScore = score;
                                            ofstream outputFile(saveFile);
                                            if (outputFile.is_open())
                                            {
                                                outputFile << highScore << " 0 0 0"; // Clear saved game
                                                outputFile.close();
                                                hasSavedGame = false;
                                            }
                                        }
                                        else
                                        {
                                            // Clear saved game on victory
                                            ofstream outputFile(saveFile);
                                            if (outputFile.is_open())
                                            {
                                                outputFile << highScore << " 0 0 0";
                                                outputFile.close();
                                                hasSavedGame = false;
                                            }
                                        }
                                        winSound.play();
                                        currentState = STATE_VICTORY;
                                        selectedMenuItem = 0;
                                    }
                                }
                            }
                        }
                    }
                }
                bulletMoveClock.restart();
            }

            // Update Hit Effects (Remove after duration)
            float deltaTime = hitEffectClock.getElapsedTime().asSeconds();
            for (int i = 0; i < MAX_HIT_EFFECTS; i++)
            {
                if (hitEffectActive[i])
                {
                    hitEffectTimer[i] += deltaTime;
                    if (hitEffectTimer[i] >= HIT_EFFECT_DURATION)
                    {
                        hitEffectActive[i] = false; // Deactivate
                    }
                }
            }
            hitEffectClock.restart();

            // Update Invincibility Status
            if (isInvincible && invincibilityTimer.getElapsedTime().asSeconds() >= INVINCIBILITY_DURATION)
            {
                isInvincible = false;
            }

        } // End of PLAYING state

        // Level Up State Logic
        else if (currentState == STATE_LEVEL_UP)
        {
            // Blink effect for "LEVEL UP" text
            if (levelUpBlinkClock.getElapsedTime().asSeconds() >= 0.3f)
            {
                levelUpBlinkState = !levelUpBlinkState;
                levelUpBlinkClock.restart();
            }

            // Return to gameplay after 2 seconds
            if (levelUpTimer.getElapsedTime().asSeconds() >= 2.0f)
            {
                currentState = STATE_PLAYING;
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

        // Victory State Logic
        else if (currentState == STATE_VICTORY)
        {
            // Victory menu navigation
            if (menuClock.getElapsedTime() >= menuCooldown)
            {
                bool menuAction = false;

                if (Keyboard::isKeyPressed(Keyboard::Up) || Keyboard::isKeyPressed(Keyboard::W))
                {
                    selectedMenuItem = (selectedMenuItem - 1 + 2) % 2;
                    menuNavSound.play();
                    menuAction = true;
                }
                else if (Keyboard::isKeyPressed(Keyboard::Down) || Keyboard::isKeyPressed(Keyboard::S))
                {
                    selectedMenuItem = (selectedMenuItem + 1) % 2;
                    menuNavSound.play();
                    menuAction = true;
                }
                else if (Keyboard::isKeyPressed(Keyboard::Enter))
                {
                    menuClickSound.play();
                    if (selectedMenuItem == 0)
                    { // Restart Game
                        currentState = STATE_PLAYING;
                        lives = 3;
                        score = 0;
                        killCount = 0;
                        level = 1;
                        bossMoveCounter = 0;
                        isInvincible = false;
                        hasShield = false;
                        for (int r = 0; r < ROWS; r++)
                        {
                            for (int c = 0; c < COLS; c++)
                            {
                                grid[r][c] = 0;
                            }
                        }
                        spaceshipCol = COLS / 2;
                        grid[ROWS - 1][spaceshipCol] = 1;
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
                    { // Main Menu
                        if (bgMusic.getStatus() != Music::Playing)
                        {
                            bgMusic.play();
                        }
                        currentState = STATE_MENU;
                        selectedMenuItem = 0;
                    }
                    menuAction = true;
                }

                if (menuAction)
                {
                    menuClock.restart();
                }
            }
        }

        // Pause State Logic
        else if (currentState == STATE_PAUSED)
        {
            // Pause menu navigation
            if (menuClock.getElapsedTime() >= menuCooldown)
            {
                bool menuAction = false;

                if (Keyboard::isKeyPressed(Keyboard::Up) || Keyboard::isKeyPressed(Keyboard::W))
                {
                    selectedMenuItem = (selectedMenuItem - 1 + 3) % 3;
                    menuNavSound.play();
                    menuAction = true;
                }
                else if (Keyboard::isKeyPressed(Keyboard::Down) || Keyboard::isKeyPressed(Keyboard::S))
                {
                    selectedMenuItem = (selectedMenuItem + 1) % 3;
                    menuNavSound.play();
                    menuAction = true;
                }
                else if (Keyboard::isKeyPressed(Keyboard::Enter))
                {
                    menuClickSound.play();
                    if (selectedMenuItem == 0)
                    { // Resume Game
                        currentState = STATE_PLAYING;
                    }
                    else if (selectedMenuItem == 1)
                    { // Restart Level (Keep level/lives, reset score and kills)
                        currentState = STATE_PLAYING;
                        score = 0;
                        killCount = 0;
                        bossMoveCounter = 0;
                        isInvincible = false;
                        // Clear grid but keep spaceship
                        for (int r = 0; r < ROWS; r++)
                        {
                            for (int c = 0; c < COLS; c++)
                            {
                                grid[r][c] = 0;
                            }
                        }
                        spaceshipCol = COLS / 2;
                        grid[ROWS - 1][spaceshipCol] = 1;
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
                    else if (selectedMenuItem == 2)
                    { // Save and Quit to Main Menu
                        // Save current game state (no killCount)
                        ofstream outputFile(saveFile);
                        if (outputFile.is_open())
                        {
                            outputFile << highScore << " " << lives << " " << score << " " << level;
                            outputFile.close();
                            hasSavedGame = true;
                            savedLives = lives;
                            savedScore = score;
                            savedLevel = level;
                        }
                        if (bgMusic.getStatus() != Music::Playing)
                        {
                            bgMusic.play();
                        }
                        currentState = STATE_MENU;
                        selectedMenuItem = 0;
                    }
                    menuAction = true;
                }
                else if (Keyboard::isKeyPressed(Keyboard::P))
                { // P to resume
                    currentState = STATE_PLAYING;
                    menuAction = true;
                }

                if (menuAction)
                {
                    menuClock.restart();
                }
            }
        }

        // Rendering Section: Draw everything to the window
        window.clear(Color(40, 40, 40)); // Clear with dark gray background

        if (currentState == STATE_MENU)
        {
            // Draw Main Menu
            window.draw(menuBackground);
            window.draw(menuTitle);

            // Update and draw high score
            char menuHighScoreBuffer[50];
            sprintf(menuHighScoreBuffer, "High Score: %d", highScore);
            menuHighScoreText.setString(menuHighScoreBuffer);
            menuHighScoreText.setPosition(windowWidth / 2 - menuHighScoreText.getLocalBounds().width / 2.0f, 180);
            window.draw(menuHighScoreText);

            for (int i = 0; i < 4; i++)
            {
                if (i == selectedMenuItem)
                {
                    menuItems[i].setFillColor(Color::Yellow);
                }
                else
                {
                    menuItems[i].setFillColor(Color::White);
                }
                window.draw(menuItems[i]);
            }
            window.draw(menuInstructions);
        }
        else if (currentState == STATE_INSTRUCTIONS)
        {
            // Draw Instructions Screen
            window.draw(menuBackground);
            window.draw(instructionsTitle);

            // Draw controls
            window.draw(controlsTitle);
            window.draw(moveText);
            window.draw(shootText);
            window.draw(pauseText);

            // Draw entities
            window.draw(entitiesTitle);

            // Player sprite
            spaceship.setPosition(60, 285);
            window.draw(spaceship);
            window.draw(playerDesc);

            // Meteor sprite
            meteor.setPosition(60, 325);
            window.draw(meteor);
            window.draw(meteorDesc);

            // Enemy sprite
            enemy.setPosition(60, 365);
            window.draw(enemy);
            window.draw(enemyDesc);

            // Boss sprite
            bossEnemy.setPosition(60, 405);
            window.draw(bossEnemy);
            window.draw(bossDesc);

            // Player bullet sprite
            bullet.setPosition(60 + BULLET_OFFSET_X, 445);
            window.draw(bullet);
            window.draw(bulletDesc);

            // Boss bullet sprite
            bossBullet.setPosition(60 + BULLET_OFFSET_X, 485);
            window.draw(bossBullet);
            window.draw(bossBulletDesc);

            // Life icon
            lifeIcon.setPosition(60 + 8, 525);
            window.draw(lifeIcon);
            window.draw(lifeDesc);

            // Shield powerup sprite
            shieldPowerUp.setPosition(60, 565);
            window.draw(shieldPowerUp);
            window.draw(shieldPowerupDesc);

            // Draw objectives
            window.draw(objectiveTitle);
            window.draw(objective1);
            window.draw(objective2);
            window.draw(objective3);
            window.draw(objective4);

            // Draw back instruction
            window.draw(instructionsBack);
        }
        else if (currentState == STATE_PLAYING)
        {
            // Draw Gameplay Screen
            window.draw(background);
            window.draw(gameBox);

            // Draw Grid Entities (Meteors, Bullets, Spaceship, Enemies)
            for (int r = 0; r < ROWS; r++)
            {
                for (int c = 0; c < COLS; c++)
                {
                    if (grid[r][c] == 1)
                    { // Spaceship
                        spaceship.setPosition(MARGIN + c * CELL_SIZE, MARGIN + r * CELL_SIZE);
                        // Blink effect during invincibility
                        if (!isInvincible || ((int)(invincibilityTimer.getElapsedTime().asMilliseconds() / 100) % 2 == 0))
                        {
                            window.draw(spaceship);
                        }
                    }
                    else if (grid[r][c] == 2)
                    { // Meteor
                        meteor.setPosition(MARGIN + c * CELL_SIZE, MARGIN + r * CELL_SIZE);
                        window.draw(meteor);
                    }
                    else if (grid[r][c] == 3)
                    { // Player Bullet
                        bullet.setPosition(MARGIN + c * CELL_SIZE + BULLET_OFFSET_X, MARGIN + r * CELL_SIZE);
                        window.draw(bullet);
                    }
                    else if (grid[r][c] == 4)
                    { // Enemy
                        enemy.setPosition(MARGIN + c * CELL_SIZE, MARGIN + r * CELL_SIZE);
                        window.draw(enemy);
                    }
                    else if (grid[r][c] == 5)
                    { // Boss
                        bossEnemy.setPosition(MARGIN + c * CELL_SIZE, MARGIN + r * CELL_SIZE);
                        window.draw(bossEnemy);
                    }
                    else if (grid[r][c] == 6)
                    { // Boss Bullet
                        bossBullet.setPosition(MARGIN + c * CELL_SIZE + BULLET_OFFSET_X, MARGIN + r * CELL_SIZE);
                        window.draw(bossBullet);
                    }
                }
            }

            // Draw shield powerups from separate array
            for (int i = 0; i < MAX_SHIELD_POWERUPS; i++)
            {
                if (shieldPowerupActive[i])
                {
                    shieldPowerUp.setPosition(MARGIN + shieldPowerupCol[i] * CELL_SIZE, MARGIN + shieldPowerupRow[i] * CELL_SIZE);
                    window.draw(shieldPowerUp);
                }
            }

            // Draw Shield over spaceship if active
            if (hasShield)
            {
                // Center shield over spaceship position
                shieldIcon.setPosition(MARGIN + spaceshipCol * CELL_SIZE + SHIELD_OFFSET, MARGIN + (ROWS - 1) * CELL_SIZE + SHIELD_OFFSET);
                window.draw(shieldIcon);
            }

            // Draw Active Hit Effects
            for (int i = 0; i < MAX_HIT_EFFECTS; i++)
            {
                if (hitEffectActive[i])
                {
                    bulletHit.setPosition(MARGIN + hitEffectCol[i] * CELL_SIZE, MARGIN + hitEffectRow[i] * CELL_SIZE);
                    window.draw(bulletHit);
                }
            }

            // Draw UI: Lives
            livesText.setString("Lives:");

            // Draw life icons
            float lifeIconStartX = livesText.getPosition().x + livesText.getLocalBounds().width + 10;
            float lifeIconY = livesText.getPosition().y + (livesText.getLocalBounds().height / 2.0f) - 12;

            for (int i = 0; i < lives; i++)
            {
                lifeIcon.setPosition(lifeIconStartX + (i * 28), lifeIconY);
                window.draw(lifeIcon);
            }

            // Draw UI: Score
            char scoreBuffer[20];
            sprintf(scoreBuffer, "Score: %d", score);
            scoreText.setString(scoreBuffer);

            // Draw UI: Kills
            char killsBuffer[50];
            sprintf(killsBuffer, "Kills: %d/%d", killCount, level * 10);
            killsText.setString(killsBuffer);

            // Draw UI: Level
            char levelBuffer[20];
            sprintf(levelBuffer, "Level: %d", level);
            levelText.setString(levelBuffer);

            // Draw UI: High Score
            char highScoreBuffer[50];
            sprintf(highScoreBuffer, "High Score: %d", highScore);
            highScoreText.setString(highScoreBuffer);

            // Render UI elements
            window.draw(title);
            window.draw(livesText);
            window.draw(scoreText);
            window.draw(killsText);
            window.draw(levelText);
            window.draw(highScoreText);
        }
        else if (currentState == STATE_LEVEL_UP)
        {
            // Draw Level Up Screen
            window.draw(background);
            window.draw(gameBox);

            // Draw spaceship
            spaceship.setPosition(MARGIN + spaceshipCol * CELL_SIZE, MARGIN + (ROWS - 1) * CELL_SIZE);
            window.draw(spaceship);

            // Draw blinking text
            if (levelUpBlinkState)
            {
                window.draw(levelUpText);
            }

            // Show updated level info
            char levelBuffer[20];
            sprintf(levelBuffer, "Level: %d", level);
            levelText.setString(levelBuffer);

            char killsBuffer[50];
            sprintf(killsBuffer, "Kills: %d/%d", killCount, level * 10);
            killsText.setString(killsBuffer);

            window.draw(title);
            window.draw(livesText);
            window.draw(scoreText);
            window.draw(killsText);
            window.draw(levelText);
        }
        else if (currentState == STATE_PAUSED)
        {
            // Draw Pause Screen (Overlay on top of game)
            window.draw(background);
            window.draw(gameBox);

            // Draw game state in background
            for (int r = 0; r < ROWS; r++)
            {
                for (int c = 0; c < COLS; c++)
                {
                    if (grid[r][c] == 1)
                    { // Spaceship
                        spaceship.setPosition(MARGIN + c * CELL_SIZE, MARGIN + r * CELL_SIZE);
                        window.draw(spaceship);
                    }
                    else if (grid[r][c] == 2)
                    { // Meteor
                        meteor.setPosition(MARGIN + c * CELL_SIZE, MARGIN + r * CELL_SIZE);
                        window.draw(meteor);
                    }
                    else if (grid[r][c] == 3)
                    { // Bullet
                        bullet.setPosition(MARGIN + c * CELL_SIZE + BULLET_OFFSET_X, MARGIN + r * CELL_SIZE);
                        window.draw(bullet);
                    }
                    else if (grid[r][c] == 4)
                    { // Enemy
                        enemy.setPosition(MARGIN + c * CELL_SIZE, MARGIN + r * CELL_SIZE);
                        window.draw(enemy);
                    }
                    else if (grid[r][c] == 5)
                    { // Boss
                        bossEnemy.setPosition(MARGIN + c * CELL_SIZE, MARGIN + r * CELL_SIZE);
                        window.draw(bossEnemy);
                    }
                    else if (grid[r][c] == 6)
                    { // Boss Bullet
                        bossBullet.setPosition(MARGIN + c * CELL_SIZE + BULLET_OFFSET_X, MARGIN + r * CELL_SIZE);
                        window.draw(bossBullet);
                    }
                }
            }

            // Draw semi-transparent overlay
            RectangleShape overlay(Vector2f(COLS * CELL_SIZE, ROWS * CELL_SIZE));
            overlay.setPosition(MARGIN, MARGIN);
            overlay.setFillColor(Color(0, 0, 0, 150)); // Semi-transparent black
            window.draw(overlay);

            // Draw pause menu
            window.draw(pauseTitle);
            for (int i = 0; i < 3; i++)
            {
                if (i == selectedMenuItem)
                {
                    pauseItems[i].setFillColor(Color::Yellow);
                }
                else
                {
                    pauseItems[i].setFillColor(Color::White);
                }
                window.draw(pauseItems[i]);
            }
        }
        else if (currentState == STATE_VICTORY)
        {
            // Draw Victory Screen
            window.draw(menuBackground);
            window.draw(victoryTitle);

            // Update and draw score
            char victoryScoreBuffer[50];
            sprintf(victoryScoreBuffer, "Final Score: %d", score);
            victoryScore.setString(victoryScoreBuffer);
            victoryScore.setPosition(windowWidth / 2 - victoryScore.getLocalBounds().width / 2.0f, 200);
            window.draw(victoryScore);

            // Draw menu items
            for (int i = 0; i < 2; i++)
            {
                if (i == selectedMenuItem)
                {
                    victoryItems[i].setFillColor(Color::Yellow);
                }
                else
                {
                    victoryItems[i].setFillColor(Color::White);
                }
                window.draw(victoryItems[i]);
            }

            window.draw(victoryInstructions);
        }
        else if (currentState == STATE_GAME_OVER)
        {
            // Draw Game Over Screen
            window.draw(menuBackground);
            window.draw(gameOverTitle);

            // Update and draw score
            char gameOverScoreBuffer[50];
            sprintf(gameOverScoreBuffer, "Final Score: %d", score);
            gameOverScore.setString(gameOverScoreBuffer);
            gameOverScore.setPosition(windowWidth / 2 - gameOverScore.getLocalBounds().width / 2.0f, 200);
            window.draw(gameOverScore);

            // Draw menu items
            for (int i = 0; i < 2; i++)
            {
                if (i == selectedMenuItem)
                {
                    gameOverItems[i].setFillColor(Color::Yellow);
                }
                else
                {
                    gameOverItems[i].setFillColor(Color::White);
                }
                window.draw(gameOverItems[i]);
            }

            window.draw(gameOverInstructions);
        }

        window.display();
    }

    return 0;
}
