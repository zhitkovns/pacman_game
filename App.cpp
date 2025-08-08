#include "App.h"
#include "MainMenu.h"
#include "SDL2/SDL_ttf.h"
#include "Level.h"
#include <iostream>

App::App() {
    std::cout << "App constructor" << std::endl;
}

App::~App() {
    currentLevel.reset();
    
    if (renderer) {
        SDL_DestroyRenderer(renderer);
        renderer = nullptr;
    }
    
    if (window) {
        SDL_DestroyWindow(window);
        window = nullptr;
    }
    
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
    
    std::cout << "App resources cleaned up" << std::endl;
}

bool App::init() {
    std::cout << "Initializing SDL..." << std::endl;
    
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return false;
    }

    if (TTF_Init() == -1) {
        std::cerr << "TTF_Init Error: " << TTF_GetError() << std::endl;
        SDL_Quit();
        return false;
    }

    if ((IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) != IMG_INIT_PNG) {
        std::cerr << "IMG_Init Error: " << IMG_GetError() << std::endl;
        TTF_Quit();
        SDL_Quit();
        return false;
    }

    std::cout << "Creating window..." << std::endl;
    window = SDL_CreateWindow("Pacman Game", 
                            SDL_WINDOWPOS_CENTERED,
                            SDL_WINDOWPOS_CENTERED,
                            800, 600,
                            SDL_WINDOW_SHOWN);
    if (!window) {
        std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
        IMG_Quit();
        TTF_Quit();
        SDL_Quit();
        return false;
    }

    std::cout << "Creating renderer..." << std::endl;
    renderer = SDL_CreateRenderer(window, -1, 
                                SDL_RENDERER_ACCELERATED | 
                                SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        std::cerr << "SDL_CreateRenderer Error: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        IMG_Quit();
        TTF_Quit();
        SDL_Quit();
        return false;
    }

    std::cout << "Initializing main menu..." << std::endl;
    mainMenu = std::make_unique<MainMenu>(renderer);
    mainMenu->addButton({{300, 100, 200, 50}, "Start Game", "start", "", {100, 200, 100}});
    mainMenu->addButton({{300, 200, 200, 50}, "Exit", "exit", "", {200, 50, 50}});

    return true;
}

void App::startGame() {
    std::cout << "Starting new game..." << std::endl;
    currentLevel = std::make_unique<Level>(renderer);
    if (!currentLevel->loadFromFile("levels/level1.txt")) {
        std::cerr << "Failed to load level!" << std::endl;
        return;
    }
    currentState = State::PLAYING;
}

void App::handleEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            currentState = State::GAME_OVER;
            return;
        }

        switch (currentState) {
            case State::MENU:
                if (event.type == SDL_MOUSEBUTTONDOWN) {
                    int x = event.button.x;
                    int y = event.button.y;
                    std::string action = mainMenu->handleClick(x, y);
                    
                    if (action == "start") startGame();
                    else if (action == "exit") currentState = State::GAME_OVER;
                }
                break;
                
            case State::PLAYING:
                if (auto pacman = currentLevel->getPacman()) {
                    if (event.type == SDL_KEYDOWN) {
                        switch(event.key.keysym.sym) {
                            case SDLK_UP: pacman->setNextDirection(Direction::UP); break;
                            case SDLK_DOWN: pacman->setNextDirection(Direction::DOWN); break;
                            case SDLK_LEFT: pacman->setNextDirection(Direction::LEFT); break;
                            case SDLK_RIGHT: pacman->setNextDirection(Direction::RIGHT); break;
                        }
                    }
                    if (currentLevel->isGameOver()) {
                        currentState = State::GAME_OVER;
                    }
                }
                break;
                
            case State::GAME_OVER:
                if (event.type == SDL_KEYDOWN || event.type == SDL_MOUSEBUTTONDOWN) {
                    currentState = State::MENU;
                    currentLevel.reset();
                }
                break;
        }
    }
}

void App::renderGameOverScreen() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    if (!currentLevel || !currentLevel->getPacman()) return;

    int score = currentLevel->getPacman()->getScore();

    TTF_Font* bigFont = TTF_OpenFont("fonts/arial.ttf", 48);
    if (!bigFont) {
        std::cerr << "Failed to load big font: " << TTF_GetError() << std::endl;
        return;
    }

    SDL_Color white = {255, 255, 255, 255};
    SDL_Color yellow = {255, 255, 0, 255};

    SDL_Surface* gameOverSurface = TTF_RenderText_Solid(bigFont, "GAME OVER", yellow);
    SDL_Texture* gameOverTexture = SDL_CreateTextureFromSurface(renderer, gameOverSurface);
    SDL_Rect gameOverRect = {
        (800 - gameOverSurface->w) / 2,
        200,
        gameOverSurface->w,
        gameOverSurface->h
    };
    SDL_RenderCopy(renderer, gameOverTexture, nullptr, &gameOverRect);

    std::string scoreText = "YOUR SCORE: " + std::to_string(score);
    SDL_Surface* scoreSurface = TTF_RenderText_Solid(bigFont, scoreText.c_str(), white);
    SDL_Texture* scoreTexture = SDL_CreateTextureFromSurface(renderer, scoreSurface);
    SDL_Rect scoreRect = {
        (800 - scoreSurface->w) / 2,
        300,
        scoreSurface->w,
        scoreSurface->h
    };
    SDL_RenderCopy(renderer, scoreTexture, nullptr, &scoreRect);

    TTF_Font* smallFont = TTF_OpenFont("fonts/arial.ttf", 24);
    if (smallFont) {
        SDL_Surface* instructionSurface = TTF_RenderText_Solid(smallFont, "Press any key to return to menu", white);
        SDL_Texture* instructionTexture = SDL_CreateTextureFromSurface(renderer, instructionSurface);
        SDL_Rect instructionRect = {
            (800 - instructionSurface->w) / 2,
            400,
            instructionSurface->w,
            instructionSurface->h
        };
        SDL_RenderCopy(renderer, instructionTexture, nullptr, &instructionRect);
        SDL_FreeSurface(instructionSurface);
        SDL_DestroyTexture(instructionTexture);
        TTF_CloseFont(smallFont);
    }

    SDL_FreeSurface(gameOverSurface);
    SDL_FreeSurface(scoreSurface);
    SDL_DestroyTexture(gameOverTexture);
    SDL_DestroyTexture(scoreTexture);
    TTF_CloseFont(bigFont);
}

void App::update(float deltaTime) {
    if (currentState == State::PLAYING && currentLevel) {
        currentLevel->update(deltaTime);
    }
}

void App::render() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    switch (currentState) {
        case State::MENU:
            mainMenu->render();
            break;
            
        case State::PLAYING:
            if (currentLevel) {
                currentLevel->render();
            }
            break;
            
        case State::GAME_OVER:
            renderGameOverScreen();
            break;
    }

    SDL_RenderPresent(renderer);
}

void App::run() {
    Uint32 lastTime = SDL_GetTicks();
    bool running = true;
    
    while (running) {
        Uint32 currentTime = SDL_GetTicks();
        float deltaTime = (currentTime - lastTime) / 1000.0f;
        lastTime = currentTime;

        handleEvents();
        if (currentState == State::GAME_OVER && !(currentLevel && currentLevel->isGameOver())) {
            running = false;
        }

        update(deltaTime);
        render();
    }
}