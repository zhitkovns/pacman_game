#include "Characters.h"
#include <SDL2/SDL_image.h>
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <climits>

// GameObject
GameObject::GameObject(int x, int y) : 
    tileX(x), tileY(y),
    pixelX(x * 16 + 8), pixelY(y * 16 + 8),
    hitbox{pixelX - 8, pixelY - 8, 16, 16},
    isActive(true),
    currentDir(Direction::NONE),
    nextDir(Direction::NONE),
    texture(nullptr) {}

GameObject::~GameObject() {
    if (texture) SDL_DestroyTexture(texture);
}

bool GameObject::canMove(Direction dir, const std::vector<std::string>& levelMap) const {
    if (dir == Direction::NONE) return false;

    // Текущие координаты в тайлах
    int checkX = tileX;
    int checkY = tileY;

    switch(dir) {
        case Direction::UP:    checkY--; break;
        case Direction::DOWN:  checkY++; break;
        case Direction::LEFT:  checkX--; break;
        case Direction::RIGHT: checkX++; break;
    }

    // Обработка туннелей
    if (checkX < 0) checkX = levelMap[0].size() - 1;
    else if (checkX >= levelMap[0].size()) checkX = 0;

    // Проверка вертикальных границ
    if (checkY < 0 || checkY >= levelMap.size()) {
        return false;
    }

    // Проверка стены
    return levelMap[checkY][checkX] != '#';
}

void GameObject::move(float deltaTime, const std::vector<std::string>& levelMap) {
    const float speed = 70.0f * deltaTime;
    
    // Проверяем смену направления только когда объект в центре тайла
    if (nextDir != Direction::NONE && 
        abs(pixelX - (tileX * 16 + 8)) < 2.0f && 
        abs(pixelY - (tileY * 16 + 8)) < 2.0f) {
        if (canMove(nextDir, levelMap)) {
            currentDir = nextDir;
            nextDir = Direction::NONE;
        }
    }

    switch(currentDir) {
        case Direction::UP:
            pixelY -= speed/2;
            if (canMove(Direction::UP, levelMap)) {
                tileY = pixelY / 16;
            } else {
                pixelY = tileY * 16 + 8; 
            }
            break;
            
        case Direction::DOWN:
            pixelY += speed;
            if (canMove(Direction::DOWN, levelMap)) {
                tileY = pixelY / 16;
            } else {
                pixelY = tileY * 16 + 8;
            }
            break;
            
        case Direction::LEFT:
            pixelX -= speed/2;
            if (canMove(Direction::LEFT, levelMap)) {
                if (pixelX < 0) pixelX = (levelMap[0].size() - 1) * 16 + 8; // Туннель
                tileX = pixelX / 16;
            } else {
                pixelX = tileX * 16 + 8;
            }
            break;
            
        case Direction::RIGHT:
            pixelX += speed;
            if (canMove(Direction::RIGHT, levelMap)) {
                if (pixelX >= levelMap[0].size() * 16) pixelX = 8; // Туннель
                tileX = pixelX / 16;
            } else {
                pixelX = tileX * 16 + 8;
            }
            break;
            
        case Direction::NONE:
            break;
    }

    if (abs(pixelX - (tileX * 16 + 8)) >= 16 || abs(pixelY - (tileY * 16 + 8)) >= 16) {
        tileX = static_cast<int>((pixelX) / 16);
        tileY = static_cast<int>((pixelY) / 16);
    }

    hitbox.x = pixelX-8;
    hitbox.y = pixelY-8;
}

void GameObject::setPosition(int tileX, int tileY) {
        this->tileX = tileX;
        this->tileY = tileY;
        this->pixelX = tileX * 16 + 8;
        this->pixelY = tileY * 16 + 8;
        this->hitbox.x = pixelX - 8;
        this->hitbox.y = pixelY - 8;
        this->currentDir = Direction::NONE;
        this->nextDir = Direction::NONE;
}

// Pacman
Pacman::Pacman(int x, int y, SDL_Renderer* renderer) : 
    GameObject(x, y), mouthOpen(false), animTimer(0),
    lives(3), score(0), isPowered(false) {
    textureOpen = IMG_LoadTexture(renderer, "sprites/pacman/1.png");
    textureClosed = IMG_LoadTexture(renderer, "sprites/pacman/2.png");
    if (!textureOpen || !textureClosed) {
        std::cerr << "Failed to load Pacman textures: " << IMG_GetError() << std::endl;
    }
}

Pacman::~Pacman() {
    std::cout << "Pacman destroyed!" << std::endl;
    SDL_DestroyTexture(textureOpen);
    SDL_DestroyTexture(textureClosed);
}

void Pacman::update(float deltaTime) {
    animTimer += deltaTime;
    if (animTimer > 0.2f) {
        animTimer = 0; 
        mouthOpen = !mouthOpen;
    }
}

void Pacman::render(SDL_Renderer* renderer) {
    SDL_Texture* tex = mouthOpen ? textureOpen : textureClosed;
    
    int renderAngle = 180;
    switch(currentDir) {
        case Direction::RIGHT: renderAngle = 180; break;
        case Direction::DOWN:  renderAngle = 270; break;
        case Direction::LEFT:  renderAngle = 0; break;
        case Direction::UP:    renderAngle = 90; break;
        case Direction::NONE:  renderAngle = 180; break;
    }

    SDL_RenderCopyEx(renderer, tex, nullptr, &hitbox, 
                    renderAngle, nullptr, SDL_FLIP_NONE);
}

void Pacman::handleInput(const SDL_Event& event) {
    if (event.type == SDL_KEYDOWN) {
        switch(event.key.keysym.sym) {
            case SDLK_UP: setNextDirection(Direction::UP); break;
            case SDLK_DOWN: setNextDirection(Direction::DOWN); break;
            case SDLK_LEFT: setNextDirection(Direction::LEFT); break;
            case SDLK_RIGHT: setNextDirection(Direction::RIGHT); break;
        }
    }
}

// Ghost
Ghost::Ghost(int x, int y, SDL_Renderer* renderer) : 
    GameObject(x, y), isReleased(false), releaseTimer(0.0f), 
    modeSwitchTimer(0.0f), isInChaseMode(true) 
{
    textureNormal = IMG_LoadTexture(renderer, "sprites/ghosts/b-0.png");
    textureFrightened = IMG_LoadTexture(renderer, "sprites/ghosts/f-0.png");
    
    if (!textureNormal || !textureFrightened) {
        std::cerr << "Failed to load ghost textures: " << IMG_GetError() << std::endl;
    }

    setIsActive(true);
    currentDir = Direction::UP;
    mode = GhostMode::SCATTER;
    modeTimer = 5.0f;
}

Ghost::~Ghost() {
    SDL_DestroyTexture(textureNormal);
    SDL_DestroyTexture(textureFrightened);
}

void Ghost::update(float deltaTime, const Pacman* pacman, std::vector<std::string>& levelMap) {
    if (isEaten) return;

    if (!isReleased) {
        releaseTimer += deltaTime;
        if (releaseTimer >= 5.0f) {
            // Открываем клетку
            if (levelMap.size() > 9 && levelMap[9].size() > 9) {
                levelMap[9][9] = ' ';
            }
            isReleased = true;
            mode = GhostMode::SCATTER;
            modeTimer = 7.0f;
        }
        return;
    }

    if (mode != GhostMode::FRIGHTENED) {
        modeTimer -= deltaTime;
        if (modeTimer <= 0) {
            mode = (mode == GhostMode::CHASE) ? GhostMode::SCATTER : GhostMode::CHASE;
            modeTimer = 5.0f; 

            // Разворачиваем призрака при смене режима
            switch(currentDir) {
                case Direction::UP: currentDir = Direction::DOWN; break;
                case Direction::DOWN: currentDir = Direction::UP; break;
                case Direction::LEFT: currentDir = Direction::RIGHT; break;
                case Direction::RIGHT: currentDir = Direction::LEFT; break;
            }
        }
    }

    // Обработка режима испуга
    if (mode == GhostMode::FRIGHTENED) {
        frightenedTimer -= deltaTime;
        if (frightenedTimer <= 0) {
            setFrightened(false);
            modeTimer = 5.0f;
        }
        if (rand() % 100 < 5) {
            std::vector<Direction> possibleDirs;
            if (canMove(Direction::UP, levelMap)) possibleDirs.push_back(Direction::UP);
            if (canMove(Direction::DOWN, levelMap)) possibleDirs.push_back(Direction::DOWN);
            if (canMove(Direction::LEFT, levelMap)) possibleDirs.push_back(Direction::LEFT);
            if (canMove(Direction::RIGHT, levelMap)) possibleDirs.push_back(Direction::RIGHT);
            
            if (!possibleDirs.empty()) {
                currentDir = possibleDirs[rand() % possibleDirs.size()];
            }
        }
    }
    else {
        updateAI(pacman, levelMap);
    }

    move(deltaTime, levelMap);
}

void Ghost::updateAI(const Pacman* pacman, const std::vector<std::string>& levelMap) {
    if (!pacman) return;

    int targetTileX, targetTileY;
    
    if (mode == GhostMode::CHASE) {
        targetTileX = pacman->getTileX();
        targetTileY = pacman->getTileY();
    } 
    else { // SCATTER 
        targetTileX = 1;
        targetTileY = 1;
    }

    std::vector<Direction> possibleDirs;
    if (canMove(Direction::UP, levelMap)) possibleDirs.push_back(Direction::UP);
    if (canMove(Direction::DOWN, levelMap)) possibleDirs.push_back(Direction::DOWN);
    if (canMove(Direction::LEFT, levelMap)) possibleDirs.push_back(Direction::LEFT);
    if (canMove(Direction::RIGHT, levelMap)) possibleDirs.push_back(Direction::RIGHT);

    // Удаляем разворот на 180° (кроме режима испуга)
    if (mode != GhostMode::FRIGHTENED) {
        possibleDirs.erase(std::remove_if(possibleDirs.begin(), possibleDirs.end(),
            [this](Direction dir) {
                return (dir == Direction::UP && currentDir == Direction::DOWN) ||
                       (dir == Direction::DOWN && currentDir == Direction::UP) ||
                       (dir == Direction::LEFT && currentDir == Direction::RIGHT) ||
                       (dir == Direction::RIGHT && currentDir == Direction::LEFT);
            }), possibleDirs.end());
    }

    if (possibleDirs.empty()) {
        if (!canMove(currentDir, levelMap)) {
            currentDir = Direction::NONE;
        }
        return;
    }

    // Выбираем оптимальное направление
    Direction bestDir = possibleDirs[0];
    int minDistance = INT_MAX;
    
    for (Direction dir : possibleDirs) {
        int newX = tileX;
        int newY = tileY;
        
        switch(dir) {
            case Direction::UP: newY--; break;
            case Direction::DOWN: newY++; break;
            case Direction::LEFT: newX /= 2; newX--; break;
            case Direction::RIGHT: newX++; break;
        }
        
        int dist = abs(newX - targetTileX) + abs(newY - targetTileY);
        
        if (dist < minDistance) {
            minDistance = dist;
            bestDir = dir;
        }
    }
    
    currentDir = bestDir;
}

void Ghost::render(SDL_Renderer* renderer) {
    if (!getIsActive() || isEaten) return;

    SDL_Texture* textureToUse = (mode == GhostMode::FRIGHTENED) ? 
                              textureFrightened : textureNormal;

    SDL_RenderCopyEx(
        renderer, textureToUse, nullptr, &hitbox,
        static_cast<int>(currentDir) * 90, nullptr, SDL_FLIP_NONE
    );
}

void Ghost::changeMode(GhostMode newMode) {
    if (newMode != GhostMode::FRIGHTENED) {
        modeTimer = 0.0f;
    }

    if (newMode != mode && !isEaten) {
        switch(currentDir) {
            case Direction::UP: currentDir = Direction::DOWN; break;
            case Direction::DOWN: currentDir = Direction::UP; break;
            case Direction::LEFT: currentDir = Direction::RIGHT; break;
            case Direction::RIGHT: currentDir = Direction::LEFT; break;
        }
    }
    
    mode = newMode;
}

void Ghost::setFrightened(bool frightened) {
    if (frightened) {
        mode = GhostMode::FRIGHTENED;
        frightenedTimer = 5.0f;
        switch(currentDir) {
            case Direction::UP: currentDir = Direction::DOWN; break;
            case Direction::DOWN: currentDir = Direction::UP; break;
            case Direction::LEFT: currentDir = Direction::RIGHT; break;
            case Direction::RIGHT: currentDir = Direction::LEFT; break;
        }
    } else {
        mode = GhostMode::CHASE;
    }
}

void Ghost::resetToStartPosition() {
        setPosition(tileX, tileY);
        isReleased = false;
        releaseTimer = 0.0f;
        changeMode(GhostMode::SCATTER);
        setIsActive(true);
}

// Dot
Dot::Dot(int x, int y, SDL_Renderer* renderer) : GameObject(x, y) {
    texture = IMG_LoadTexture(renderer, "sprites/map/big-1.png");
    setIsActive(true);
}

Dot::~Dot() {
    SDL_DestroyTexture(texture);
}

void Dot::render(SDL_Renderer* renderer) {
    SDL_RenderCopy(renderer, texture, nullptr, &hitbox);
}

// Energizer
Energizer::Energizer(int x, int y, SDL_Renderer* renderer) : GameObject(x, y) {
    texture = IMG_LoadTexture(renderer, "sprites/map/big-0.png");
    setIsActive(true);
}

Energizer::~Energizer() {
    SDL_DestroyTexture(texture);
}

void Energizer::render(SDL_Renderer* renderer) {
    SDL_RenderCopy(renderer, texture, nullptr, &hitbox);
}

// Fruit
const std::map<FruitType, std::string> Fruit::fruitTextures = {
    {FruitType::ORANGE, "sprites/fruits/orange.png"},
    {FruitType::APPLE, "sprites/fruits/apple.png"}
};

Fruit::Fruit(int x, int y, FruitType type, SDL_Renderer* renderer) : 
    GameObject(x, y), type(type), visibleTime(9.0f) {
    texture = IMG_LoadTexture(renderer, fruitTextures.at(type).c_str());
    if (!texture) {
        std::cerr << "Failed to load fruit texture: " << IMG_GetError() << std::endl;
    }
    setIsActive(true);
}

Fruit::~Fruit() {
    if (texture) {
        SDL_DestroyTexture(texture);
    }
}

void Fruit::update(float deltaTime) {
    visibleTime -= deltaTime;
    if (visibleTime <= 0) {
        setIsActive(false);
    }
}

void Fruit::render(SDL_Renderer* renderer) {
    if (texture) {
        SDL_RenderCopy(renderer, texture, nullptr, &hitbox);
    }
}

int Fruit::getPoints() const {
    switch(type) {
        case FruitType::ORANGE: return 100;
        case FruitType::APPLE:  return 500;
        default: return 0;
    }
}