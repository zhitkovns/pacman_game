#pragma once
#include <vector>
#include <memory>
#include "Characters.h"
#include <SDL2/SDL_ttf.h>

class Level {
private:
    std::vector<std::string> layout;
    std::unique_ptr<Pacman> pacman;
    std::vector<std::unique_ptr<GameObject>> game_objects;
    SDL_Renderer* renderer;
    TTF_Font* font;
    void renderMaze() const;
    bool isWall(int x, int y) const;
    void resetPositions();
    bool gameOverFlag = false;

    int dotsEaten = 0;
    bool firstFruitSpawned = false;
    bool secondFruitSpawned = false;
    std::unique_ptr<Fruit> currentFruit;
    std::vector<FruitType> eatenFruits;
    float fruitTimer = 0.0f;
    
    void spawnFruit();
    void updateFruit(float deltaTime);
    void renderEatenFruits() const;

public:
    Level(SDL_Renderer* renderer);
    ~Level();
    bool loadFromFile(const std::string& path);
    void update(float deltaTime);
    
    void render();
    Pacman* getPacman() { return pacman.get(); }
    const std::vector<std::string>& getMap() const { return layout; }
    void renderText(const std::string& text, int x, int y, SDL_Color color);
    bool isGameOver() const { return gameOverFlag; }
    void restartLevel(bool keepProgress);
};