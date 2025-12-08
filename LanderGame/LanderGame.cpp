#include <iostream>
#include <ostream>
#include <Ice/Core/Engine.h>
#include "Core/Game.h"

int main(int argc, char* argv[])
{
    Engine engine;
    Game game;
    engine.Run(&game);
    return 0;
}