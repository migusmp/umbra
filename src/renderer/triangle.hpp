#pragma once
#include <glad/glad.h>

class Triangle {
  public:
    Triangle();
    ~Triangle();

    void draw() const;

  private:
    GLuint vao;
    GLuint vbo;
};
