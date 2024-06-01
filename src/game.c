#include "game.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h> 
#include <pthread.h>

SDL_Texture *loadTexture(SnakeGame *game, const char *path) {
    SDL_Surface *surface = IMG_Load(path);
    if (!surface) {
        printf("Unable to load image %s! SDL_image Error: %s\n", path, IMG_GetError());
        return NULL;
    }
    SDL_Texture *texture = SDL_CreateTextureFromSurface(game->renderer, surface);
    SDL_FreeSurface(surface);
    return texture;
}

void generateFood(SnakeGame *game) {
    game->food.x = rand() % (SCREEN_WIDTH / CELL_SIZE) * CELL_SIZE;
    game->food.y = rand() % (SCREEN_HEIGHT / CELL_SIZE) * CELL_SIZE;
    for (int i = 0; i < game->snakeLength; i++) {
        if (game->snake[i].x == game->food.x && game->snake[i].y == game->food.y) {
            generateFood(game);
        }
    }
}

void *playSoundEffect(void *arg) {
    const char *path = (const char *)arg;
    Mix_Chunk *sound = Mix_LoadWAV(path);
    if (!sound) {
        printf("Failed to load sound! SDL_mixer Error: %s\n", Mix_GetError());
        return NULL;
    }
    Mix_PlayChannel(-1, sound, 0);
    while (Mix_Playing(-1) != 0) {
        SDL_Delay(100);
    }
    Mix_FreeChunk(sound);
    return NULL;
}

void playSound(const char *path) {
    pthread_t soundThread;
    pthread_create(&soundThread, NULL, playSoundEffect, (void *)path);
    pthread_detach(soundThread);
}

bool checkCollision(SnakeGame *game) {
    if (game->snake[0].x < 0 || game->snake[0].x >= SCREEN_WIDTH ||
        game->snake[0].y < 0 || game->snake[0].y >= SCREEN_HEIGHT) {
        return true;
    }
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
        if (event.type == SDL_QUIT) {
            game->running = false;
        } else if (event.type == SDL_KEYDOWN) {
            switch (event.key.keysym.sym) {
                case SDLK_UP:
                    if (game->direction != DOWN) game->direction = UP;
                    break;
                case SDLK_DOWN:
                    if (game->direction != UP) game->direction = DOWN;
                    break;
                case SDLK_LEFT:
                    if (game->direction != RIGHT) game->direction = LEFT;
                    break;
                case SDLK_RIGHT:
                    if (game->direction != LEFT) game->direction = RIGHT;
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
    if (newHead.x < 0) newHead.x = SCREEN_WIDTH - CELL_SIZE;
    else if (newHead.x >= SCREEN_WIDTH) newHead.x = 0;
    if (newHead.y < 0) newHead.y = SCREEN_HEIGHT - CELL_SIZE;
    else if (newHead.y >= SCREEN_HEIGHT) newHead.y = 0;
    if (newHead.x == game->food.x && newHead.y == game->food.y) {
        game->snakeLength++;
        generateFood(game);
        game->score++;
        playSound("assets/default_textures/sounds/apple_eat.wav");
    }
    for (int i = game->snakeLength - 1; i > 0; --i) {
        game->snake[i] = game->snake[i - 1];
    }
    game->snake[0] = newHead;
    if (checkCollision(game)) {
        game->gameState = GAME_OVER;
    }
}

void renderText(SnakeGame *game, const char *text, int x, int y) {
    SDL_Color color = {255, 255, 255, 255};
    SDL_Surface *surface = TTF_RenderText_Solid(game->font, text, color);
    SDL_Texture *texture = SDL_CreateTextureFromSurface(game->renderer, surface);
    SDL_Rect destRect = {x, y, surface->w, surface->h};
    SDL_RenderCopy(game->renderer, texture, NULL, &destRect);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}

void render(SnakeGame *game) 
{   
    SDL_RenderClear(game->renderer);
    // Render background
    SDL_RenderCopy(game->renderer, game->backgroundTexture, NULL, NULL);

    // Render snake
    SDL_Rect rect;
    double angle = 0.0;
    SDL_RendererFlip flip = SDL_FLIP_NONE;

    for (int i = 0; i < game->snakeLength; ++i)
    {
        rect = (SDL_Rect){game->snake[i].x, game->snake[i].y, CELL_SIZE, CELL_SIZE};
        if (i == 0)
        {
            // Determine the rotation angle for the head based on the direction
            double angle;
            switch (game->direction)
            {
            case UP:
                angle = 270.0;
                break;
            case DOWN:
                angle = 90;
                break;
            case RIGHT:
                angle = 0.0;
                break;
            case LEFT:
                angle = 180.0;
                break;
            default:
                angle = 0.0;
                break;
            }
            SDL_RenderCopyEx(game->renderer, game->headTexture, NULL, &rect, angle, NULL, SDL_FLIP_NONE);
        }
        else if (i == game->snakeLength - 1)
        {
            // Determine the rotation angle for the tail based on the direction
            Point prevSegment = game->snake[i - 1];
            // Edge cases
            if ((prevSegment.x == (SCREEN_WIDTH - CELL_SIZE)) && (game->snake[i].x == 0))
            {
                angle = 180.0;
            }
            else if ((prevSegment.x == 0) && (game->snake[i].x == (SCREEN_WIDTH - CELL_SIZE)))
            {
                angle = 0.0;
            }
            else if ((prevSegment.y == (SCREEN_HEIGHT - CELL_SIZE)) && (game->snake[i].y == 0))
            {
                angle = 270.0;
            }
            else if ((prevSegment.y == 0) && (game->snake[i].y == (SCREEN_HEIGHT - CELL_SIZE)))
            {
                angle = 90.0;
            }
            // Normal cases
            else if ((prevSegment.x < game->snake[i].x))
            {
                angle = 180.0; // Tail pointing left
            }
            else if ((prevSegment.x > game->snake[i].x))
            {
                angle = 0.0; // Tail pointing right
            }
            else if ((prevSegment.y < game->snake[i].y))
            {
                angle = 270.0; // Tail pointing up
            }
            else if ((prevSegment.y > game->snake[i].y))
            {
                angle = 90.0; // Tail pointing down
            }
            SDL_RenderCopyEx(game->renderer, game->tailTexture, NULL, &rect, angle, NULL, SDL_FLIP_NONE);
        }
        else
        {
            // Determine if this segment is turning
            Point prevSegment = game->snake[i - 1];
            Point nextSegment = game->snake[i + 1];
            if ((prevSegment.x != nextSegment.x) && (prevSegment.y != nextSegment.y))
            {
                // This segment is turning
                if (prevSegment.y < game->snake[i].y && nextSegment.x < game->snake[i].x) // Right to Up
                {
                    angle = 270.0;
                    flip = SDL_FLIP_VERTICAL;
                }
                else if (prevSegment.y > game->snake[i].y && nextSegment.x < game->snake[i].x) // Right to Down
                {
                    angle = 90.0;
                    flip = SDL_FLIP_NONE;
                }
                else if (prevSegment.y < game->snake[i].y && nextSegment.x > game->snake[i].x) // Left to Up
                {
                    angle = 270.0;
                    flip = SDL_FLIP_NONE;
                }
                else if (prevSegment.y > game->snake[i].y && nextSegment.x > game->snake[i].x) // Left to Down
                {
                    angle = 90.0;
                    flip = SDL_FLIP_VERTICAL;
                }
                else if (prevSegment.x < game->snake[i].x && nextSegment.y < game->snake[i].y) // Bottom to Left
                {
                    angle = 180.0;
                    flip = SDL_FLIP_NONE;
                }
                else if (prevSegment.x < game->snake[i].x && nextSegment.y > game->snake[i].y) // Bottom to Right
                {
                    angle = 180.0;
                    flip = SDL_FLIP_VERTICAL;
                }
                else if (prevSegment.x > game->snake[i].x && nextSegment.y < game->snake[i].y) // Up to Left
                {
                    angle = 0.0;
                    flip = SDL_FLIP_VERTICAL;
                }
                else if (prevSegment.x > game->snake[i].x && nextSegment.y > game->snake[i].y) // Up to right
                {
                    angle = 0.0;
                    flip = SDL_FLIP_NONE;
                }
                SDL_RenderCopyEx(game->renderer, game->turnTexture, NULL, &rect, angle, NULL, flip);
            }
            else
            {
                // Determine the rotation angle for the body segment
                if (prevSegment.x != game->snake[i].x)
                {
                    angle = 0.0; // Body horizontal
                }
                else if (prevSegment.y != game->snake[i].y)
                {
                    angle = 90.0; // Body vertical
                }
                SDL_RenderCopyEx(game->renderer, game->bodyTexture, NULL, &rect, angle, NULL, SDL_FLIP_NONE);
            }
        }
    }

    // Render food
    rect = (SDL_Rect){game->food.x, game->food.y, CELL_SIZE, CELL_SIZE};
    SDL_RenderCopy(game->renderer, game->appleTexture, NULL, &rect);

    // Render score
    char scoreText[50];
    sprintf(scoreText, "Score: %d", game->score);
    renderText(game, scoreText, 10, 10);

    SDL_RenderPresent(game->renderer);
}



void renderStartScreen(SnakeGame *game) {
    SDL_RenderClear(game->renderer);
    renderText(game, "Press Enter to Start", SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2);
    SDL_RenderPresent(game->renderer);
}

void renderGameOverScreen(SnakeGame *game) {
    SDL_RenderClear(game->renderer);
    char gameOverText[50];
    sprintf(gameOverText, "Game Over! Score: %d", game->score);
    renderText(game, gameOverText, SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 - 50);
    renderText(game, "Press Enter to Restart", SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2);
    SDL_RenderPresent(game->renderer);
}

void handleStartScreenInput(SnakeGame *game) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            game->running = false;
        } else if (event.type == SDL_KEYDOWN) {
            if (event.key.keysym.sym == SDLK_RETURN) {
                game->gameState = GAME_RUNNING;
            }
        }
    }
}

void handleGameOverScreenInput(SnakeGame *game) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            game->running = false;
        } else if (event.type == SDL_KEYDOWN) {
            if (event.key.keysym.sym == SDLK_RETURN) {
                game->gameState = START_SCREEN;
                game->score = 0;
                game->snakeLength = INITIAL_LENGTH;
                game->direction = RIGHT;
                for (int i = 0; i < INITIAL_LENGTH; i++) {
                    game->snake[i] = (Point){(INITIAL_LENGTH - i - 1) * CELL_SIZE, 0};
                }
            }
        }
    }
}

void runGame(SnakeGame *game) {
    while (game->running) {
        switch (game->gameState) {
            case START_SCREEN:
                handleStartScreenInput(game);
                renderStartScreen(game);
                break;
            case GAME_RUNNING:
                handleInput(game);
                update(game);
                render(game);
                break;
            case GAME_OVER:
                handleGameOverScreenInput(game);
                renderGameOverScreen(game);
                break;
        }
        SDL_Delay(BASE_DELAY_MS);
    }
}

void initializeGame(SnakeGame *game) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
        game->running = false;
        return;
    }
    game->window = SDL_CreateWindow("Snake", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!game->window) {
        printf("Window could not be created! SDL Error: %s\n", SDL_GetError());
        game->running = false;
        return;
    }
    game->renderer = SDL_CreateRenderer(game->window, -1, SDL_RENDERER_ACCELERATED);
    if (!game->renderer) {
        printf("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
        game->running = false;
        return;
    }
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
        game->running = false;
        return;
    }
    if (TTF_Init() == -1) {
        printf("SDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError());
        game->running = false;
        return;
    }
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        printf("SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError());
        game->running = false;
        return;
    }
    game->backgroundTexture = loadTexture(game, "assets/default_textures/background/background.png");
    game->headTexture = loadTexture(game, "assets/default_textures/snake/head.png");
    game->bodyTexture = loadTexture(game, "assets/default_textures/snake/body.png");
    game->tailTexture = loadTexture(game, "assets/default_textures/snake/tail.png");
    game->turnTexture = loadTexture(game, "assets/default_textures/snake/turn.png");
    game->appleTexture = loadTexture(game, "assets/default_textures/apple/apple.png");
    game->font = TTF_OpenFont("assets/default_textures/fonts/OpenSans-Regular.ttf", FONT_SIZE);
    if (!game->font) {
        printf("Failed to load font! SDL_ttf Error: %s\n", TTF_GetError());
        game->running = false;
        return;
    }
    game->running = true;
    game->gameState = START_SCREEN;
    game->score = 0;
    game->snakeLength = INITIAL_LENGTH;
    game->direction = RIGHT;
    for (int i = 0; i < INITIAL_LENGTH; i++) {
        game->snake[i] = (Point){(INITIAL_LENGTH - i - 1) * CELL_SIZE, 0};
    }
    srand((unsigned int)time(NULL));
    generateFood(game);
}

void cleanupGame(SnakeGame *game) {
    SDL_DestroyTexture(game->backgroundTexture);
    SDL_DestroyTexture(game->headTexture);
    SDL_DestroyTexture(game->bodyTexture);
    SDL_DestroyTexture(game->tailTexture);
    SDL_DestroyTexture(game->turnTexture);
    SDL_DestroyTexture(game->appleTexture);
    TTF_CloseFont(game->font);
    SDL_DestroyRenderer(game->renderer);
    SDL_DestroyWindow(game->window);
    Mix_Quit();
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}
