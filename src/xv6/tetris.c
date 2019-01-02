#include "types.h"
#include "user.h"

int
main(int argc, char** argv)
{
    int score = tetris();
    printf(1, "Tetris score: %d\n", score);
    exit();
}
