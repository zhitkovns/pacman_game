#include "App.h"
#include <iostream>

int main(int argc, char* argv[]) {
    std::cout << "Starting application..." << std::endl;
    
    App game;
    if (!game.init()) {
        std::cerr << "Failed to initialize game!" << std::endl;
        return 1;
    }
    
    std::cout << "Entering game loop..." << std::endl;
    game.run();
    
    std::cout << "Application exited normally" << std::endl;
    return 0;
}