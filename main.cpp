// SFML libraries
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
// C++ libraries
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <ctime>
// namespaces
using namespace std;
using namespace sf;
// Grid Setup
const int ROWS = 23;
const int COLS = 15;
const int CELL_SIZE = 40;
const int MARGIN = 40;                                               // Margin around the grid
const float BULLET_OFFSET_X = (CELL_SIZE - CELL_SIZE * 0.3f) / 2.0f; // Center bullets horizontally
const float SHIELD_OFFSET = CELL_SIZE * -0.15f;                      // Center shield overlay
// Game States
const int STATE_MENU = 0;
const int STATE_PLAYING = 1;
const int STATE_INSTRUCTIONS = 2;
const int STATE_GAME_OVER = 3;
const int STATE_LEVEL_UP = 4;
const int STATE_VICTORY = 5;
const int STATE_PAUSED = 6;
// Helper functions:
void saveHighScoreAndGameOver(int& score, int& highScore, char saveFile[], bool& hasSavedGame, int& currentState, int& selectedMenuItem, Sound& loseSound)
{
    if (score > highScore)
    {
        highScore = score;
    }
    
    ofstream outputFile(saveFile);
    if (outputFile.is_open())
    {
        outputFile << highScore << " 0 0 0";
        outputFile.close();
        hasSavedGame = false;
    }
    
    loseSound.play();
    currentState = STATE_GAME_OVER;
    selectedMenuItem = 0;
}
void saveHighScoreAndVictory(int& score, int& highScore, char saveFile[], bool& hasSavedGame, int& currentState, int& selectedMenuItem, Sound& winSound)
{
    if (score > highScore)
    {
        highScore = score;
    }
    
    ofstream outputFile(saveFile);
    if (outputFile.is_open())
    {
        outputFile << highScore << " 0 0 0";
        outputFile.close();
        hasSavedGame = false;
    }
    
    winSound.play();
    currentState = STATE_VICTORY;
    selectedMenuItem = 0;
}
void createExplosionEffect(int row, int col, int hitEffectRow[], int hitEffectCol[], float hitEffectTimer[], bool hitEffectActive[], int maxEffects)
{
    for (int i = 0; i < maxEffects; i++)
    {
        if (!hitEffectActive[i])
        {
            hitEffectRow[i] = row;
            hitEffectCol[i] = col;
            hitEffectTimer[i] = 0.0f;
            hitEffectActive[i] = true;
            break;
        }
    }
}
void clearGrid(int grid[][15])
{
    for (int r = 0; r < ROWS; r++)
    {
        for (int c = 0; c < COLS; c++)
        {
            grid[r][c] = 0;
        }
    }
}
void clearEntities(int grid[][15])
{
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
}
void restartAllClocks(Clock& meteorSpawn, Clock& meteorMove, Clock& enemySpawn, Clock& enemyMove, Clock& bossSpawn, Clock& bossMove, Clock& bossBulletMove, Clock& bulletMove, Clock& shieldSpawn, Clock& shieldMove)
{
    meteorSpawn.restart();
    meteorMove.restart();
    enemySpawn.restart();
    enemyMove.restart();
    bossSpawn.restart();
    bossMove.restart();
    bossBulletMove.restart();
    bulletMove.restart();
    shieldSpawn.restart();
    shieldMove.restart();
}
void resetSpaceship(int grid[][15], int& spaceshipCol)
{
    grid[ROWS - 1][spaceshipCol] = 0;
    spaceshipCol = COLS / 2;
    grid[ROWS - 1][spaceshipCol] = 1;
}
void setMenuColors(Text items[], int count, int selectedIndex)
{
    for (int i = 0; i < count; i++)
    {
        items[i].setFillColor(i == selectedIndex ? Color::Yellow : Color::White);
    }
}
bool loadTexture(Texture& texture, const char path[])
{
    if (!texture.loadFromFile(path))
    {
        cerr << "Failed to load " << path << endl;
        return false;
    }
    return true;
}
void setupSprite(Sprite& sprite, Texture& texture, float scaleX = 1.0f, float scaleY = 1.0f)
{
    sprite.setTexture(texture);
    sprite.setScale(
        (CELL_SIZE * scaleX) / texture.getSize().x,
        (CELL_SIZE * scaleY) / texture.getSize().y);
}
// Main Function
int main()
{
    srand(static_cast<unsigned int>(time(0))); // Random Number Generator Setup
    // Window Setup
    const int windowWidth = COLS * CELL_SIZE + MARGIN * 2 + 500;
    const int windowHeight = ROWS * CELL_SIZE + MARGIN * 2;
    RenderWindow window(VideoMode(windowWidth, windowHeight), "Space Shooter");
    window.setFramerateLimit(60);
    // Save File Handling
    int highScore = 0;
    int savedLives = 0;
    int savedScore = 0;
    int savedLevel = 0;
    bool hasSavedGame = false;
    char saveFile[] = "save-file.txt";
    ifstream inputFile(saveFile);
    if (inputFile.is_open()) // Check if file exists
    {
        inputFile >> highScore >> savedLives >> savedScore >> savedLevel;
        inputFile.close();
        if (savedLevel > 0 && savedLives > 0) // Check if saved game exists
        {
            hasSavedGame = true;
        }
    }
    else // Create new save file with default data
    {
        ofstream createFile(saveFile);
        if (createFile.is_open())
        {
            createFile << "0 0 0 0";
            createFile.close();
        }
    }
    // Game Variables
    int currentState = STATE_MENU;
    int selectedMenuItem = 0;
    int lives = 3;
    int score = 0;
    int killCount = 0;
    int level = 1;
    const int MAX_LEVEL = 5;
    bool isInvincible = false;
    Clock invincibilityTimer;
    const float INVINCIBILITY_DURATION = 2.0f;
    Clock levelUpTimer;
    bool levelUpBlinkState = true;
    Clock levelUpBlinkClock;
    int bossMoveCounter = 0;
    // Grid System: 0=Empty, 1=Player, 2=Meteor, 3=Bullet, 4=Enemy, 5=Boss, 6=Boss Bullet
    int grid[ROWS][COLS] = {0};
    // Shield Powerup System
    const int MAX_SHIELD_POWERUPS = 5;
    int shieldPowerupRow[MAX_SHIELD_POWERUPS] = {-1, -1, -1, -1, -1};
    int shieldPowerupCol[MAX_SHIELD_POWERUPS] = {-1, -1, -1, -1, -1};
    bool shieldPowerupActive[MAX_SHIELD_POWERUPS] = {false, false, false, false, false};
    int shieldPowerupDirection[MAX_SHIELD_POWERUPS] = {0, 0, 0, 0, 0};
    bool hasShield = false;
    // Hit Effect System
    const int MAX_HIT_EFFECTS = 50;
    int hitEffectRow[MAX_HIT_EFFECTS] = {0};
    int hitEffectCol[MAX_HIT_EFFECTS] = {0};
    float hitEffectTimer[MAX_HIT_EFFECTS] = {0.0f};
    bool hitEffectActive[MAX_HIT_EFFECTS] = {false};
    const float HIT_EFFECT_DURATION = 0.3f;
    // Spaceship Initialization: Set up player's spaceship at starting position
    int spaceshipCol = COLS / 2;
    grid[ROWS - 1][spaceshipCol] = 1;
    // Textures and Sprites Setup
    Texture spaceshipTexture;
    if (!loadTexture(spaceshipTexture, "assets/images/player.png")) return -1;
    Sprite spaceship;
    setupSprite(spaceship, spaceshipTexture);
    Texture lifeTexture;
    if (!loadTexture(lifeTexture, "assets/images/life.png")) return -1;
    Sprite lifeIcon;
    lifeIcon.setTexture(lifeTexture);
    lifeIcon.setScale(24.0f / lifeTexture.getSize().x, 24.0f / lifeTexture.getSize().y);
    Texture shieldTexture, shieldPowerUpTexture;
    if (!loadTexture(shieldTexture, "assets/images/shield.png")) return -1;
    if (!loadTexture(shieldPowerUpTexture, "assets/images/shield-powerup.png")) return -1;
    Sprite shieldIcon, shieldPowerUp;
    setupSprite(shieldPowerUp, shieldPowerUpTexture);
    setupSprite(shieldIcon, shieldTexture, 1.3f, 1.3f);
    Texture bgTexture;
    if (!loadTexture(bgTexture, "assets/images/backgroundColor.png")) return -1;
    Sprite background;
    background.setTexture(bgTexture);
    background.setScale(
        static_cast<float>(COLS * CELL_SIZE) / bgTexture.getSize().x,
        static_cast<float>(ROWS * CELL_SIZE) / bgTexture.getSize().y);
    background.setPosition(MARGIN, MARGIN);
    RectangleShape gameBox(Vector2f(COLS * CELL_SIZE, ROWS * CELL_SIZE));
    gameBox.setFillColor(Color::Transparent);
    gameBox.setOutlineThickness(5);
    gameBox.setOutlineColor(Color::Black);
    gameBox.setPosition(MARGIN, MARGIN);
    Texture meteorTexture;
    if (!loadTexture(meteorTexture, "assets/images/meteorSmall.png")) return -1;
    Sprite meteor;
    setupSprite(meteor, meteorTexture);
    Texture enemyTexture, bossEnemyTexture;
    if (!loadTexture(enemyTexture, "assets/images/enemyUFO.png")) return -1;
    if (!loadTexture(bossEnemyTexture, "assets/images/enemyShip.png")) return -1;
    Sprite enemy, bossEnemy;
    setupSprite(enemy, enemyTexture);
    setupSprite(bossEnemy, bossEnemyTexture);
    Texture bulletTexture, bulletHitTexture, bossBulletTexture, bossBulletHitTexture;
    if (!loadTexture(bulletTexture, "assets/images/laserRed.png")) return -1;
    if (!loadTexture(bulletHitTexture, "assets/images/laserRedShot.png")) return -1;
    if (!loadTexture(bossBulletTexture, "assets/images/laserGreen.png")) return -1;
    if (!loadTexture(bossBulletHitTexture, "assets/images/laserGreenShot.png")) return -1;
    Sprite bullet, bulletHit, bossBullet, bossBulletHit;
    setupSprite(bullet, bulletTexture, 0.3f, 0.8f);
    setupSprite(bulletHit, bulletHitTexture);
    setupSprite(bossBullet, bossBulletTexture, 0.3f, 0.8f);
    setupSprite(bossBulletHit, bossBulletHitTexture);
    Texture menuBgTexture;
    if (!loadTexture(menuBgTexture, "assets/images/starBackground.png")) return -1;
    Sprite menuBackground;
    menuBackground.setTexture(menuBgTexture);
    menuBackground.setScale(
        static_cast<float>(windowWidth) / menuBgTexture.getSize().x,
        static_cast<float>(windowHeight) / menuBgTexture.getSize().y);
    menuBackground.setPosition(0, 0);
    // Font Setup for text
    Font font;
    if (!font.loadFromFile("assets/fonts/font.ttf"))
    {
        cerr << "Failed to load font" << endl;
        return -1;
    }
    // Music and Sound Effects Setup
    Music bgMusic;
    if (!bgMusic.openFromFile("assets/sounds/bg-music.mp3"))
    {
        cerr << "Failed to load background music" << endl;
        return -1;
    }
    bgMusic.setLoop(true);  // Music never ends
    bgMusic.setVolume(30);  // low volume
    bgMusic.play();         // start playing as game starts
    SoundBuffer shootBuffer, explosionBuffer, damageBuffer, levelUpBuffer;
    SoundBuffer menuClickBuffer, menuNavBuffer, winBuffer, loseBuffer;
    if (!shootBuffer.loadFromFile("assets/sounds/shoot.wav") ||
        !explosionBuffer.loadFromFile("assets/sounds/explosion.wav") ||
        !damageBuffer.loadFromFile("assets/sounds/damage.mp3") ||
        !levelUpBuffer.loadFromFile("assets/sounds/level-up.mp3") ||
        !menuClickBuffer.loadFromFile("assets/sounds/menu-click.mp3") ||
        !menuNavBuffer.loadFromFile("assets/sounds/menu-navigate.wav") ||
        !winBuffer.loadFromFile("assets/sounds/win.wav") ||
        !loseBuffer.loadFromFile("assets/sounds/lose.wav"))
    {
        cerr << "Failed to load sound files" << endl;
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
    // Text Setup throughout the game
    Text menuTitle("SPACE SHOOTER", font, 40);
    menuTitle.setFillColor(Color::Yellow); 
    menuTitle.setPosition(windowWidth / 2 - menuTitle.getLocalBounds().width / 2.0f, 100); // Center the title relative to the screen
    Text menuItems[4];  // 4 menu options
    const char menuTexts[4][20] = {"Start Game", "Load Saved Game", "Instructions", "Exit"};
    for (int i = 0; i < 4; i++)
    {
        menuItems[i].setFont(font);
        menuItems[i].setString(menuTexts[i]);
        menuItems[i].setCharacterSize(28);
        menuItems[i].setFillColor(Color::White);
        menuItems[i].setPosition(windowWidth / 2 - menuItems[i].getLocalBounds().width / 2.0f, 260 + i * 56); // Centered
    }
    Text menuHighScoreText("High Score: 0", font, 24);  // Placeholder text (0)
    menuHighScoreText.setFillColor(Color::Yellow);
    menuHighScoreText.setPosition(windowWidth / 2 - menuHighScoreText.getLocalBounds().width / 2.0f, 180); // Centered
    Text menuInstructions("Use UP/DOWN or W/S to navigate  |  ENTER to select", font, 18);
    menuInstructions.setFillColor(Color(150, 150, 150));
    menuInstructions.setPosition(windowWidth / 2 - menuInstructions.getLocalBounds().width / 2.0f, windowHeight - 80); // Centered
    // Playing state text
    Text title("Space  Shooter", font, 28);
    title.setFillColor(Color::Yellow);
    title.setPosition(MARGIN + COLS * CELL_SIZE + 20, MARGIN);
    Text livesText("Lives:", font, 20);
    livesText.setFillColor(Color::White);
    livesText.setPosition(MARGIN + COLS * CELL_SIZE + 20, MARGIN + 150);
    Text scoreText("Score: 0", font, 20);
    scoreText.setFillColor(Color::White);
    scoreText.setPosition(MARGIN + COLS * CELL_SIZE + 20, MARGIN + 200);
    Text killsText("Kills: 0/10", font, 20);
    killsText.setFillColor(Color::White);
    killsText.setPosition(MARGIN + COLS * CELL_SIZE + 20, MARGIN + 230);
    Text levelText("Level: 1", font, 20);
    levelText.setFillColor(Color::White);
    levelText.setPosition(MARGIN + COLS * CELL_SIZE + 20, MARGIN + 280);
    Text highScoreText("High Score: 0", font, 20);
    highScoreText.setFillColor(Color::Yellow);
    highScoreText.setPosition(MARGIN + COLS * CELL_SIZE + 20, MARGIN + 330);
    // Game Over Screen
    Text gameOverTitle("GAME OVER", font, 40);
    gameOverTitle.setFillColor(Color::Red);
    gameOverTitle.setPosition(windowWidth / 2 - gameOverTitle.getLocalBounds().width / 2.0f, 100);
    Text gameOverScore("Final Score: 0", font, 28);
    gameOverScore.setFillColor(Color::Yellow);
    gameOverScore.setPosition(windowWidth / 2 - gameOverScore.getLocalBounds().width / 2.0f, 200);
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
    Text gameOverInstructions("Use UP/DOWN or W/S to navigate  |  ENTER to select", font, 18);
    gameOverInstructions.setFillColor(Color(150, 150, 150));
    gameOverInstructions.setPosition(windowWidth / 2 - gameOverInstructions.getLocalBounds().width / 2.0f, windowHeight - 80);
    // Level Up Screen
    Text levelUpText("LEVEL UP!", font, 40);
    levelUpText.setFillColor(Color::Green);
    float gridCenterX = MARGIN + (COLS * CELL_SIZE) / 2.0f;
    float gridCenterY = MARGIN + (ROWS * CELL_SIZE) / 2.0f;
    levelUpText.setPosition(gridCenterX - levelUpText.getLocalBounds().width / 2.0f, gridCenterY - levelUpText.getLocalBounds().height / 2.0f - 10);
    // Pause Screen
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
    // Victory Screen
    Text victoryTitle("VICTORY!", font, 40);
    victoryTitle.setFillColor(Color::Yellow);
    victoryTitle.setPosition(windowWidth / 2 - victoryTitle.getLocalBounds().width / 2.0f, 100);
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
    // Instructions Screen
    Text instructionsTitle("HOW TO PLAY", font, 40);
    instructionsTitle.setFillColor(Color::Yellow);
    instructionsTitle.setPosition(windowWidth / 2 - instructionsTitle.getLocalBounds().width / 2.0f, 40);
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
    Text entitiesTitle("ENTITIES", font, 24);
    entitiesTitle.setFillColor(Color::Cyan);
    entitiesTitle.setPosition(50, 250);
    Text playerDesc("Your Ship", font, 18);
    playerDesc.setFillColor(Color::White);
    playerDesc.setPosition(120, 290);
    Text meteorDesc("Meteor - 1 Point (Avoid collision!)", font, 18);
    meteorDesc.setFillColor(Color::White);
    meteorDesc.setPosition(120, 330);
    Text enemyDesc("Enemy - 3 Points (Avoid collision!)", font, 18);
    enemyDesc.setFillColor(Color::White);
    enemyDesc.setPosition(120, 370);
    Text bossDesc("Boss - 5 Points (Level 3+) (Avoid collision!)", font, 18);
    bossDesc.setFillColor(Color::White);
    bossDesc.setPosition(120, 410);
    Text bulletDesc("Your Bullet", font, 18);
    bulletDesc.setFillColor(Color::White);
    bulletDesc.setPosition(120, 450);
    Text bossBulletDesc("Boss Bullet - Avoid!", font, 18);
    bossBulletDesc.setFillColor(Color::White);
    bossBulletDesc.setPosition(120, 490);
    Text lifeDesc("Life Icon - Indicates remaining lives", font, 18);
    lifeDesc.setFillColor(Color::White);
    lifeDesc.setPosition(120, 530);
    Text shieldPowerupDesc("Shield Powerup - Absorbs 1 Hit (Level 3+)", font, 18);
    shieldPowerupDesc.setFillColor(Color::White);
    shieldPowerupDesc.setPosition(120, 570);
    Text systemsTitle("GAME SYSTEMS", font, 24);
    systemsTitle.setFillColor(Color::Cyan);
    systemsTitle.setPosition(50, 620);
    Text livesDesc("Lives: You start with 3 lives. Lose one when hit any enemy.", font, 18);
    livesDesc.setFillColor(Color::White);
    livesDesc.setPosition(50, 660);
    Text levelsDesc("Levels: Destroy 10 enemies/bosses per level to advance.", font, 18);
    levelsDesc.setFillColor(Color::White);
    levelsDesc.setPosition(50, 690);
    Text highScoreDesc("High Score: Your best score is saved automatically.", font, 18);
    highScoreDesc.setFillColor(Color::White);
    highScoreDesc.setPosition(50, 720);
    Text objectiveTitle("OBJECTIVE", font, 24);
    objectiveTitle.setFillColor(Color::Cyan);
    objectiveTitle.setPosition(50, 770);
    Text objective1("- Destroy enemies and bosses", font, 18);
    objective1.setFillColor(Color::White);
    objective1.setPosition(50, 810);
    Text objective2("- Do not lose all your lives", font, 18);
    objective2.setFillColor(Color::White);
    objective2.setPosition(50, 840);
    Text objective3("- Complete Level 5 to win!", font, 18);
    objective3.setFillColor(Color::White);
    objective3.setPosition(50, 870);

    Text instructionsBack("Press ESC or BACKSPACE to return to menu", font, 18);
    instructionsBack.setFillColor(Color(150, 150, 150));
    instructionsBack.setPosition(windowWidth / 2 - instructionsBack.getLocalBounds().width / 2.0f, windowHeight - 80);
    // All the clocks and cooldowns controlling the time of events in the game
    // Movement Delay to avoid fast movement when key is held
    Clock moveClock;
    Time moveCooldown = milliseconds(100);
    // After what time will a new meteor spawn
    Clock meteorSpawnClock;
    Clock meteorMoveClock;
    float nextSpawnTime = 1.0f + (rand() % 3);
    // After what time will a new enemy spawn
    Clock enemySpawnClock;
    Clock enemyMoveClock;
    float nextEnemySpawnTime = 2.0f + (rand() % 4);
    // After what time will a new boss spawn (level 3+)
    Clock bossSpawnClock;
    Clock bossMoveClock;
    Clock bossBulletMoveClock;
    float nextBossSpawnTime = 8.0f + (rand() % 5);
    // After what time will a new shield powerup spawn (level 3+)
    Clock shieldPowerupSpawnClock;
    Clock shieldPowerupMoveClock;
    float nextShieldPowerupSpawnTime = 15.0f + (rand() % 10);
    // Bullet firing delay and bullet speed
    Clock bulletMoveClock;
    Clock bulletFireClock;
    Time bulletFireCooldown = milliseconds(300);
    // How long to show explosion effects
    Clock hitEffectClock;
    // same delay as movement for menu navigation to avoid fast input
    Clock menuClock;
    Time menuCooldown = milliseconds(200);
    // The Game Statrs from here
    while (window.isOpen())
    {
        // Check if the user closes the window or not
        Event event;
        while (window.pollEvent(event))
        {
            if (event.type == Event::Closed)
                window.close();
        }
        // C++ Logic for each Game Screen
        // Menu Screen
        if (currentState == STATE_MENU)
        {
            if (menuClock.getElapsedTime() >= menuCooldown) // Condition for menu cooldown
            {
                bool menuAction = false;
                if (Keyboard::isKeyPressed(Keyboard::Up) || Keyboard::isKeyPressed(Keyboard::W))
                {
                    selectedMenuItem = (selectedMenuItem - 1 + 4) % 4; // (+4 so that selected never becomes negative)
                    menuNavSound.play();
                    menuAction = true;
                }
                else if (Keyboard::isKeyPressed(Keyboard::Down) || Keyboard::isKeyPressed(Keyboard::S))
                {
                    selectedMenuItem = (selectedMenuItem + 1) % 4;
                    menuNavSound.play();
                    menuAction = true;
                }
                else if (Keyboard::isKeyPressed(Keyboard::Enter))
                {
                    menuClickSound.play();
                    if (selectedMenuItem == 0) // (Start New Game)
                    {
                        bgMusic.stop();
                        currentState = STATE_PLAYING;
                        // Game Will start fresh
                        lives = 3;
                        score = 0;
                        killCount = 0;
                        level = 1;
                        bossMoveCounter = 0;
                        hasShield = false;
                        clearGrid(grid);
                        for (int i = 0; i < MAX_SHIELD_POWERUPS; i++)
                        {
                            shieldPowerupActive[i] = false;
                        }
                        resetSpaceship(grid, spaceshipCol);
                        restartAllClocks(meteorSpawnClock, meteorMoveClock, enemySpawnClock, enemyMoveClock,
                                        bossSpawnClock, bossMoveClock, bossBulletMoveClock, bulletMoveClock,
                                        shieldPowerupSpawnClock, shieldPowerupMoveClock);
                    }
                    else if (selectedMenuItem == 1) // (Load Saved Game)
                    {
                        if (hasSavedGame)  // Will only work if there is a saved game
                        {
                            bgMusic.stop();
                            currentState = STATE_PLAYING;
                            // Game will start with saved lives, score, and level
                            lives = savedLives;
                            score = savedScore;
                            killCount = 0;
                            level = savedLevel;
                            bossMoveCounter = 0;
                            hasShield = false;
                            clearGrid(grid);
                            for (int i = 0; i < MAX_SHIELD_POWERUPS; i++)
                            {
                                shieldPowerupActive[i] = false;
                            }
                            resetSpaceship(grid, spaceshipCol);
                            restartAllClocks(meteorSpawnClock, meteorMoveClock, enemySpawnClock, enemyMoveClock,
                                            bossSpawnClock, bossMoveClock, bossBulletMoveClock, bulletMoveClock,
                                            shieldPowerupSpawnClock, shieldPowerupMoveClock);
                        }
                        else
                        {
                            cout << "No Saved Game Exists!" << endl;
                        }
                    }
                    else if (selectedMenuItem == 2) // (Instructions)
                    {
                        currentState = STATE_INSTRUCTIONS;
                    }
                    else if (selectedMenuItem == 3) // (Exit Game)
                    {
                        bgMusic.stop();
                        window.close();
                    }
                    menuAction = true; // trigger the cooldown
                }
                if (menuAction)
                {
                    menuClock.restart(); // Restart cooldown
                }
            }

            setMenuColors(menuItems, 4, selectedMenuItem);
        }
        // Game Over Screen
        else if (currentState == STATE_GAME_OVER)
        {
            if (menuClock.getElapsedTime() >= menuCooldown) // Same Cooldown logic for menu navigation
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
                    if (selectedMenuItem == 0) // (Restart Game)
                    {
                        currentState = STATE_PLAYING;
                        lives = 3;
                        score = 0;
                        killCount = 0;
                        level = 1;
                        bossMoveCounter = 0;
                        isInvincible = false;
                        hasShield = false;
                        clearGrid(grid);
                        for (int i = 0; i < MAX_SHIELD_POWERUPS; i++)
                        {
                            shieldPowerupActive[i] = false;
                        }
                        resetSpaceship(grid, spaceshipCol);
                        restartAllClocks(meteorSpawnClock, meteorMoveClock, enemySpawnClock, enemyMoveClock,
                                        bossSpawnClock, bossMoveClock, bossBulletMoveClock, bulletMoveClock,
                                        shieldPowerupSpawnClock, shieldPowerupMoveClock);
                    }
                    else if (selectedMenuItem == 1) // (Return to Main Menu)
                    {
                        if (bgMusic.getStatus() != Music::Playing)
                        {
                            bgMusic.play();
                        }
                        currentState = STATE_MENU;
                        selectedMenuItem = 0;
                    }
                    menuAction = true;
                }
                if (menuAction) // Same menu cooldown logic
                {
                    menuClock.restart();
                }
            }
            setMenuColors(gameOverItems, 2, selectedMenuItem);
        }
        // Instructions Screen
        else if (currentState == STATE_INSTRUCTIONS)
        {
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
        // Playing Screen
        else if (currentState == STATE_PLAYING)
        {
            if (menuClock.getElapsedTime() >= menuCooldown) // Constantly check for pause input
            {
                if (Keyboard::isKeyPressed(Keyboard::P))
                {
                    currentState = STATE_PAUSED;
                    selectedMenuItem = 0;
                    menuClock.restart();
                }
            }
            // Spaceshipe Movement left right
            if (moveClock.getElapsedTime() >= moveCooldown)
            {
                bool moved = false;
                if ((Keyboard::isKeyPressed(Keyboard::Left) || Keyboard::isKeyPressed(Keyboard::A)) && spaceshipCol > 0)
                {
                    grid[ROWS - 1][spaceshipCol] = 0; // Clear current position
                    spaceshipCol--;                   // Move left
                    grid[ROWS - 1][spaceshipCol] = 1; // Put Spaceship there
                    moved = true; // trigger cooldown
                }
                else if ((Keyboard::isKeyPressed(Keyboard::Right) || Keyboard::isKeyPressed(Keyboard::D)) && spaceshipCol < COLS - 1)
                {
                    grid[ROWS - 1][spaceshipCol] = 0; // Clear current position
                    spaceshipCol++;                    // Move right
                    grid[ROWS - 1][spaceshipCol] = 1; // Put Spaceship there
                    moved = true; // trigger cooldown
                }
                if (moved) // restart cooldown timer
                {
                    moveClock.restart();
                }
            }
            // Bullet firing
            if (Keyboard::isKeyPressed(Keyboard::Space) && bulletFireClock.getElapsedTime() >= bulletFireCooldown)
            {
                int bulletRow = ROWS - 2;  // Just above the spaceship
                if (bulletRow >= 0 && grid[bulletRow][spaceshipCol] == 0)
                {
                    grid[bulletRow][spaceshipCol] = 3;
                    shootSound.play();
                }
                bulletFireCooldown = milliseconds(300); // can shoot bullet only every 0.3 seconds
                bulletFireClock.restart();
            }
            // Metoer spawning
            if (meteorSpawnClock.getElapsedTime().asSeconds() >= nextSpawnTime)
            {
                int randomCol = rand() % COLS;  // Any random column
                if (grid[0][randomCol] == 0) // Only spawn if that area is empty
                {
                    grid[0][randomCol] = 2;
                }
                meteorSpawnClock.restart();
                nextSpawnTime = 1.0f + (rand() % 3);
            }
            // Enemy Spawining
            if (enemySpawnClock.getElapsedTime().asSeconds() >= nextEnemySpawnTime)
            {
                int randomCol = rand() % COLS;  // Any random column
                if (grid[0][randomCol] == 0) // Check empty
                {
                    grid[0][randomCol] = 4;
                }
                enemySpawnClock.restart();
                float baseTime = 2.0f - (level * 0.35f);  // Base spawn time for each level (decreases with level)
                float variance = 2.5f - (level * 0.35f);  // Random variation int he spawning
                if (baseTime < 0.5f) // should nowt be too fast
                    baseTime = 0.5f;
                if (variance < 1.0f) // should not be too fast
                    variance = 1.0f;
                nextEnemySpawnTime = baseTime + (rand() % (int)variance); // calculate time
            }
            // Boos spawning
            if (level >= 3 && bossSpawnClock.getElapsedTime().asSeconds() >= nextBossSpawnTime)
            {
                int randomCol = rand() % COLS;  // Any random column
                if (grid[0][randomCol] == 0) // Check empty
                {
                    grid[0][randomCol] = 5;
                }
                bossSpawnClock.restart();
                float bossBaseTime = 10.0f - ((level - 3) * 1.5f);  // Decreases with level
                float bossVariance = 4.0f;  // Random variation
                // same logic as enemies
                if (bossBaseTime < 5.0f)
                    bossBaseTime = 5.0f;
                nextBossSpawnTime = bossBaseTime + (rand() % (int)bossVariance);
            }
            // Shield Powerup Spawning
            if (level >= 3 && shieldPowerupSpawnClock.getElapsedTime().asSeconds() >= nextShieldPowerupSpawnTime)
            {
                for (int i = 0; i < MAX_SHIELD_POWERUPS; i++) // separate array for powerups
                {
                    if (!shieldPowerupActive[i]) // empty slot
                    {
                        int randomCol = rand() % COLS;  // Any random column
                        shieldPowerupRow[i] = 0;        // Top row
                        shieldPowerupCol[i] = randomCol;
                        shieldPowerupActive[i] = true;  // powerup now visible
                        shieldPowerupDirection[i] = 0;  // move down
                        break;  // Only 1 powerup
                    }
                }
                shieldPowerupSpawnClock.restart();
                float shieldBaseTime;
                float shieldVariance;
                if (level < 5) // 20-35 seconds for levels 3 and 4
                {
                    shieldBaseTime = 20.0f;
                    shieldVariance = 15.0f;
                }
                else // 12-20 seconds for level 5
                {
                    shieldBaseTime = 12.0f;
                    shieldVariance = 8.0f;
                }
                nextShieldPowerupSpawnTime = shieldBaseTime + (rand() % (int)shieldVariance); // calculate time
            }
            // meteor speed
            float meteorMoveSpeed = 0.7f - ((level - 1) * 0.12f); // speed formula based on level (decreases by 0.12s per level)
            if (meteorMoveSpeed < 0.333f)  // cannot go below 0.333s
                meteorMoveSpeed = 0.333f;
            if (meteorMoveClock.getElapsedTime().asSeconds() >= meteorMoveSpeed)
            {
                // Loop from bottom to top and update meteor positions
                for (int r = ROWS - 1; r >= 0; r--)
                {
                    for (int c = 0; c < COLS; c++)
                    {
                        if (grid[r][c] == 2)
                        {
                            if (r == ROWS - 1) // check if it goes below screen
                            {
                                grid[r][c] = 0; // remove it
                            }
                            else
                            {
                                grid[r][c] = 0; // Clear current position
                                if (grid[r + 1][c] == 0 || grid[r + 1][c] == 2)
                                {
                                    grid[r + 1][c] = 2;  // Place meteor in new position
                                }
                                else if (grid[r + 1][c] == 1) // collision with player
                                {
                                    if (hasShield)
                                    {
                                        hasShield = false;
                                        isInvincible = true;
                                        invincibilityTimer.restart(); // 2s invincibility
                                        damageSound.play();
                                    }
                                    else if (!isInvincible)
                                    {
                                        lives--;
                                        damageSound.play();
                                        isInvincible = true;
                                        invincibilityTimer.restart();
                                        if (lives <= 0) // game over
                                        {
                                            saveHighScoreAndGameOver(score, highScore, saveFile, hasSavedGame,
                                                                   currentState, selectedMenuItem, loseSound);
                                        }
                                    }
                                }
                                else if (grid[r + 1][c] == 3) // collision with bullet
                                {
                                    int meteorPoints = 1 + (rand() % 2); // Random 1-2 points
                                    score += meteorPoints;
                                    explosionSound.play();
                                    grid[r + 1][c] = 0;
                                    createExplosionEffect(r + 1, c, hitEffectRow, hitEffectCol, hitEffectTimer,
                                                        hitEffectActive, MAX_HIT_EFFECTS);
                                }
                            }
                        }
                    }
                }
                meteorMoveClock.restart();
            }
            // shield powerup movement
            if (shieldPowerupMoveClock.getElapsedTime().asSeconds() >= 0.5f)
            {
                for (int i = 0; i < MAX_SHIELD_POWERUPS; i++)
                {
                    if (shieldPowerupActive[i])
                    {
                        if (shieldPowerupRow[i] >= ROWS - 1) // moves below screen
                        {
                            shieldPowerupActive[i] = false;
                            continue;
                        }
                        if (grid[shieldPowerupRow[i]][shieldPowerupCol[i]] == 1) // player claimed shield
                        {
                            if (!hasShield) {
                                hasShield = true;
                                levelUpSound.play();
                            }
                            shieldPowerupActive[i] = false;
                            continue;
                        }
                        shieldPowerupRow[i]++; // move down every time
                        if (grid[shieldPowerupRow[i]][shieldPowerupCol[i]] == 1) // player claimed shield
                        {
                            if (!hasShield) {
                                hasShield = true;
                                levelUpSound.play();
                            }
                            shieldPowerupActive[i] = false;
                            continue;
                        }
                    }
                }
                shieldPowerupMoveClock.restart();  // reset timer
            }
            // enemy movement logic
            float enemyMoveSpeed = 0.7f - ((level - 1) * 0.12f);  // same speed logic as meteors
            if (enemyMoveClock.getElapsedTime().asSeconds() >= enemyMoveSpeed)
            {
                for (int r = ROWS - 1; r >= 0; r--)
                {
                    for (int c = 0; c < COLS; c++)
                    {
                        if (grid[r][c] == 4)
                        {
                            if (r == ROWS - 1) // enemy reached bottom
                            {
                                grid[r][c] = 0;
                                if (hasShield)
                                {
                                    hasShield = false;
                                    isInvincible = true;
                                    invincibilityTimer.restart();
                                    damageSound.play();
                                }
                                else if (!isInvincible)
                                {
                                    lives--;
                                    damageSound.play();
                                    isInvincible = true;
                                    invincibilityTimer.restart();
                                    if (lives <= 0)
                                    {
                                        saveHighScoreAndGameOver(score, highScore, saveFile, hasSavedGame,
                                                               currentState, selectedMenuItem, loseSound);
                                    }
                                }
                            }
                            else
                            {
                                grid[r][c] = 0;
                                if (grid[r + 1][c] == 0 || grid[r + 1][c] == 4)
                                {
                                    grid[r + 1][c] = 4;
                                }
                                else if (grid[r + 1][c] == 1) // collision with player
                                {
                                    if (hasShield)
                                    {
                                        hasShield = false;
                                        isInvincible = true;
                                        invincibilityTimer.restart();
                                        explosionSound.play();
                                    }
                                    else if (!isInvincible)
                                    {
                                        lives--;
                                        damageSound.play();
                                        isInvincible = true;
                                        invincibilityTimer.restart();
                                        if (lives <= 0)
                                        {
                                            saveHighScoreAndGameOver(score, highScore, saveFile, hasSavedGame,
                                                                   currentState, selectedMenuItem, loseSound);
                                        }
                                    }
                                }
                                else if (grid[r + 1][c] == 3) // collision with bullet
                                {
                                    score += 3;  // 3 score
                                    killCount++; // +1 kill
                                    explosionSound.play();
                                    grid[r + 1][c] = 0;
                                    createExplosionEffect(r + 1, c, hitEffectRow, hitEffectCol, hitEffectTimer,
                                                        hitEffectActive, MAX_HIT_EFFECTS);
                                    // check if level up                    
                                    int killsNeeded = level * 10;
                                    if (level < MAX_LEVEL && killCount >= killsNeeded)
                                    {
                                        level++;
                                        levelUpSound.play();
                                        killCount = 0;
                                        bossMoveCounter = 0;
                                        clearEntities(grid);
                                        resetSpaceship(grid, spaceshipCol);
                                        currentState = STATE_LEVEL_UP;
                                        levelUpTimer.restart(); // level up screen time
                                        levelUpBlinkClock.restart();
                                    }
                                    else if (level >= MAX_LEVEL && killCount >= killsNeeded)
                                    {
                                        saveHighScoreAndVictory(score, highScore, saveFile, hasSavedGame,
                                                              currentState, selectedMenuItem, winSound);
                                    }
                                }
                            }
                        }
                    }
                }
                enemyMoveClock.restart();
            }
            // boss movement logic
            float bossMoveSpeed = 0.8f - ((level - 3) * 0.1f);  // same speed logic as enemies
            if (bossMoveSpeed < 0.5f) // cannot go below 0.5s
                bossMoveSpeed = 0.5f;
            if (bossMoveClock.getElapsedTime().asSeconds() >= bossMoveSpeed)
            {
                for (int r = ROWS - 1; r >= 0; r--)
                {
                    for (int c = 0; c < COLS; c++)
                    {
                        if (grid[r][c] == 5)
                        {
                            if (r == ROWS - 1) // bottom of screen
                            {
                                grid[r][c] = 0;
                                if (hasShield)
                                {
                                    hasShield = false;
                                    isInvincible = true;
                                    invincibilityTimer.restart();
                                    damageSound.play();
                                }
                                else if (!isInvincible)
                                {
                                    lives--;
                                    damageSound.play();
                                    isInvincible = true;
                                    invincibilityTimer.restart();
                                    if (lives <= 0)
                                    {
                                        saveHighScoreAndGameOver(score, highScore, saveFile, hasSavedGame,
                                                               currentState, selectedMenuItem, loseSound);
                                    }
                                }
                            }
                            else
                            {
                                int nextRow = r + 1;
                                int nextCell = grid[nextRow][c];
                                grid[r][c] = 0;
                                if (nextCell == 0 || nextCell == 5 || nextCell == 6 || nextCell == 2 || nextCell == 4) // move down
                                {
                                    grid[nextRow][c] = 5;
                                }
                                else if (nextCell == 1) // collision with player
                                {
                                    if (hasShield)
                                    {
                                        hasShield = false;
                                        isInvincible = true;
                                        invincibilityTimer.restart();
                                        explosionSound.play();
                                    }
                                    else if (!isInvincible)
                                    {
                                        lives--;
                                        damageSound.play();
                                        isInvincible = true;
                                        invincibilityTimer.restart();
                                        if (lives <= 0)
                                        {
                                            saveHighScoreAndGameOver(score, highScore, saveFile, hasSavedGame,
                                                                   currentState, selectedMenuItem, loseSound);
                                        }
                                    }
                                }
                                else if (nextCell == 3) // collision with bullet
                                {
                                    score += 5;  // 5 points
                                    killCount++; // +1 kill
                                    explosionSound.play();
                                    grid[nextRow][c] = 0;
                                    createExplosionEffect(nextRow, c, hitEffectRow, hitEffectCol, hitEffectTimer,
                                                        hitEffectActive, MAX_HIT_EFFECTS);
                                    // same level up check logic                    
                                    int killsNeeded = level * 10;
                                    if (level < MAX_LEVEL && killCount >= killsNeeded)
                                    {
                                        level++;
                                        levelUpSound.play();
                                        killCount = 0;
                                        bossMoveCounter = 0;
                                        clearEntities(grid);
                                        resetSpaceship(grid, spaceshipCol);
                                        currentState = STATE_LEVEL_UP;
                                        levelUpTimer.restart();
                                        levelUpBlinkClock.restart();
                                    }
                                    else if (level >= MAX_LEVEL && killCount >= killsNeeded)
                                    {
                                        saveHighScoreAndVictory(score, highScore, saveFile, hasSavedGame,
                                                              currentState, selectedMenuItem, winSound);
                                    }
                                }
                            }
                        }
                    }
                }
                // Boss bullet firing logic
                bossMoveCounter++; // boss has moved
                float firingInterval;
                if (level == 3)
                {
                    firingInterval = 4; // fire bullet every 4 movements
                }
                else if (level == 4)
                {
                    firingInterval = 3; // fire every 3 movements
                }
                else
                {
                    firingInterval = 2; // fire every 2 movements
                }
                if (bossMoveCounter >= firingInterval)
                {
                    for (int r = 0; r < ROWS; r++)
                    {
                        for (int c = 0; c < COLS; c++)
                        {
                            if (grid[r][c] == 5)
                            {
                                if (r < ROWS - 1)
                                {
                                    int bulletRow = r + 1;  // just below the boss
                                    if (bulletRow < ROWS && grid[bulletRow][c] == 0)
                                    {
                                        grid[bulletRow][c] = 6; // create bullet
                                    }
                                }
                            }
                        }
                    }
                    bossMoveCounter = 0; // counter reset
                }
                bossMoveClock.restart();
            }
            // boss bullet miovement logic
            float bossBulletSpeed = 0.15f; // Move every 0.15 seconds (very fast, regardless of level)
            if (bossBulletMoveClock.getElapsedTime().asSeconds() >= bossBulletSpeed)
            {
                for (int r = ROWS - 1; r >= 0; r--)
                {
                    for (int c = 0; c < COLS; c++)
                    {
                        if (grid[r][c] == 6)
                        {
                            if (r == ROWS - 1)
                            {
                                grid[r][c] = 0; // remove when below screen
                            }
                            else
                            {
                                grid[r][c] = 0; // Clear current position
                                if (grid[r + 1][c] == 1) // collision with player
                                {
                                    if (hasShield)
                                    {
                                        hasShield = false;
                                        isInvincible = true;
                                        invincibilityTimer.restart();
                                        explosionSound.play();
                                    }
                                    else if (!isInvincible)
                                    {
                                        lives--;
                                        damageSound.play();
                                        isInvincible = true;
                                        invincibilityTimer.restart();
                                        if (lives <= 0)
                                        {
                                            saveHighScoreAndGameOver(score, highScore, saveFile, hasSavedGame,
                                                                   currentState, selectedMenuItem, loseSound);
                                        }
                                    }
                                    createExplosionEffect(r + 1, c, hitEffectRow, hitEffectCol, hitEffectTimer,
                                                        hitEffectActive, MAX_HIT_EFFECTS);
                                }
                                else if (grid[r + 1][c] == 2 || grid[r + 1][c] == 4)
                                {
                                    grid[r + 1][c] = 6; // bullet moves through anything
                                }
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
            // player bullet movement logic almost the same as the boss one
            if (bulletMoveClock.getElapsedTime().asSeconds() >= 0.05f)
            {
                for (int r = 0; r < ROWS; r++)
                {
                    for (int c = 0; c < COLS; c++)
                    {
                        if (grid[r][c] == 3)
                        {
                            if (r == 0)
                            {
                                grid[r][c] = 0; // goes above screen
                            }
                            else
                            {
                                grid[r][c] = 0;
                                if (grid[r - 1][c] == 0 || grid[r - 1][c] == 3)
                                {
                                    grid[r - 1][c] = 3;  // Move bullet up
                                }
                                else if (grid[r - 1][c] == 6) // bullet vs boss bullet
                                {
                                    explosionSound.play();
                                    grid[r - 1][c] = 0; // Destroy both bullets
                                    createExplosionEffect(r - 1, c, hitEffectRow, hitEffectCol, hitEffectTimer,
                                                        hitEffectActive, MAX_HIT_EFFECTS);
                                }
                                else if (grid[r - 1][c] == 2) // bullet vs meteor
                                {
                                    int meteorPoints = 1 + (rand() % 2);
                                    score += meteorPoints;
                                    explosionSound.play();
                                    grid[r - 1][c] = 0;
                                    createExplosionEffect(r - 1, c, hitEffectRow, hitEffectCol, hitEffectTimer,
                                                        hitEffectActive, MAX_HIT_EFFECTS);
                                }
                                else if (grid[r - 1][c] == 4) // bullet vs enemy
                                {
                                    score += 3;
                                    killCount++;
                                    explosionSound.play();
                                    grid[r - 1][c] = 0;
                                    createExplosionEffect(r - 1, c, hitEffectRow, hitEffectCol, hitEffectTimer,
                                                        hitEffectActive, MAX_HIT_EFFECTS);
                                    // levle up check
                                    int killsNeeded = level * 10;
                                    if (level < MAX_LEVEL && killCount >= killsNeeded)
                                    {
                                        level++;
                                        levelUpSound.play();
                                        killCount = 0;
                                        bossMoveCounter = 0;
                                        clearEntities(grid);
                                        resetSpaceship(grid, spaceshipCol);
                                        currentState = STATE_LEVEL_UP;
                                        levelUpTimer.restart();
                                        levelUpBlinkClock.restart();
                                    }
                                    else if (level >= MAX_LEVEL && killCount >= killsNeeded)
                                    {
                                        saveHighScoreAndVictory(score, highScore, saveFile, hasSavedGame,
                                                              currentState, selectedMenuItem, winSound);
                                    }
                                }
                                else if (grid[r - 1][c] == 5) // bullet vs boss
                                {
                                    score += 5;
                                    killCount++;
                                    explosionSound.play();
                                    grid[r - 1][c] = 0;
                                    createExplosionEffect(r - 1, c, hitEffectRow, hitEffectCol, hitEffectTimer,
                                                        hitEffectActive, MAX_HIT_EFFECTS);
                                    int killsNeeded = level * 10;
                                    if (level < MAX_LEVEL && killCount >= killsNeeded)
                                    {
                                        level++;
                                        levelUpSound.play();
                                        killCount = 0;
                                        bossMoveCounter = 0;
                                        clearEntities(grid);
                                        resetSpaceship(grid, spaceshipCol);
                                        currentState = STATE_LEVEL_UP;
                                        levelUpTimer.restart();
                                        levelUpBlinkClock.restart();
                                    }
                                    else if (level >= MAX_LEVEL && killCount >= killsNeeded)
                                    {
                                        saveHighScoreAndVictory(score, highScore, saveFile, hasSavedGame,
                                                              currentState, selectedMenuItem, winSound);
                                    }
                                }
                            }
                        }
                    }
                }
                bulletMoveClock.restart();
            }
            // hit effect management
            float deltaTime = hitEffectClock.getElapsedTime().asSeconds();
            for (int i = 0; i < MAX_HIT_EFFECTS; i++)
            {
                if (hitEffectActive[i])  // all the active effects
                {
                    hitEffectTimer[i] += deltaTime;  // time passes
                    if (hitEffectTimer[i] >= HIT_EFFECT_DURATION)  // check if hit effect visible more than 0.3s
                    {
                        hitEffectActive[i] = false; // remove it
                    }
                }
            }
            hitEffectClock.restart();
            if (isInvincible && invincibilityTimer.getElapsedTime().asSeconds() >= INVINCIBILITY_DURATION)  // check if invincibitly over
            {
                isInvincible = false;
            }
        }
        // Level up screen
        else if (currentState == STATE_LEVEL_UP)
        {
            if (levelUpBlinkClock.getElapsedTime().asSeconds() >= 0.3f) // blibking effect every 0.3s
            {
                levelUpBlinkState = !levelUpBlinkState;  // on and off
                levelUpBlinkClock.restart();
            }
            if (levelUpTimer.getElapsedTime().asSeconds() >= 2.0f) // after 2s back to playing
            {
                currentState = STATE_PLAYING;
                restartAllClocks(meteorSpawnClock, meteorMoveClock, enemySpawnClock, enemyMoveClock,
                                bossSpawnClock, bossMoveClock, bossBulletMoveClock, bulletMoveClock,
                                shieldPowerupSpawnClock, shieldPowerupMoveClock);
            }
        }
        // Victory screen
        else if (currentState == STATE_VICTORY)
        {
            if (menuClock.getElapsedTime() >= menuCooldown)  // same menu logic 
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
                    if (selectedMenuItem == 0)  // (restart Game)
                    {
                        currentState = STATE_PLAYING;
                        // start fresh
                        lives = 3;
                        score = 0;
                        killCount = 0;
                        level = 1;
                        bossMoveCounter = 0;
                        isInvincible = false;
                        hasShield = false;
                        clearGrid(grid);
                        for (int i = 0; i < MAX_SHIELD_POWERUPS; i++)
                        {
                            shieldPowerupActive[i] = false;
                        }
                        resetSpaceship(grid, spaceshipCol);
                        restartAllClocks(meteorSpawnClock, meteorMoveClock, enemySpawnClock, enemyMoveClock,
                                        bossSpawnClock, bossMoveClock, bossBulletMoveClock, bulletMoveClock,
                                        shieldPowerupSpawnClock, shieldPowerupMoveClock);
                    }
                    else if (selectedMenuItem == 1)  // (main menu)
                    {
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
        // Pause screen
        else if (currentState == STATE_PAUSED)
        {
            if (menuClock.getElapsedTime() >= menuCooldown) // same menu logic
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
                    if (selectedMenuItem == 0) // (resume game)
                    {
                        currentState = STATE_PLAYING;
                    }
                    else if (selectedMenuItem == 1)  // (restart level)
                    {
                        currentState = STATE_PLAYING;
                        killCount = 0;
                        bossMoveCounter = 0;
                        isInvincible = false;
                        hasShield = false;
                        clearGrid(grid);
                        for (int i = 0; i < MAX_SHIELD_POWERUPS; i++)
                        {
                            shieldPowerupActive[i] = false;
                        }
                        resetSpaceship(grid, spaceshipCol);
                        restartAllClocks(meteorSpawnClock, meteorMoveClock, enemySpawnClock, enemyMoveClock,
                                        bossSpawnClock, bossMoveClock, bossBulletMoveClock, bulletMoveClock,
                                        shieldPowerupSpawnClock, shieldPowerupMoveClock);
                    }
                    else if (selectedMenuItem == 2)  // (save and quit
                    {
                        ofstream outputFile(saveFile); // open file and save all score etc to it
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
                {
                    currentState = STATE_PLAYING;
                    menuAction = true;
                }
                if (menuAction)
                {
                    menuClock.restart();
                }
            }
        }
        // SFML Rendering for each Game Screen
        window.clear(Color(40, 40, 40)); // Dark Gray Backfground
        // Menu Screen
        if (currentState == STATE_MENU)
        {
            window.draw(menuBackground);
            window.draw(menuTitle);
            char menuHighScoreBuffer[50];
            sprintf(menuHighScoreBuffer, "High Score: %d", highScore); // %d fetches from highscore var and updates the string
            menuHighScoreText.setString(menuHighScoreBuffer);
            menuHighScoreText.setPosition(windowWidth / 2 - menuHighScoreText.getLocalBounds().width / 2.0f, 180);
            window.draw(menuHighScoreText);
            for (int i = 0; i < 4; i++)
            {
                menuItems[i].setFillColor(i == selectedMenuItem ? Color::Yellow : Color::White);
                window.draw(menuItems[i]);
            }
            window.draw(menuInstructions);
        }
        // Instructions Screen
        else if (currentState == STATE_INSTRUCTIONS)
        {
            window.draw(menuBackground);
            window.draw(instructionsTitle);
            window.draw(controlsTitle);
            window.draw(moveText);
            window.draw(shootText);
            window.draw(pauseText);
            window.draw(entitiesTitle);
            spaceship.setPosition(60, 285);
            window.draw(spaceship);
            window.draw(playerDesc);
            meteor.setPosition(60, 325);
            window.draw(meteor);
            window.draw(meteorDesc);
            enemy.setPosition(60, 365);
            window.draw(enemy);
            window.draw(enemyDesc);
            bossEnemy.setPosition(60, 405);
            window.draw(bossEnemy);
            window.draw(bossDesc);
            bullet.setPosition(60 + BULLET_OFFSET_X, 445);
            window.draw(bullet);
            window.draw(bulletDesc);
            bossBullet.setPosition(60 + BULLET_OFFSET_X, 485);
            window.draw(bossBullet);
            window.draw(bossBulletDesc);
            lifeIcon.setPosition(60 + 8, 525);
            window.draw(lifeIcon);
            window.draw(lifeDesc);
            shieldPowerUp.setPosition(60, 565);
            window.draw(shieldPowerUp);
            window.draw(shieldPowerupDesc);
            window.draw(systemsTitle);
            window.draw(livesDesc);
            window.draw(levelsDesc);
            window.draw(highScoreDesc);
            window.draw(objectiveTitle);
            window.draw(objective1);
            window.draw(objective2);
            window.draw(objective3);
            window.draw(instructionsBack);
        }
        // Playing Screen
        else if (currentState == STATE_PLAYING)
        {
            window.draw(background);
            window.draw(gameBox);
            // File all the grid with relevant sprites based on 0-6
            for (int r = 0; r < ROWS; r++)
            {
                for (int c = 0; c < COLS; c++)
                {
                    if (grid[r][c] == 1)
                    {
                        spaceship.setPosition(MARGIN + c * CELL_SIZE, MARGIN + r * CELL_SIZE);
                        if (!isInvincible || ((int)(invincibilityTimer.getElapsedTime().asMilliseconds() / 100) % 2 == 0))
                        {
                            window.draw(spaceship);
                        }
                    }
                    else if (grid[r][c] == 2)
                    {
                        meteor.setPosition(MARGIN + c * CELL_SIZE, MARGIN + r * CELL_SIZE);
                        window.draw(meteor);
                    }
                    else if (grid[r][c] == 3)
                    {
                        bullet.setPosition(MARGIN + c * CELL_SIZE + BULLET_OFFSET_X, MARGIN + r * CELL_SIZE);
                        window.draw(bullet);
                    }
                    else if (grid[r][c] == 4)
                    {
                        enemy.setPosition(MARGIN + c * CELL_SIZE, MARGIN + r * CELL_SIZE);
                        window.draw(enemy);
                    }
                    else if (grid[r][c] == 5)
                    {
                        bossEnemy.setPosition(MARGIN + c * CELL_SIZE, MARGIN + r * CELL_SIZE);
                        window.draw(bossEnemy);
                    }
                    else if (grid[r][c] == 6)
                    {
                        bossBullet.setPosition(MARGIN + c * CELL_SIZE + BULLET_OFFSET_X, MARGIN + r * CELL_SIZE);
                        window.draw(bossBullet);
                    }
                }
            }
            // Show all powerups
            for (int i = 0; i < MAX_SHIELD_POWERUPS; i++)
            {
                if (shieldPowerupActive[i])
                {
                    shieldPowerUp.setPosition(MARGIN + shieldPowerupCol[i] * CELL_SIZE, MARGIN + shieldPowerupRow[i] * CELL_SIZE); // set posioton relative to the grid
                    window.draw(shieldPowerUp);
                }
            }
            if (hasShield) // draw shield over the player
            {
                shieldIcon.setPosition(MARGIN + spaceshipCol * CELL_SIZE + SHIELD_OFFSET, MARGIN + (ROWS - 1) * CELL_SIZE + SHIELD_OFFSET);
                window.draw(shieldIcon);
            }
            for (int i = 0; i < MAX_HIT_EFFECTS; i++)
            {
                if (hitEffectActive[i])
                {
                    bulletHit.setPosition(MARGIN + hitEffectCol[i] * CELL_SIZE, MARGIN + hitEffectRow[i] * CELL_SIZE);
                    window.draw(bulletHit);
                }
            }
            livesText.setString("Lives:");
            // Icon for lives remaining
            float lifeIconStartX = livesText.getPosition().x + livesText.getLocalBounds().width + 10;
            float lifeIconY = livesText.getPosition().y + (livesText.getLocalBounds().height / 2.0f) - 12;
            for (int i = 0; i < lives; i++) // draw based on how many left
            {
                lifeIcon.setPosition(lifeIconStartX + (i * 28), lifeIconY); // + (i*28) so that they dont draw on top of each other
                window.draw(lifeIcon);
            }
            char scoreBuffer[20];
            sprintf(scoreBuffer, "Score: %d", score); // same update logic
            scoreText.setString(scoreBuffer);
            char killsBuffer[50];
            sprintf(killsBuffer, "Kills: %d/%d", killCount, level * 10);
            killsText.setString(killsBuffer);
            char levelBuffer[20];
            sprintf(levelBuffer, "Level: %d", level);
            levelText.setString(levelBuffer);
            char highScoreBuffer[50];
            sprintf(highScoreBuffer, "High Score: %d", highScore);
            highScoreText.setString(highScoreBuffer);
            window.draw(title);
            window.draw(livesText);
            window.draw(scoreText);
            window.draw(killsText);
            window.draw(levelText);
            window.draw(highScoreText);
        }
        // Level Up Screen
        else if (currentState == STATE_LEVEL_UP)
        {
            window.draw(background);
            window.draw(gameBox);
            spaceship.setPosition(MARGIN + spaceshipCol * CELL_SIZE, MARGIN + (ROWS - 1) * CELL_SIZE);
            window.draw(spaceship);
            if (levelUpBlinkState)
            {
                window.draw(levelUpText);
            }
            char levelBuffer[20];
            sprintf(levelBuffer, "Level: %d", level);
            levelText.setString(levelBuffer);

            char killsBuffer[50];
            sprintf(killsBuffer, "Kills: %d/%d", killCount, level * 10);
            killsText.setString(killsBuffer);

            // Draw UI elements (same as gameplay screen)
            window.draw(title);
            window.draw(livesText);
            window.draw(scoreText);
            window.draw(killsText);
            window.draw(levelText);
        }
        // Pause Screen
        else if (currentState == STATE_PAUSED)
        {
            window.draw(background);
            window.draw(gameBox);
            for (int r = 0; r < ROWS; r++)
            {
                for (int c = 0; c < COLS; c++)
                {
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
            RectangleShape overlay(Vector2f(COLS * CELL_SIZE, ROWS * CELL_SIZE));
            overlay.setPosition(MARGIN, MARGIN);
            overlay.setFillColor(Color(0, 0, 0, 150)); // semi transparent background
            window.draw(overlay);
            window.draw(pauseTitle);
            for (int i = 0; i < 3; i++)
            {
                pauseItems[i].setFillColor(i == selectedMenuItem ? Color::Yellow : Color::White);
                window.draw(pauseItems[i]);
            }
        }
        // Victory Screen
        else if (currentState == STATE_VICTORY)
        {
            window.draw(menuBackground);
            window.draw(victoryTitle);
            char victoryScoreBuffer[50];
            sprintf(victoryScoreBuffer, "Final Score: %d", score); // same update logic
            victoryScore.setString(victoryScoreBuffer);
            victoryScore.setPosition(windowWidth / 2 - victoryScore.getLocalBounds().width / 2.0f, 200);
            window.draw(victoryScore);
            for (int i = 0; i < 2; i++)
            {
                victoryItems[i].setFillColor(i == selectedMenuItem ? Color::Yellow : Color::White);
                window.draw(victoryItems[i]);
            }
            window.draw(victoryInstructions);
        }
        // Game Over Screen
        else if (currentState == STATE_GAME_OVER)
        {
            window.draw(menuBackground);
            window.draw(gameOverTitle);
            char gameOverScoreBuffer[50];
            sprintf(gameOverScoreBuffer, "Final Score: %d", score);
            gameOverScore.setString(gameOverScoreBuffer);
            gameOverScore.setPosition(windowWidth / 2 - gameOverScore.getLocalBounds().width / 2.0f, 200);
            window.draw(gameOverScore);
            for (int i = 0; i < 2; i++)
            {
                gameOverItems[i].setFillColor(i == selectedMenuItem ? Color::Yellow : Color::White);
                window.draw(gameOverItems[i]);
            }
            window.draw(gameOverInstructions);
        }
        // After Drawing everything, display it on the screen
        window.display();
    }
    return 0;
}
