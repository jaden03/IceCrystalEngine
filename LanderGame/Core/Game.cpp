#include "Game.h"

#include <iostream>
#include <ostream>

void Game::OnInit()
{
    std::cout << "Hello World!" << std::endl;
}

void Game::OnUpdate(float deltaTime)
{
    
}

void Game::OnFixedUpdate(float fixedDelaTime)
{
    
}


void Game::OnShutdown()
{
    std:: cout << "Bye!" << std::endl;
}


