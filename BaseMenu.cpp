#include "BaseMenu.h"
#include <SDL2/SDL_ttf.h>
#include <iostream>

BaseMenu::BaseMenu(SDL_Renderer* renderer) : renderer(renderer) {}

void BaseMenu::addButton(const Button& button) {
    buttons.push_back(button);
}

const std::vector<Button>& BaseMenu::getButtons() const {
    return buttons;
}

void BaseMenu::render() {
    SDL_SetRenderDrawColor(renderer, bg_color.r, bg_color.g, bg_color.b, bg_color.a);
    SDL_RenderFillRect(renderer, nullptr);

    const char* fontPath = "fonts/arial.ttf";

    TTF_Font* font = TTF_OpenFont(fontPath, 24);
    if (!font) {
        std::cerr << "TTF_OpenFont Error: " << TTF_GetError() << std::endl;
        return;
    }

    SDL_Color textColor = {255, 255, 255, 255};

    for (const auto& button : buttons) {
        SDL_SetRenderDrawColor(renderer, button.color.r, button.color.g, button.color.b, 255);
        SDL_RenderFillRect(renderer, &button.rect);

        SDL_Surface* textSurface = TTF_RenderText_Blended(font, button.text.c_str(), textColor);
        if (!textSurface) {
            std::cerr << "TTF_RenderText Error: " << TTF_GetError() << std::endl;
            continue;
        }

        SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
        SDL_FreeSurface(textSurface);

        int textWidth, textHeight;
        SDL_QueryTexture(textTexture, nullptr, nullptr, &textWidth, &textHeight);
        SDL_Rect textRect = {
            button.rect.x + (button.rect.w - textWidth)/2,
            button.rect.y + (button.rect.h - textHeight)/2,
            textWidth,
            textHeight
        };

        SDL_RenderCopy(renderer, textTexture, nullptr, &textRect);
        SDL_DestroyTexture(textTexture);
    }

    TTF_CloseFont(font);
}