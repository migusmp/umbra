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
    Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices);
    ~Mesh();

    // Mesh posee recursos de GPU (vao/vbo/ebo) — copiarla literalmente
    // duplicaría esos números sin duplicar el recurso real, y al destruir
    // ambas copias se liberaría el mismo buffer dos veces (double free).
    // Se prohíbe la copia y se permite solo MOVER: transferir la propiedad
    // de un objeto Mesh a otro, dejando el original vacío.
    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;

    Mesh(Mesh&& other) noexcept;
    Mesh& operator=(Mesh&& other) noexcept;

    void draw() const;

   private:
    void setupMesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices);

    GLuint vao;
    GLuint vbo;
    GLuint ebo;
    size_t indexCount;
};
