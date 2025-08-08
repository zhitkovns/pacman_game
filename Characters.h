#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <string>
#include <vector>
#include <memory>
#include <iostream>
#include <map>

enum class Direction { UP, RIGHT, DOWN, LEFT, NONE };
enum class GhostMode { CHASE, SCATTER, FRIGHTENED, EATEN };
enum class FruitType { ORANGE, APPLE };

class GameObject {
protected:
    int tileX, tileY;
    int pixelX, pixelY;
    SDL_Rect hitbox;
    bool isActive;
    Direction currentDir;
    Direction nextDir;
    SDL_Texture* texture;

public:
    GameObject(int x, int y);
    virtual ~GameObject();
    virtual void update(float deltaTime) {};
    virtual void render(SDL_Renderer* renderer) = 0;
    bool canMove(Direction dir, const std::vector<std::string>& levelMap) const;
    void move(float deltaTime, const std::vector<std::string>& levelMap);
    SDL_Rect getHitbox() const { return hitbox; }
    bool getIsActive() const { return isActive; }
    void setIsActive(bool active) { isActive = active; }
    Direction getDirection() const { return currentDir; }
    void setNextDirection(Direction dir) { nextDir = dir; }
    bool checkCollision(const GameObject& other) const { return SDL_HasIntersection(&this->hitbox, &other.hitbox); }
    int getTileX() const { return tileX; }
    int getTileY() const { return tileY; }
    int getPixelX() const { return pixelX; }
    int getPixelY() const { return pixelY; }
    void setPosition(int tileX, int tileY);
    
};

class Pacman : public GameObject {
private:
    SDL_Texture* textureOpen;
    SDL_Texture* textureClosed;
    bool mouthOpen;
    float animTimer;
    int lives;
    int score;
    bool isPowered;

public:
    Pacman(int x, int y, SDL_Renderer* renderer);
    ~Pacman() override;
    void update(float deltaTime) override;
    void render(SDL_Renderer* renderer) override;
    void handleInput(const SDL_Event& event);
    void addScore(int points) { score += points; }
    void activatePower(bool active) { isPowered = active; }
    int getLives() const { return lives; }
    int getScore() const { return score; }
    int loseLives() { return lives--; }
    void setLives(int newLives) { lives = newLives; }
    void setScore(int newScore) { score = newScore; }
    bool getIsPowered() const { return isPowered; }
};

class Ghost : public GameObject {
private:
    SDL_Texture* textureNormal;
    SDL_Texture* textureFrightened;
    GhostMode mode = GhostMode::SCATTER;
    float modeTimer = 0.0f;
    bool isReleased = false;
    float releaseTimer = 0.0f;
    int targetX = 0;
    int targetY = 0;
    void updateAI(const Pacman* pacman, const std::vector<std::string>& levelMap);
    float modeSwitchTimer;
    bool isInChaseMode;
    float frightenedTimer = 0;
    static int ghostsEaten;
    bool isEaten = false;

public:
    Ghost(int x, int y, SDL_Renderer* renderer);
    ~Ghost() override;
    void update(float deltaTime, const Pacman* pacman, std::vector<std::string>& levelMap);
    void render(SDL_Renderer* renderer) override;
    GhostMode getMode() const { return mode; }
    void changeMode(GhostMode newMode);
    void resetToStartPosition();
    void setFrightened(bool frightened);
    bool getIsEaten() const { return isEaten; }
    void setEaten(bool eaten) { isEaten = eaten; }
    bool getIsReleased() { return isReleased; }
};

class Dot : public GameObject {
public:
    Dot(int x, int y, SDL_Renderer* renderer);
    ~Dot() override;

    void update(float deltaTime) override {}
    void render(SDL_Renderer* renderer) override;
};

class Energizer : public GameObject {
public:
    Energizer(int x, int y, SDL_Renderer* renderer);
    ~Energizer() override;

    void update(float deltaTime) override {}
    void render(SDL_Renderer* renderer) override;
};

class Fruit : public GameObject {
private:
    FruitType type;
    float visibleTime;
    
public:
    static const std::map<FruitType, std::string> fruitTextures;

    Fruit(int x, int y, FruitType type, SDL_Renderer* renderer);
    ~Fruit() override;
    void update(float deltaTime) override;
    void render(SDL_Renderer* renderer) override;
    int getPoints() const;
    FruitType getType() const { return type; }
};