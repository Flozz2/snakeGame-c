#ifndef GAME_H
#define GAME_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <stdbool.h>

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480
#define CELL_SIZE 40
#define INITIAL_LENGTH 3
#define FONT_SIZE 24
#define BASE_DELAY_MS 200

typedef enum {
    START_SCREEN,
    GAME_RUNNING,
    GAME_OVER
} GameState;

typedef struct {
    int x;
    int y;
} Point;

typedef enum {
    UP,
    RIGHT,
    DOWN,
    LEFT
} Direction;

typedef struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *backgroundTexture;
    SDL_Texture *headTexture;
    SDL_Texture *bodyTexture;
    SDL_Texture *tailTexture;
    SDL_Texture *turnTexture;
    SDL_Texture *appleTexture;
    TTF_Font *font;
    Point snake[SCREEN_WIDTH * SCREEN_HEIGHT / (CELL_SIZE * CELL_SIZE)];
    int snakeLength;
    Point food;
    Direction direction;
    int score;
    bool running;
    GameState gameState;
} SnakeGame;

void initializeGame(SnakeGame *game);
void cleanupGame(SnakeGame *game);
void handleInput(SnakeGame *game);
void update(SnakeGame *game);
void render(SnakeGame *game);
void renderStartScreen(SnakeGame *game);
void renderGameOverScreen(SnakeGame *game);
void handleStartScreenInput(SnakeGame *game);
void handleGameOverScreenInput(SnakeGame *game);
void runGame(SnakeGame *game);

#endif // GAME_H
