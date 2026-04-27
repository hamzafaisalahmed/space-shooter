#include "Game.h"
#include <iostream>

int main()
{
    try
    {
        Game game;
        game.init();
        game.run();
    }
    catch (const std::exception &ex)
    {
        std::cerr << "Fatal error: " << ex.what() << std::endl;
        return 1;
    }
    return 0;
}
