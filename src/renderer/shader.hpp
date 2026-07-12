#pragma once
#include <glad/glad.h>
#include <string>

class Shader {
  public:
    // Recibe el CÓDIGO FUENTE GLSL como texto (no rutas de archivo, todavía
    // no tenemos un sistema de carga de assets).
    Shader(const std::string &vertexSrc, const std::string &fragmentSrc);
    ~Shader();

    void use() const;

  private:
    GLuint compile(GLenum type, const std::string &source);
    GLuint programId;
};
