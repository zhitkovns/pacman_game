#include "MainMenu.h"

MainMenu::MainMenu(SDL_Renderer* renderer) : BaseMenu(renderer) {
    bg_color = {30, 30, 50, 255};
}

std::string MainMenu::handleClick(int x, int y) {
    SDL_Point point = {x, y};
    for (const auto& button : getButtons()) {
        if (SDL_PointInRect(&point, &button.rect)) {
            return button.action;
        }
    }
    return "";
}