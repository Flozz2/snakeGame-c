#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480
#define CELL_SIZE 20
#define INITIAL_LENGTH 3
#define BASE_DELAY_MS 200
#define FONT_SIZE 24

typedef struct {
    int x, y;
} Point;

typedef enum {
    UP, DOWN, LEFT, RIGHT
} Direction;

typedef struct {
    SDL_Window* window;
    SDL_Renderer* renderer;
    TTF_Font* font;
    SDL_Texture* backgroundTexture;
    SDL_Texture* headTexture;
    SDL_Texture* bodyTexture;
    SDL_Texture* tailTexture;
    SDL_Texture* appleTexture;
    Direction direction;
    Point snake[SCREEN_WIDTH * SCREEN_HEIGHT / CELL_SIZE];
    Point food;
    int snakeLength;
    bool running;
    int score;
} SnakeGame;

SDL_Texture* loadTexture(SnakeGame *game, const char* path) {
    SDL_Surface* surface = IMG_Load(path);
    if (!surface) {
        printf("Unable to load image %s! SDL_image Error: %s\n", path, IMG_GetError());
        return NULL;
    }
    SDL_Texture* texture = SDL_CreateTextureFromSurface(game->renderer, surface);
    SDL_FreeSurface(surface);
    return texture;
}

void generateFood(SnakeGame *game) {
    game->food.x = rand() % (SCREEN_WIDTH / CELL_SIZE) * CELL_SIZE;
    game->food.y = rand() % (SCREEN_HEIGHT / CELL_SIZE) * CELL_SIZE;
    for (int i = 0; i < game->snakeLength; i++){
        if (game->snake[i].x == game->food.x  && game->snake[i].y == game->food.y){
            generateFood(game);
        }

    }
}

bool checkCollision(SnakeGame *game) {
    // Check wall collision
    if (game->snake[0].x < 0)
        game->snake[0].x = SCREEN_WIDTH - CELL_SIZE;
    else if (game->snake[0].x >= SCREEN_WIDTH)
        game->snake[0].x = 0;

    if (game->snake[0].y < 0)
        game->snake[0].y = SCREEN_HEIGHT - CELL_SIZE;
    else if (game->snake[0].y >= SCREEN_HEIGHT)
        game->snake[0].y = 0;

    // Check self collision
    for (int i = 1; i < game->snakeLength; ++i) {
        if (game->snake[0].x == game->snake[i].x && game->snake[0].y == game->snake[i].y) {
            return true;
        }
    }
    return false;
}

void handleInput(SnakeGame *game) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT)
            game->running = false;
        else if (event.type == SDL_KEYDOWN) {
            switch (event.key.keysym.sym) {
                case SDLK_UP:
                    if (game->direction != DOWN)
                        game->direction = UP;
                    break;
                case SDLK_DOWN:
                    if (game->direction != UP)
                        game->direction = DOWN;
                    break;
                case SDLK_LEFT:
                    if (game->direction != RIGHT)
                        game->direction = LEFT;
                    break;
                case SDLK_RIGHT:
                    if (game->direction != LEFT)
                        game->direction = RIGHT;
                    break;
            }
        }
    }
}

void update(SnakeGame *game) {
    Point newHead = game->snake[0];
    switch (game->direction) {
        case UP:
            newHead.y -= CELL_SIZE;
            break;
        case DOWN:
            newHead.y += CELL_SIZE;
            break;
        case LEFT:
            newHead.x -= CELL_SIZE;
            break;
        case RIGHT:
            newHead.x += CELL_SIZE;
            break;
    }

    // Move snake
    for (int i = game->snakeLength - 1; i > 0; --i) {
        game->snake[i] = game->snake[i - 1];
    }
    game->snake[0] = newHead;

    if (newHead.x == game->food.x && newHead.y == game->food.y) {
        // Add a new segment to the snake
        game->snakeLength++;
        // Ensure the new segment doesn't affect the previous segment's position
        game->snake[game->snakeLength - 1] = game->snake[game->snakeLength - 2];
        // Generate new food
        generateFood(game);
        // Increase score
        game->score++;
    }

    if (checkCollision(game)) {
        game->running = false;
    }
}

void renderText(SnakeGame *game, const char *text, int x, int y) {
    SDL_Color color = {255, 255, 255, 255}; // White color
    SDL_Surface* surface = TTF_RenderText_Solid(game->font, text, color);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(game->renderer, surface);

    SDL_Rect destRect;
    destRect.x = x;
    destRect.y = y;
    destRect.w = surface->w;
    destRect.h = surface->h;

    SDL_RenderCopy(game->renderer, texture, NULL, &destRect);

    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}

void render(SnakeGame *game) {
    // Render background
    SDL_RenderCopy(game->renderer, game->backgroundTexture, NULL, NULL);

    // Render snake
    SDL_Rect rect;
    for (int i = 0; i < game->snakeLength; ++i) {
        if (i == 1) {
            SDL_RenderCopy(game->renderer, game->headTexture, NULL, &rect);
        } else if (i == game->snakeLength - 1) {
            SDL_RenderCopy(game->renderer, game->tailTexture, NULL, &rect);
        } else {
            SDL_RenderCopy(game->renderer, game->bodyTexture, NULL, &rect);
        }
        rect = (SDL_Rect) {game->snake[i].x, game->snake[i].y, CELL_SIZE, CELL_SIZE};
    }

    // Render food
    rect = (SDL_Rect) {game->food.x, game->food.y, CELL_SIZE, CELL_SIZE};
    SDL_RenderCopy(game->renderer, game->appleTexture, NULL, &rect);

    // Render score
    char scoreText[50];
    sprintf(scoreText, "Score: %d", game->score);
    renderText(game, scoreText, 10, 10);

    SDL_RenderPresent(game->renderer);
}

void runGame(SnakeGame *game) {
    int delay_ms = BASE_DELAY_MS;
    while (game->running) {
        handleInput(game);
        update(game);
        render(game);
        SDL_Delay(delay_ms);
        // Decrease delay as score increases (makes the game faster)
        delay_ms = BASE_DELAY_MS - game->score * 5;
        if (delay_ms < 50) {
            delay_ms = 50; // Cap minimum delay to prevent game from becoming too fast
        }
    }
}

int SDL_main(int argc, char *argv[]) {
    srand(time(NULL)); // Seed for random number generation
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();
    IMG_Init(IMG_INIT_PNG);

    SnakeGame game;
    game.window = SDL_CreateWindow("Snake Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    game.renderer = SDL_CreateRenderer(game.window, -1, SDL_RENDERER_ACCELERATED);
    game.font = TTF_OpenFont("textures/fonts/OpenSans-Regular.ttf", FONT_SIZE);
    if (!game.font) {
        fprintf(stderr, "Failed to load font: %s\n", TTF_GetError());
        return -1;
    }
    game.backgroundTexture = loadTexture(&game, "textures/background/background.png");
    game.headTexture = loadTexture(&game, "textures/snake/head.png");
    game.bodyTexture = loadTexture(&game, "textures/snake/body.png");
    game.tailTexture = loadTexture(&game, "textures/snake/tail.png");
    game.appleTexture = loadTexture(&game, "textures/apple/apple.png");
    if (!game.backgroundTexture || !game.headTexture || !game.bodyTexture || !game.tailTexture || !game.appleTexture) {
        fprintf(stderr, "Failed to load one or more textures\n");
        return -1;
    }
    game.direction = RIGHT;
    game.snake[0].x = SCREEN_WIDTH / 2;
    game.snake[0].y = SCREEN_HEIGHT / 2;
    game.snakeLength = INITIAL_LENGTH;
    for (int i = 1; i < INITIAL_LENGTH; ++i) {
        game.snake[i].x = game.snake[0].x - i * CELL_SIZE;
        game.snake[i].y = game.snake[0].y;
    }
    generateFood(&game);
    game.running = true;
    game.score = 0;
    runGame(&game);
    TTF_CloseFont(game.font);
    SDL_DestroyTexture(game.backgroundTexture);
    SDL_DestroyTexture(game.headTexture);
    SDL_DestroyTexture(game.bodyTexture);
    SDL_DestroyTexture(game.tailTexture);
    SDL_DestroyTexture(game.appleTexture);
    SDL_DestroyRenderer(game.renderer);
    SDL_DestroyWindow(game.window);
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
    return 0;
}
