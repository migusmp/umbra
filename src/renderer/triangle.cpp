#include "triangle.hpp"

Triangle::Triangle() {
    // Tres vértices, cada uno con posición (x, y, z).
    // Coordenadas en "clip space": van de -1 a 1 en cada eje, sin importar
    // el tamaño real de la ventana en píxeles. (0,0) es el centro.
    float vertices[] = {
        //  x,     y,    z
        0.0f,  0.5f,  0.0f, // arriba
        -0.5f, -0.5f, 0.0f, // abajo-izquierda
        0.5f,  -0.5f, 0.0f, // abajo-derecha
    };

    // 1. Genera un VBO y sube los datos en bruto a la GPU
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // 2. Genera un VAO: la "receta" de cómo leer ese VBO
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // Vuelve a bindear el VBO mientras el VAO está activo, para que quede
    // asociado a esta configuración de lectura.
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    // Atributo 0 = posición. 3 floats por vértice, sin normalizar,
    // sin espacio extra entre vértices (stride = 3 floats), empieza en 0.
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float),
                          (void *)0);
    glEnableVertexAttribArray(0);

    // Desbindear para no arrastrar este estado por accidente a otro sitio
    glBindVertexArray(0);
}

Triangle::~Triangle() {
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
}

void Triangle::draw() const {
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);
}
