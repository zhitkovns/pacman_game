cmake_minimum_required(VERSION 3.10)
project(SDLApp)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Поиск необходимых библиотек
find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2 REQUIRED sdl2 SDL2_ttf SDL2_image)

# Список исходных файлов
set(SOURCES
    App.cpp
    BaseMenu.cpp
    Button.cpp
    Characters.cpp
    Level.cpp
    MainMenu.cpp
    main.cpp
)

# Список заголовочных файлов
set(HEADERS
    App.h
    BaseMenu.h
    Button.h
    Characters.h
    Level.h
    MainMenu.h
)

# Создание исполняемого файла
add_executable(app ${SOURCES} ${HEADERS})

# Настройка include-директорий
target_include_directories(app PRIVATE
    ${SDL2_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Настройка линковки
target_link_libraries(app PRIVATE
    ${SDL2_LIBRARIES}
    SDL2_ttf
    SDL2_image
)

# Копирование ресурсов в бинарную директорию
file(COPY sprites DESTINATION ${CMAKE_BINARY_DIR})
file(COPY levels DESTINATION ${CMAKE_BINARY_DIR})
configure_file(menu_config.json ${CMAKE_BINARY_DIR}/menu_config.json COPYONLY)