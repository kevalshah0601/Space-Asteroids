#include <GL/glut.h>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <vector>
#include <cstring>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// Global variables
bool gameStarted = false; 
const int MAX_ASTEROIDS = 17;
int avoidedAsteroidsCount = 0;
float playerSpeedMultiplier = 0.0f;
int currentLevel = 1;
float minimapX = 20.0f;
float minimapY = 20.0f;
float minimapSize = 150.0f;

// Structure to represent an asteroid
struct Asteroid {
    float x, y;
    float size;
    float speed;
    float rotationAngle;
    float rotationSpeed;
    int numVertices;
    bool collided;
    GLuint asteroidTexture; 
};

// Structure to represent a power-up
struct PowerUp {
    float x, y;
    float width, height;
    float speed;
    bool active;
};

// Global variables for power-up and its size
PowerUp powerUp;
const float POWERUP_SIZE = 20.0f;

// Array of asteroids and coordinates for the space station
Asteroid asteroids[MAX_ASTEROIDS];
float spaceStationX, spaceStationY;

// Player constants
const int PLAYER_SIZE = 50; // Adjusted size to fit the image
float playerX = 400, playerY = 20;
float playerSpeed = 11.0f;

// Counters and variables for health, score, and game over state
int collisionCounter = 0;
float initialWindowSizeX = 800.0f;
float initialWindowSizeY = 600.0f;

// Texture for the spaceship
GLuint spaceshipTexture;

int playerHealth = 100;
int maxHealth = 100;

int score = 0;
bool gameOver = false;

// Function to load texture from file
void loadTexture(const char* filename, GLuint& texture) {
    int width, height, numChannels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(filename, &width, &height, &numChannels, 0);

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

    stbi_image_free(data);
}

// Function to draw the power-up
void drawPowerUp() {
    if (powerUp.active) {
        glColor3f(0.0f, 1.0f, 0.0f); // Green color for the power-up
        glBegin(GL_QUADS);
        glVertex2f(powerUp.x - powerUp.width / 2, powerUp.y - powerUp.height / 2);
        glVertex2f(powerUp.x + powerUp.width / 2, powerUp.y - powerUp.height / 2);
        glVertex2f(powerUp.x + powerUp.width / 2, powerUp.y + powerUp.height / 2);
        glVertex2f(powerUp.x - powerUp.width / 2, powerUp.y + powerUp.height / 2);
        glEnd();
    }
}

// Function to update power-up position and check collision
void updatePowerUp() {
    if (!powerUp.active) {
        static int powerUpTimer = 0;
        powerUpTimer++;

        // Activate power-up every 10 seconds (adjust as needed) only if the game is in progress
        if (powerUpTimer >= 600 && gameStarted && !gameOver) {
            powerUp.active = true;
            powerUp.x = rand() % int(initialWindowSizeX);
            powerUp.y = initialWindowSizeY + POWERUP_SIZE;
            powerUpTimer = 0; // Reset the timer
        }
    }

    if (powerUp.active) {
        powerUp.y -= powerUp.speed;

        // Check collision with the player
        float dx = powerUp.x - playerX;
        float dy = powerUp.y - playerY;
        float distance = sqrt(dx * dx + dy * dy);

        if (distance < (powerUp.width + PLAYER_SIZE) / 2) {
            powerUp.active = false;
            playerHealth = maxHealth; // Restore player's health to maximum
        }

        // Check if power-up is out of bounds
        if (powerUp.y < 0) {
            powerUp.active = false;
        }
    }
}

// Function to initialize game elements
void init() {
    loadTexture("player.png", spaceshipTexture);

    for (int i = 0; i < MAX_ASTEROIDS; i++) {
        asteroids[i].x = rand() % int(initialWindowSizeX);
        asteroids[i].y = rand() % int(initialWindowSizeY);
        asteroids[i].size = rand() % 30 + 10;
        asteroids[i].speed = (rand() % 8 + 1) / 5.0f;
        asteroids[i].rotationSpeed = (rand() % 5 + 1) * 2.0f;
        asteroids[i].numVertices = rand() % 6 + 5;
        asteroids[i].rotationAngle = 0.0f;
        asteroids[i].collided = false;
        char asteroidTexturePath[50];
        sprintf(asteroidTexturePath, "asteroid.jpeg");  
        loadTexture(asteroidTexturePath, asteroids[i].asteroidTexture);
    }

    spaceStationX = rand() % int(initialWindowSizeX);
    spaceStationY = rand() % int(initialWindowSizeY / 2) + initialWindowSizeY / 2;
    powerUp.x = rand() % int(initialWindowSizeX);
    powerUp.y = initialWindowSizeY + POWERUP_SIZE; // Start above the window
    powerUp.width = POWERUP_SIZE;
    powerUp.height = POWERUP_SIZE;
    powerUp.speed = 4.0f; // Adjust the speed as needed
    powerUp.active = false;
}

// Function to draw the health bar
void drawHealthBar() {
    // Draw full health bar
    glColor3f(0.0f, 1.0f, 0.0f); // Green color for the full health
    glBegin(GL_QUADS);
    glVertex2f(playerX - PLAYER_SIZE / 2, playerY - 25);
    glVertex2f(playerX - PLAYER_SIZE / 2 + (static_cast<float>(playerHealth) / maxHealth) * PLAYER_SIZE, playerY - 25);
    glVertex2f(playerX - PLAYER_SIZE / 2 + (static_cast<float>(playerHealth) / maxHealth) * PLAYER_SIZE, playerY - 20);
    glVertex2f(playerX - PLAYER_SIZE / 2, playerY - 20);
    glEnd();

    // Draw empty health bar
    glColor3f(1.0f, 0.0f, 0.0f); // Red color for the empty health
    glBegin(GL_QUADS);
    glVertex2f(playerX - PLAYER_SIZE / 2 + (static_cast<float>(playerHealth) / maxHealth) * PLAYER_SIZE, playerY - 25);
    glVertex2f(playerX + PLAYER_SIZE / 2, playerY - 25);
    glVertex2f(playerX + PLAYER_SIZE / 2, playerY - 20);
    glVertex2f(playerX - PLAYER_SIZE / 2 + (static_cast<float>(playerHealth) / maxHealth) * PLAYER_SIZE, playerY - 20);
    glEnd();
}

// Function to draw the spaceship
void drawSpaceship() {
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, spaceshipTexture);
    glColor3f(1.0f, 1.0f, 1.0f);

    glBegin(GL_QUADS);
    glColor3f(0.0f, 0.0f, 0.0f); // Black color for the background

    glVertex2f(playerX - PLAYER_SIZE / 2, playerY);
    glVertex2f(playerX + PLAYER_SIZE / 2, playerY);
    glVertex2f(playerX + PLAYER_SIZE / 2, playerY + PLAYER_SIZE);
    glVertex2f(playerX - PLAYER_SIZE / 2, playerY + PLAYER_SIZE);

    glColor3f(1.0f, 1.0f, 1.0f); // White color for the spaceship

    glTexCoord2f(0.0f, 0.0f);
    glVertex2f(playerX - PLAYER_SIZE / 2, playerY);

    glTexCoord2f(1.0f, 0.0f);
    glVertex2f(playerX + PLAYER_SIZE / 2, playerY);

    glTexCoord2f(1.0f, 1.0f);
    glVertex2f(playerX + PLAYER_SIZE / 2, playerY + PLAYER_SIZE);

    glTexCoord2f(0.0f, 1.0f);
    glVertex2f(playerX - PLAYER_SIZE / 2, playerY + PLAYER_SIZE);
    glEnd();

    glDisable(GL_TEXTURE_2D);

    // Draw health bar
    drawHealthBar();
}

// Function to draw the asteroids using textures
void drawAsteroids() {
    glEnable(GL_TEXTURE_2D);

    for (int i = 0; i < MAX_ASTEROIDS; i++) {
        if (!asteroids[i].collided) {
            glBindTexture(GL_TEXTURE_2D, asteroids[i].asteroidTexture);
            glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

            glPushMatrix();
            glTranslatef(asteroids[i].x, asteroids[i].y, 0);
            glRotatef(asteroids[i].rotationAngle, 0.0f, 0.0f, 1.0f);

            glBegin(GL_POLYGON);
            for (int j = 0; j < asteroids[i].numVertices; j++) {
                float angle = 2.0f * M_PI * j / asteroids[i].numVertices;
                float radius = asteroids[i].size / 1.0f;
                float x = radius * cos(angle);
                float y = radius * sin(angle);

                // Texture coordinates
                float texCoordX = (x + radius) / (2.0f * radius);
                float texCoordY = (y + radius) / (2.0f * radius);

                glTexCoord2f(texCoordX, texCoordY);
                glVertex2f(x, y);
            }
            glEnd();

            glPopMatrix();
        }
    }

    glDisable(GL_TEXTURE_2D);
}

// Function to draw the space station
void drawSpaceStation() {
    // Central Hub
    glColor3f(0.25f, 0.88f, 0.82f); 
    glBegin(GL_QUADS);
    glVertex2f(spaceStationX - 20, spaceStationY - 20);
    glVertex2f(spaceStationX + 20, spaceStationY - 20);
    glVertex2f(spaceStationX + 20, spaceStationY + 20);
    glVertex2f(spaceStationX - 20, spaceStationY + 20);
    glEnd();

    // Rotating Arms
    glColor3f(1.0f, 0.84f, 0.0f); 
    for (int i = 0; i < 4; i++) {
        glPushMatrix();
        glTranslatef(spaceStationX, spaceStationY, 0.0f);
        glRotatef(i * 90.0f, 0.0f, 0.0f, 1.0f);
        glTranslatef(25.0f, 0.0f, 0.0f);

        glBegin(GL_QUADS);
        glVertex2f(-5.0f, -15.0f);
        glVertex2f(5.0f, -15.0f);
        glVertex2f(5.0f, 15.0f);
        glVertex2f(-5.0f, 15.0f);
        glEnd();

        glPopMatrix();
    }
}

// Function to draw the welcome screen
void drawWelcomeScreen() {
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity(); // Load the identity matrix to clear any transformations

    if (!gameOver) {
        if (!gameStarted) {
            // Display welcome text
            glColor3f(1.0f, 1.0f, 1.0f);
            int textWidth = glutBitmapLength(GLUT_BITMAP_TIMES_ROMAN_24, (const unsigned char*)"Welcome to Space Asteroids");
            int textHeight = 24; // Height of the font

            int centerX = (initialWindowSizeX - textWidth) / 2;
            int centerY = initialWindowSizeY / 2;

            glRasterPos2f(centerX, centerY + textHeight / 2);
            for (const char* c = "Welcome to Space Asteroids"; *c != '\0'; c++) {
                glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, *c);
            }

            // Display instruction text
            int smallerTextWidth = glutBitmapLength(GLUT_BITMAP_HELVETICA_18, (const unsigned char*)"Press Down Key to play");
            int smallerTextHeight = 18;

            int smallerCenterX = (initialWindowSizeX - smallerTextWidth) / 2;
            int smallerCenterY = centerY - textHeight + smallerTextHeight / 2;

            glRasterPos2f(smallerCenterX, smallerCenterY);
            for (const char* c = "Press Down Key to play"; *c != '\0'; c++) {
                glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
            }
        } else {
            // Display score during gameplay
            glColor3f(1.0f, 1.0f, 1.0f);
            char scoreText[50];
            sprintf(scoreText, "Score: %d", score);

            int scoreTextWidth = glutBitmapLength(GLUT_BITMAP_HELVETICA_18, (const unsigned char*)scoreText);
            int scoreTextHeight = 18;

            int scoreX = (initialWindowSizeX - scoreTextWidth) / 2;
            int scoreY = initialWindowSizeY / 2 - scoreTextHeight - 10;

            glRasterPos2f(scoreX, scoreY);
            for (const char* c = scoreText; *c != '\0'; c++) {
                glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
            }
        }
    } else {
        // Display game over screen
        glColor3f(1.0f, 1.0f, 1.0f);
        int gameOverTextWidth = glutBitmapLength(GLUT_BITMAP_TIMES_ROMAN_24, (const unsigned char*)"Game Over");
        int gameOverTextHeight = 24; // Height of the font

        int gameOverX = (initialWindowSizeX - gameOverTextWidth) / 2;
        int gameOverY = initialWindowSizeY / 2;

        glRasterPos2f(gameOverX, gameOverY);
        for (const char* c = "Game Over"; *c != '\0'; c++) {
            glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, *c);
        }

        // Display final score
        char finalScoreText[50];
        sprintf(finalScoreText, "Final Score: %d", score);

        int finalScoreTextWidth = glutBitmapLength(GLUT_BITMAP_HELVETICA_18, (const unsigned char*)finalScoreText);
        int finalScoreTextHeight = 18;

        int finalScoreX = (initialWindowSizeX - finalScoreTextWidth) / 2;
        int finalScoreY = gameOverY - gameOverTextHeight - finalScoreTextHeight;

        glRasterPos2f(finalScoreX, finalScoreY);
        for (const char* c = finalScoreText; *c != '\0'; c++) {
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
        }

        // Display restart instruction
        int restartTextWidth = glutBitmapLength(GLUT_BITMAP_HELVETICA_18, (const unsigned char*)"Press R to restart");
        int restartTextHeight = 18;

        int restartX = (initialWindowSizeX - restartTextWidth) / 2;
        int restartY = finalScoreY - finalScoreTextHeight - restartTextHeight;

        glRasterPos2f(restartX, restartY);
        for (const char* c = "Press R to restart"; *c != '\0'; c++) {
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
        }
    }

    glutSwapBuffers();
}

// Function to update asteroid positions
void updateAsteroids() {
    for (int i = 0; i < MAX_ASTEROIDS; i++) {
        asteroids[i].y -= asteroids[i].speed;
        asteroids[i].rotationAngle += asteroids[i].rotationSpeed;

        if (asteroids[i].y < 0) {
            // Reset asteroid properties when out of bounds
            asteroids[i].y = initialWindowSizeY;
            asteroids[i].x = rand() % int(initialWindowSizeX);
            asteroids[i].size = rand() % 30 + 10;
            asteroids[i].speed = (rand() % 8 + 1) / 5.0f;
            asteroids[i].rotationSpeed = (rand() % 5 + 1) * 2.0f;
            asteroids[i].numVertices = rand() % 6 + 5;
            asteroids[i].rotationAngle = 0.0f;
            asteroids[i].collided = false;
        }
    }
}

// Function to check collisions between the spaceship and asteroids
void checkCollisions() {
    for (int i = 0; i < MAX_ASTEROIDS; i++) {
        if (!asteroids[i].collided) {
            float dx = asteroids[i].x - playerX;
            float dy = asteroids[i].y - playerY;
            float distance = sqrt(dx * dx + dy * dy);

            if (distance < (asteroids[i].size + PLAYER_SIZE) / 2) {
                // Handle collision with an asteroid
                asteroids[i].collided = true;
                collisionCounter++;

                // Deduct health based on asteroid size
                playerHealth -= static_cast<int>(asteroids[i].size);

                if (playerHealth <= 0) {
                    // Game over logic
                    playerHealth = 0;
                    gameOver = true;
                }

                // Increase score based on asteroid size
                score += static_cast<int>(asteroids[i].size);

                if (!gameOver) {
                    // Increase speed multiplier when avoiding asteroids
                    playerSpeedMultiplier += 0.1f;
                    avoidedAsteroidsCount++;
                }
            }
        }
    }
}

// Function to check collision with the space station
void checkSpaceStationCollision() {
    // Calculate distance between player and space station
    float dx = spaceStationX - playerX;
    float dy = spaceStationY - playerY;
    float distance = sqrt(dx * dx + dy * dy);

    // Check if collision occurred
    if (distance < (20 + PLAYER_SIZE) / 2) {
        // Reset player position after reaching the space station
        playerX = 400;
        playerY = 20;

        // Increase level and update score based on current level and speed multiplier
        currentLevel++;
        int levelScoreBonus = static_cast<int>(currentLevel * playerSpeedMultiplier * 5);
        score += levelScoreBonus;

        // Adjust asteroid properties based on the level
        for (int i = 0; i < MAX_ASTEROIDS; i++) {
            asteroids[i].size += 5; // Increase asteroid size
            asteroids[i].speed += 0.5f; // Increase asteroid speed
            asteroids[i].rotationSpeed += 0.5f; // Increase asteroid rotation speed
        }

        // Reset space station position
        spaceStationX = rand() % int(initialWindowSizeX);
        spaceStationY = rand() % int(initialWindowSizeY / 2) + initialWindowSizeY / 2;
    }
}

// Function to draw the minimap
void drawMinimap() {
    // Only draw the minimap if the game is started
    if (gameStarted) {
        // Draw a smaller border around the minimap
        float minimapBorderSize = 50.0f;
        glColor3f(1.0f, 1.0f, 1.0f);
        glBegin(GL_LINE_LOOP);
        glVertex2f(minimapX, minimapY);
        glVertex2f(minimapX + minimapBorderSize, minimapY);
        glVertex2f(minimapX + minimapBorderSize, minimapY + minimapBorderSize);
        glVertex2f(minimapX, minimapY + minimapBorderSize);
        glEnd();

        // Draw player position on the minimap
        glColor3f(1.0f, 1.0f, 1.0f);
        float minimapPlayerX = minimapX + (playerX / initialWindowSizeX) * minimapBorderSize;
        float minimapPlayerY = minimapY + (playerY / initialWindowSizeY) * minimapBorderSize;
        glBegin(GL_QUADS);
        glVertex2f(minimapPlayerX - 1, minimapPlayerY - 1);
        glVertex2f(minimapPlayerX + 1, minimapPlayerY - 1);
        glVertex2f(minimapPlayerX + 1, minimapPlayerY + 1);
        glVertex2f(minimapPlayerX - 1, minimapPlayerY + 1);
        glEnd();

        // Draw space station on the minimap
        float minimapStationX = minimapX + (spaceStationX / initialWindowSizeX) * minimapBorderSize;
        float minimapStationY = minimapY + (spaceStationY / initialWindowSizeY) * minimapBorderSize;
        glColor3f(1.0f, 1.0f, 0.0f);
        glBegin(GL_QUADS);
        glVertex2f(minimapStationX - 2, minimapStationY - 2);
        glVertex2f(minimapStationX + 2, minimapStationY - 2);
        glVertex2f(minimapStationX + 2, minimapStationY + 2);
        glVertex2f(minimapStationX - 2, minimapStationY + 2);
        glEnd();
    }
}

// Function to render the game scene
void render() {
    glClear(GL_COLOR_BUFFER_BIT);

    if (!gameOver) {
        if (gameStarted) {
            glPushMatrix();
            glPushMatrix();
            drawSpaceship();
            drawAsteroids();
            drawSpaceStation();
            glColor3f(1.0f, 1.0f, 1.0f);

            // Display score
            char scoreText[20];
            sprintf(scoreText, "Score: %d", score);

            int scoreTextWidth = glutBitmapLength(GLUT_BITMAP_HELVETICA_12, (const unsigned char*)scoreText);
            int scoreTextHeight = 12;

            int scoreX = 10;
            int scoreY = initialWindowSizeY - scoreTextHeight - 10;

            glRasterPos2f(scoreX, scoreY);
            for (const char* c = scoreText; *c != '\0'; c++) {
                glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *c);
            }

            // Display current level on the top right corner
            char levelText[20];
            sprintf(levelText, "Level: %d", currentLevel);

            int levelTextWidth = glutBitmapLength(GLUT_BITMAP_HELVETICA_12, (const unsigned char*)levelText);
            int levelTextHeight = 12;

            int levelX = initialWindowSizeX - levelTextWidth - 10;
            int levelY = initialWindowSizeY - levelTextHeight - 10;

            glRasterPos2f(levelX, levelY);
            for (const char* c = levelText; *c != '\0'; c++) {
                glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *c);
            }
            glPopMatrix();
        } else {
            drawWelcomeScreen();
        }
    } else {
        drawWelcomeScreen();
    }

    drawPowerUp();
    drawMinimap();
    glutSwapBuffers();
}

// Function to update game state
void update(int value) {
    if (!gameOver) {
        if (gameStarted) {
            updateAsteroids();
            checkCollisions();
            checkSpaceStationCollision();
        }
    }

    updatePowerUp();
    glutPostRedisplay();
    glutTimerFunc(16, update, 0); // 60 FPS
}

// Function to handle special keys
void specialKeys(int key, int x, int y) {
    if (!gameOver && key == GLUT_KEY_DOWN) {
        gameStarted = true;
    }

    if (!gameOver && gameStarted) {
        switch (key) {
        case GLUT_KEY_UP:
            playerY += playerSpeed;
            playerSpeedMultiplier = 2.0f;  // Increase speed multiplier when moving up
            break;
        case GLUT_KEY_DOWN:
            playerY -= playerSpeed;
            break;
        case GLUT_KEY_LEFT:
            playerX -= playerSpeed;
            break;
        case GLUT_KEY_RIGHT:
            playerX += playerSpeed;
            break;
        }

        // Keep the player within the screen boundaries
        playerX = fmax(0, fmin(initialWindowSizeX, playerX));
        playerY = fmax(0, fmin(initialWindowSizeY, playerY));
    }

    glutPostRedisplay();
}

// Function to restart the game
void restartGame() {
    // Reset game state
    gameOver = false;
    gameStarted = false;
    score = 0;
    currentLevel = 1;  // Reset current level to 1

    // Reset player position and health
    playerX = 400;
    playerY = 20;
    playerHealth = maxHealth;

    // Reset asteroids and space station
    for (int i = 0; i < MAX_ASTEROIDS; i++) {
        asteroids[i].x = rand() % int(initialWindowSizeX);
        asteroids[i].y = rand() % int(initialWindowSizeY);
        asteroids[i].size = rand() % 30 + 10;
        asteroids[i].speed = (rand() % 8 + 1) / 5.0f;
        asteroids[i].rotationSpeed = (rand() % 5 + 1) * 2.0f;
        asteroids[i].numVertices = rand() % 6 + 5;
        asteroids[i].rotationAngle = 0.0f;
        asteroids[i].collided = false;
    }

    spaceStationX = rand() % int(initialWindowSizeX);
    spaceStationY = rand() % int(initialWindowSizeY / 2) + initialWindowSizeY / 2;
}

// Function to handle keyboard input
void keyboard(unsigned char key, int x, int y) {
    if (gameOver && key == 'r') {
        restartGame();
    }
}

// Function to handle window reshaping
void reshape(int width, int height) {
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, width, 0, height);
    glMatrixMode(GL_MODELVIEW);
    initialWindowSizeX = width;
    initialWindowSizeY = height;

    // Update positions based on the new window size
    for (int i = 0; i < MAX_ASTEROIDS; i++) {
        asteroids[i].x = rand() % int(initialWindowSizeX);
        asteroids[i].y = rand() % int(initialWindowSizeY);
    }

    spaceStationX = rand() % int(initialWindowSizeX);
    spaceStationY = rand() % int(initialWindowSizeY / 2) + initialWindowSizeY / 2;

    // Only draw the welcome screen if the game is not started
    if (!gameStarted) {
        drawWelcomeScreen();
    }
}

// Main function
int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Space Asteroids");
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glutDisplayFunc(render);
    glutReshapeFunc(reshape);
    glutTimerFunc(0, update, 0);
    glutSpecialFunc(specialKeys);
    glutKeyboardFunc(keyboard);
    init();
    glutMainLoop();
    return 0;
}
