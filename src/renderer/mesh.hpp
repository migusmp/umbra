#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>

// Un vértice ahora tiene tres datos, no solo posición. Normal y texCoords
// no se usan todavía en el shader, pero assimp los da de fábrica al cargar
// un modelo real, así que se preparan aquí ya para no tener que rehacer
// esta clase cuando lleguemos a iluminación/texturas.
struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoords;
};

class Mesh {
  public:
    Mesh(const std::vector<Vertex> &vertices,
         const std::vector<unsigned int> &indices);
    ~Mesh();

    void draw() const;

  private:
    void setupMesh(const std::vector<Vertex> &vertices,
                   const std::vector<unsigned int> &indices);

    GLuint vao;
    GLuint vbo;
    GLuint ebo;
    size_t indexCount;
};
