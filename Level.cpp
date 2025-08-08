#include "Level.h"
#include <fstream>
#include <iostream>

Level::Level(SDL_Renderer* renderer) : renderer(renderer) {
    if (!renderer) throw std::runtime_error("Null renderer");
    font = TTF_OpenFont("fonts/arial.ttf", 24);
    if (!font) {
        std::cerr << "Failed to load font: " << TTF_GetError() << std::endl;
    }
}

Level::~Level() {
    if (font) {
        TTF_CloseFont(font);
        font = nullptr;
    }
    layout.clear();
    game_objects.clear();
    std::cout << "Level destroyed" << std::endl;
}

bool Level::loadFromFile(const std::string& path) {
    layout.clear();
    game_objects.clear();
    pacman.reset();

    std::ifstream file(path);
    if (!file) {
        std::cerr << "Error: Failed to open " << path << "\n";
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        if (!line.empty()) layout.push_back(line);
    }

    bool pacman_created = false;
    for (int y = 0; y < layout.size(); ++y) {
        for (int x = 0; x < layout[y].size(); ++x) {
            const char c = layout[y][x];
            switch (c) {
                case 'P': 
                    if (!pacman_created) {
                        pacman = std::make_unique<Pacman>(x, y, renderer);
                        pacman_created = true;
                        std::cout << "Pacman CREATED at (" << x << "," << y << ")\n";
                    }
                    break;
                    
                case 'G':
                    game_objects.push_back(std::make_unique<Ghost>(x, y, renderer));
                    break;
                    
                case '.':
                    game_objects.push_back(std::make_unique<Dot>(x, y, renderer));
                    break;
                    
                case 'o':
                    game_objects.push_back(std::make_unique<Energizer>(x, y, renderer));
                    break;
            }
        }
    }

    if (!pacman) {
        std::cerr << "Warning: No Pacman in level! Creating default...\n";
        pacman = std::make_unique<Pacman>(1, 1, renderer);
    }

    return true;
}

void Level::update(float deltaTime) {
    if (!pacman || !pacman->getIsActive()) return;

    pacman->update(deltaTime);
    pacman->move(deltaTime, layout);

    // Обработка точек и энерджайзеров
    for (auto it = game_objects.begin(); it != game_objects.end(); ) {
        if (auto dot = dynamic_cast<Dot*>(it->get())) {
            if (pacman->checkCollision(*dot)) {
                pacman->addScore(10);
                dotsEaten++;
                it = game_objects.erase(it);
                
                // Проверка условий появления фруктов
                if ((dotsEaten == 70 && !firstFruitSpawned) || 
                    (dotsEaten == 170 && !secondFruitSpawned)) {
                    spawnFruit();
                    if (dotsEaten == 70) firstFruitSpawned = true;
                    else secondFruitSpawned = true;
                }
                continue;
            }
        }
        else if (auto energizer = dynamic_cast<Energizer*>(it->get())) {
            if (pacman->checkCollision(*energizer)) {
                pacman->addScore(50);
                dotsEaten++;
                for (auto& obj : game_objects) {
                    if (auto ghost = dynamic_cast<Ghost*>(obj.get())) {
                        ghost->setFrightened(true);
                    }
                }
                it = game_objects.erase(it);
                
                if ((dotsEaten == 70 && !firstFruitSpawned) || 
                    (dotsEaten == 170 && !secondFruitSpawned)) {
                    spawnFruit();
                    if (dotsEaten == 70) firstFruitSpawned = true;
                    else secondFruitSpawned = true;
                }
                continue;
            }
        }
        ++it;
    }

    updateFruit(deltaTime);

    // Обработка столкновений с призраком
    for (auto& obj : game_objects) {
        if (auto ghost = dynamic_cast<Ghost*>(obj.get())) {
            ghost->update(deltaTime, pacman.get(), layout);
            
            if (ghost->getIsReleased() && pacman->checkCollision(*ghost) && !ghost->getIsEaten()) {
                if (ghost->getMode() == GhostMode::FRIGHTENED) {
                    ghost->setEaten(true);
                    pacman->addScore(200);
                } else {
                    pacman->loseLives();
                    resetPositions();
                    if (pacman->getLives() <= 0) {
                        gameOverFlag = true;
                    }
                    break;
                }
            }
        }
    }
    bool allGhostsEaten = true;
    bool allDotsEaten = true;
    
    for (auto& obj : game_objects) {
        if (auto ghost = dynamic_cast<Ghost*>(obj.get())) {
            if (!ghost->getIsEaten()) {
                allGhostsEaten = false;
            }
        }
        else if (dynamic_cast<Dot*>(obj.get()) || dynamic_cast<Energizer*>(obj.get())) {
            allDotsEaten = false;
        }
    }

    if (allDotsEaten || allGhostsEaten) {
        restartLevel(true);
    }
}

void Level::renderText(const std::string& text, int x, int y, SDL_Color color) {
    SDL_Surface* surface = TTF_RenderText_Solid(font, text.c_str(), color);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_Rect rect = {x, y, surface->w, surface->h};
    SDL_RenderCopy(renderer, texture, nullptr, &rect);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}

void Level::render() {
    renderMaze();

    for (auto& obj : game_objects) {
        if (dynamic_cast<Dot*>(obj.get()) || dynamic_cast<Energizer*>(obj.get())) {
            obj->render(renderer);
        }
    }

    if (currentFruit) {
        std::cout << "Rendering fruit at (" 
                  << currentFruit->getTileX() << "," 
                  << currentFruit->getTileY() << ")" << std::endl;
        currentFruit->render(renderer);
    }

    if (pacman && pacman->getIsActive()) {
        pacman->render(renderer);
    }

    for (auto& obj : game_objects) {
        if (auto ghost = dynamic_cast<Ghost*>(obj.get())) {
            if (!ghost->getIsEaten() && ghost->getIsActive()) {
                ghost->render(renderer);
            }
        }
    }

    renderEatenFruits();

    int uiX = 24 * 16 + 20;
    int uiY = 50;
    SDL_Color white = {255, 255, 255, 255};
    SDL_Color yellow = {255, 255, 0, 255};

    if (pacman) {
        renderText("Lives:", uiX, uiY, white);
        renderText(std::to_string(pacman->getLives()), uiX + 100, uiY, yellow);
        renderText("Score:", uiX, uiY + 40, white);
        renderText(std::to_string(pacman->getScore()), uiX + 100, uiY + 40, yellow);
    }
}

void Level::renderMaze() const {
    SDL_SetRenderDrawColor(renderer, 33, 33, 255, 255);
    
    for (int y = 0; y < layout.size(); ++y) {
        for (int x = 0; x < layout[y].size(); ++x) {
            if (layout[y][x] == '#') {
                SDL_Rect wall = {x * 16, y * 16, 16, 16};
                SDL_RenderFillRect(renderer, &wall);
            }
        }
    }
}

bool Level::isWall(int x, int y) const {
    if (y >= 0 && y < layout.size() && x >= 0 && x < layout[y].size()) {
        return layout[y][x] == '#';
    }
    return true;
}

void Level::resetPositions() {
    if (!pacman) return;
    
    for (int y = 0; y < layout.size(); ++y) {
        for (int x = 0; x < layout[y].size(); ++x) {
            if (layout[y][x] == 'P') {
                pacman->setPosition(x, y);
                pacman->setIsActive(true);
                break;
            }
        }
    }
    
    for (auto& obj : game_objects) {
        if (auto ghost = dynamic_cast<Ghost*>(obj.get())) {
            ghost->setEaten(false);
            ghost->setIsActive(true);
            ghost->setFrightened(false);
            
            for (int y = 0; y < layout.size(); ++y) {
                for (int x = 0; x < layout[y].size(); ++x) {
                    if (layout[y][x] == 'G') {
                        ghost->setPosition(x, y);
                        break;
                    }
                }
            }
        }
    }
}

void Level::restartLevel(bool keepProgress) {
    int savedLives = pacman ? pacman->getLives() : 3;
    int savedScore = pacman ? pacman->getScore() : 0;
    bool wasPowered = pacman ? pacman->getIsPowered() : false;

    loadFromFile("levels/level1.txt");
    
    if (keepProgress && pacman) {
        pacman->setLives(savedLives);
        pacman->setScore(savedScore);
        pacman->activatePower(wasPowered);
    } else {
        eatenFruits.clear();
    }
    
    dotsEaten = 0;
    firstFruitSpawned = false;
    secondFruitSpawned = false;
    currentFruit.reset();
    
    gameOverFlag = false;
}

void Level::spawnFruit() {
    FruitType type = (rand() % 2 == 0) ? FruitType::ORANGE : FruitType::APPLE;

    int spawnX = layout[0].size() / 2;
    int spawnY = 20;
    
    // Проверка границ
    if (spawnY >= layout.size()) {
        spawnY = layout.size() - 2;
    }
    
    currentFruit = std::make_unique<Fruit>(spawnX, spawnY, type, renderer);
    fruitTimer = 9.0f;
}

void Level::updateFruit(float deltaTime) {
    if (currentFruit) {
        currentFruit->update(deltaTime);
        fruitTimer -= deltaTime;
        
        if (fruitTimer <= 0 || !currentFruit->getIsActive()) {
            currentFruit.reset();
        }
        else if (pacman && pacman->checkCollision(*currentFruit)) {
            pacman->addScore(currentFruit->getPoints());
            eatenFruits.push_back(currentFruit->getType());
            
            // Ограничиваем количество отображаемых фруктов (последние 7)
            if (eatenFruits.size() > 7) {
                eatenFruits.erase(eatenFruits.begin());
            }
            
            currentFruit.reset();
        }
    }
}

void Level::renderEatenFruits() const {
    if (eatenFruits.empty()) return;
    
    int startX = 100;
    int y = layout.size() * 16 + 10;
    int spacing = 20;
    
    for (size_t i = 0; i < eatenFruits.size(); ++i) {
        const std::string& texturePath = Fruit::fruitTextures.at(eatenFruits[i]);
        SDL_Texture* texture = IMG_LoadTexture(renderer, texturePath.c_str());
        
        if (texture) {
            SDL_Rect dst = {startX + static_cast<int>(i) * spacing, y, 16, 16};
            SDL_RenderCopy(renderer, texture, nullptr, &dst);
            SDL_DestroyTexture(texture);
        }
    }
}