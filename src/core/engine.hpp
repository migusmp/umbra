#pragma once
#include "../renderer/camera.hpp"
#include "../renderer/mesh.hpp"
#include "../renderer/shader.hpp"
#include "window.hpp"
#include <SDL2/SDL.h>
#include <memory>

class Engine {
  public:
    Engine(const std::string &title, int width, int height);
    ~Engine();

    void run();

  private:
    void processEvents();
    void update(float dt);
    void render();

    std::unique_ptr<Window> window;
    std::unique_ptr<Shader> shader;
    std::unique_ptr<Mesh> mesh;
    std::unique_ptr<Camera> camera;

    int width;
    int height;
    bool running;
    Uint64 lastTime;
};
