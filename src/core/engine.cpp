#include "engine.hpp"

#include <iostream>

Engine::Engine(const std::string& title, int width, int height) : running(false) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "Error initializing SDL: " << SDL_GetError() << std::endl;
        return;
    }

    // Solo ahora, con SDL ya inicializado, creamos la ventana.
    window = std::make_unique<Window>(title, width, height);

    running = true;
}

Engine::~Engine() {
    SDL_Quit();
}

void Engine::run() {
    while (running) {
        processEvents();
        update();
        render();
    }
}

void Engine::processEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            running = false;
        }
    }
}

void Engine::update() {
    // De momento no hay nada que actualizar.
    // Aquí entrará más adelante la lógica del ECS: mover entidades,
    // resolver físicas, etc.
}

void Engine::render() {
    glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    SDL_GL_SwapWindow(window->getHandle());
}
