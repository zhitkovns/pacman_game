#pragma once
#include <SDL2/SDL.h>
#include <memory>

class MainMenu;
class Level;

class App {
public:
    App();
    ~App();
    
    bool init();
    void run();
    
    App(const App&) = delete;
    App& operator=(const App&) = delete;
    
private:
    enum class State { MENU, PLAYING, GAME_OVER };
    
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    std::unique_ptr<MainMenu> mainMenu;
    std::unique_ptr<Level> currentLevel;
    State currentState = State::MENU;
    
    void handleEvents();
    void update(float deltaTime);
    void render();
    void startGame();
    void renderGameOverScreen();
};