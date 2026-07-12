#include "shader.hpp"
#include <iostream>
#include <vector>

Shader::Shader(const std::string &vertexSrc, const std::string &fragmentSrc) {
    GLuint vertexShader = compile(GL_VERTEX_SHADER, vertexSrc);
    GLuint fragmentShader = compile(GL_FRAGMENT_SHADER, fragmentSrc);

    programId = glCreateProgram();
    glAttachShader(programId, vertexShader);
    glAttachShader(programId, fragmentShader);
    glLinkProgram(programId);

    // Comprobar errores de LINKADO (distinto de errores de compilación,
    // que ya se comprueban dentro de compile())
    GLint success;
    glGetProgramiv(programId, GL_LINK_STATUS, &success);
    if (!success) {
        GLint logLength;
        glGetProgramiv(programId, GL_INFO_LOG_LENGTH, &logLength);
        std::vector<char> log(logLength);
        glGetProgramInfoLog(programId, logLength, nullptr, log.data());
        std::cerr << "Error linking shader program:\n"
                  << log.data() << std::endl;
        std::exit(1);
    }

    // Una vez linkados dentro del programa, los shaders individuales ya no
    // hacen falta como objetos sueltos — se pueden borrar.
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

Shader::~Shader() { glDeleteProgram(programId); }

GLuint Shader::compile(GLenum type, const std::string &source) {
    GLuint shader = glCreateShader(type);
    const char *src = source.c_str();
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLint logLength;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
        std::vector<char> log(logLength);
        glGetShaderInfoLog(shader, logLength, nullptr, log.data());
        std::string typeName =
            (type == GL_VERTEX_SHADER) ? "vertex" : "fragment";
        std::cerr << "Error compiling " << typeName << " shader:\n"
                  << log.data() << std::endl;
        std::exit(1);
    }

    return shader;
}

void Shader::use() const { glUseProgram(programId); }
