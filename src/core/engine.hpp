#pragma once
#include "../renderer/camera.hpp"
#include "../renderer/shader.hpp"
#include "../renderer/triangle.hpp"
#include <SDL2/SDL.h>

#include <memory>

#include "SDL_stdinc.h"
#include "window.hpp"

class Engine {
  public:
    Engine(const std::string &title, int width, int height);
    ~Engine();

    void run();

  private:
    void processEvents();
    void update(float dt);
    void render();

    // unique_ptr en vez de Window directo: así controlamos exactamente CUÁNDO
    // se construye (después de SDL_Init), en vez de que C++ lo haga
    // automáticamente antes del cuerpo del constructor.
    std::unique_ptr<Window> window;
    std::unique_ptr<Shader> shader;
    std::unique_ptr<Triangle> triangle;
    std::unique_ptr<Camera> camera;

    int width;
    int height;
    bool running;
    Uint64 lastTime;
};
