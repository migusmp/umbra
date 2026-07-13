#pragma once
#include <SDL2/SDL.h>

#include <memory>

#include "../ecs/registry.hpp"
#include "../renderer/camera.hpp"
#include "../renderer/shader.hpp"
#include "../ui/debug_ui.hpp"
#include "window.hpp"

class Engine {
   public:
    Engine(const std::string& title, int width, int height);
    ~Engine();

    void run();

   private:
    void processEvents();
    void update(float dt);
    void render();

    std::unique_ptr<Window> window;
    std::unique_ptr<Shader> shader;
    std::unique_ptr<Camera> camera;
    std::unique_ptr<DebugUI> debugUI;

    Registry registry;
    Entity cubeEntity;

    int width;
    int height;
    bool running;
    Uint64 lastTime;
};
