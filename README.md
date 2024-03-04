# Space Asteroids Game

## Introduction

Welcome to Space Asteroids, a thrilling 2D space-themed game where you navigate a spaceship through an asteroid-filled space, collect power-ups, and reach the space station to advance through levels. Dodge asteroids, challenge yourself with progressively difficult levels, and enjoy an engaging gaming experience.

## Compilation and Execution Guide

### Prerequisites

Before compiling and running the game, ensure you have the following dependencies installed:

- OpenGL
- GLUT (OpenGL Utility Toolkit)
- STB Image library

### File Structure

To run the game properly, make sure your folder contains the following files:

1. `main.cpp`: The main source code file.
2. `asteroid.jpeg`: Image file for asteroid texture.
3. `player.png`: Image file for the spaceship texture.
4. `stb_image.h`: Library for loading image files. You can download it [here](https://github.com/nothings/stb/blob/master/stb_image.h).

### Compilation

1. Open a terminal window.
2. Navigate to the directory containing the game files.
3. Use the following command to compile the code:

```bash
g++ -o space_asteroids main.cpp -lGL -lGLU -lglut -std=c++11 -lpthread
```

### Execution

After successful compilation, run the executable with:

```bash
./space_asteroids
```

### Game Controls

- **Start Game:** Down Arrow Key
- **Move Up:** Up Arrow key
- **Move Down:** Down Arrow key
- **Move Left:** Left Arrow key
- **Move Right:** Right Arrow key
- **Reset Game:** R key

### Game Over

The game ends when your spaceship collides with an asteroid. After the game is over, you can reset by pressing the 'R' key.

## Conclusion

Space Asteroids provides an enjoyable gaming experience with intuitive controls, challenging levels, and exciting power-ups. Have a great time exploring space and mastering the art of asteroid dodging!

**Author:** Shah Keval Bhushanbhai (B21C069)
