#pragma once
#include <SDL2/SDL.h>
#include <vector>
#include "Button.h"

class BaseMenu {
protected:
    SDL_Renderer* renderer;
    std::vector<Button> buttons;
    SDL_Color bg_color = {30, 30, 30, 255};
    
public:
    explicit BaseMenu(SDL_Renderer* renderer);
    void addButton(const Button& button);
    const std::vector<Button>& getButtons() const;
    void render();
};