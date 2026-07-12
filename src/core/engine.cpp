#include "engine.hpp"
#include <iostream>

// De momento el GLSL vive aquí mismo, como texto (raw string literal, con
// R"(...)"). Más adelante, cuando tengamos un AssetManager, esto pasará a
// cargarse desde archivos .vert/.frag en assets/shaders/.

static const char *vertexShaderSrc = R"(
#version 450 core
layout (location = 0) in vec3 aPos;

void main() {
    gl_Position = vec4(aPos, 1.0);
}
)";

static const char *fragmentShaderSrc = R"(
#version 450 core
out vec4 FragColor;

void main() {
    FragColor = vec4(0.9, 0.4, 0.2, 1.0); // naranja
}
)";

Engine::Engine(const std::string &title, int width, int height)
    : running(false), lastTime(0) {

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "Error initializing SDL: " << SDL_GetError() << std::endl;
        std::exit(1);
    }

    window = std::make_unique<Window>(title, width, height);

    // Shader y Triangle se crean AQUÍ, después de que Window ya haya
    // cargado glad — si los creas antes, las funciones de OpenGL que usan
    // por dentro todavía no existirían (mismo problema de orden que ya
    // vimos con SDL_Init).
    shader = std::make_unique<Shader>(vertexShaderSrc, fragmentShaderSrc);
    triangle = std::make_unique<Triangle>();

    lastTime = SDL_GetTicks64();
    running = true;
}

Engine::~Engine() { SDL_Quit(); }

void Engine::run() {
    while (running) {
        Uint64 currentTime = SDL_GetTicks64();
        float dt = (currentTime - lastTime) / 1000.0f;
        lastTime = currentTime;

        processEvents();
        update(dt);
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

void Engine::update(float dt) { (void)dt; }

void Engine::render() {
    glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    shader->use();
    triangle->draw();

    SDL_GL_SwapWindow(window->getHandle());
}
