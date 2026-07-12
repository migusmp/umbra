#pragma once
#include <SDL2/SDL.h>
#include <glad/glad.h>

#include <string>

class Window {
   public:
    Window(const std::string& title, int width, int height);
    ~Window();

    SDL_Window* getHandle() const;
    SDL_GLContext getGLContext() const;

   private:
    int width;
    int height;
    std::string title;
    SDL_Window* window;
    SDL_GLContext glContext;
};
