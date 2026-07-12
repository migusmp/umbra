#include "engine.hpp"
#include "../renderer/camera.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

// De momento el GLSL vive aquí mismo, como texto (raw string literal, con
// R"(...)"). Más adelante, cuando tengamos un AssetManager, esto pasará a
// cargarse desde archivos .vert/.frag en assets/shaders/.

static const char *vertexShaderSrc = R"(
#version 450 core
layout (location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    gl_Position = projection * view * model * vec4(aPos, 1.0);
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
    : width(width), height(height), running(false), lastTime(0) {

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

    float aspectRatio = static_cast<float>(width) / static_cast<float>(height);
    // Cámara colocada un poco atrás en Z, mirando hacia el origen (donde
    // está el triángulo) por la dirección por defecto (-Z).
    camera = std::make_unique<Camera>(glm::vec3(0.0f, 0.0f, 6.0f), aspectRatio);

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

    // Model: de momento el triángulo se queda quieto en el origen, sin
    // rotación ni escala — matriz identidad.
    glm::mat4 model = glm::mat4(1.0f);

    shader->setMat4("model", model);
    shader->setMat4("view", camera->getViewMatrix());
    shader->setMat4("projection", camera->getProjectionMatrix());

    triangle->draw();

    SDL_GL_SwapWindow(window->getHandle());
}
