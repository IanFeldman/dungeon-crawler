#include <iostream>
#include <conio.h>
#include <Windows.h>
#include "game.h"

int main(int argc, char** argv)
{
    Game game;

    if (game.Initialize()) {
        game.Runloop();
    }

    game.Shutdown();

    return 0;
}