#include "mesh.hpp"

#include <cstddef>  // offsetof

Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices)
    : indexCount(indices.size()) {
    setupMesh(vertices, indices);
}

Mesh::~Mesh() {
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ebo);
}

Mesh::Mesh(Mesh&& other) noexcept
    : vao(other.vao), vbo(other.vbo), ebo(other.ebo), indexCount(other.indexCount) {
    // Deja al objeto ORIGINAL sin handles válidos (0), para que su
    // destructor no borre los buffers que ahora posee este nuevo objeto.
    other.vao = 0;
    other.vbo = 0;
    other.ebo = 0;
    other.indexCount = 0;
}

Mesh& Mesh::operator=(Mesh&& other) noexcept {
    if (this != &other) {
        // Libera lo que este objeto ya tenía antes de robar lo del otro
        glDeleteVertexArrays(1, &vao);
        glDeleteBuffers(1, &vbo);
        glDeleteBuffers(1, &ebo);

        vao = other.vao;
        vbo = other.vbo;
        ebo = other.ebo;
        indexCount = other.indexCount;

        other.vao = 0;
        other.vbo = 0;
        other.ebo = 0;
        other.indexCount = 0;
    }
    return *this;
}

void Mesh::setupMesh(const std::vector<Vertex>& vertices,
                     const std::vector<unsigned int>& indices) {
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(vao);

    // VBO: los vértices únicos, con todos sus datos (position+normal+UV)
    // seguidos en memoria, struct a struct.
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(),
                 GL_STATIC_DRAW);

    // EBO: la lista de índices que dice cómo conectar esos vértices en
    // triángulos. GL_ELEMENT_ARRAY_BUFFER es el target especial para esto
    // (distinto de GL_ARRAY_BUFFER, que es para datos "por vértice").
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(),
                 GL_STATIC_DRAW);

    // Ahora los atributos ya no son "3 floats seguidos" a pelo como en
    // Triangle — son campos de una struct, así que el stride es
    // sizeof(Vertex) completo, y cada atributo usa offsetof() para saber
    // en qué byte exacto empieza dentro de esa struct.

    // location 0 = posición
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (void*)offsetof(Vertex, position));
    glEnableVertexAttribArray(0);

    // location 1 = normal (todavía no se usa en el shader, pero ya está
    // disponible en el VAO para cuando lleguemos a iluminación)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (void*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(1);

    // location 2 = coordenadas de textura
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (void*)offsetof(Vertex, texCoords));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
}

void Mesh::draw() const {
    glBindVertexArray(vao);
    // glDrawElements en vez de glDrawArrays: dibuja siguiendo el EBO, no
    // el orden bruto del VBO. indexCount es cuántos índices hay en total
    // (3 por triángulo), no cuántos vértices únicos.
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indexCount), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}
