#include "Button.h"
#include <SDL2/SDL.h>

Button::Button() : rect{0, 0, 0, 0}, text(""), action(""), target(""), color{0, 0, 0}, isHovered(false) {}

Button::Button(SDL_Rect r, std::string t, std::string a, std::string tar, SDL_Color c) 
    : rect(r), text(t), action(a), target(tar), color(c), isHovered(false) {}

bool Button::contains(int x, int y) const {
    return (x >= rect.x && x <= rect.x + rect.w &&
            y >= rect.y && y <= rect.y + rect.h);
}

void Button::setHovered(bool hover) {
    isHovered = hover;
}