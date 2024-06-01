#include "game.h"

int main(int argc, char* argv[]) {
    SnakeGame game;
    initializeGame(&game);
    if (game.running) {
        runGame(&game);
    }
    cleanupGame(&game);
    return 0;
}
