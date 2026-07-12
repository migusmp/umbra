#pragma once
#include <SDL2/SDL.h>

#include <memory>

#include "window.hpp"

class Engine {
   public:
    Engine(const std::string& title, int width, int height);
    ~Engine();

    void run();

   private:
    void processEvents();
    void update();
    void render();

    // unique_ptr en vez de Window directo: así controlamos exactamente CUÁNDO
    // se construye (después de SDL_Init), en vez de que C++ lo haga
    // automáticamente antes del cuerpo del constructor.
    std::unique_ptr<Window> window;
    bool running;
};
