#pragma once
#include "BaseMenu.h"

class MainMenu : public BaseMenu {
public:
    MainMenu(SDL_Renderer* renderer);
    std::string handleClick(int x, int y);
};