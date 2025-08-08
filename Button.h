#pragma once
#include <SDL2/SDL.h>
#include <string>

struct Button {
    SDL_Rect rect;
    std::string text;
    std::string action;
    std::string target;
    SDL_Color color;
    bool isHovered;
    
    Button();
    Button(SDL_Rect r, std::string t, std::string a, std::string tar, SDL_Color c);
    
    bool contains(int x, int y) const;
    void setHovered(bool hover);
};