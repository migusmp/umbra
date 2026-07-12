# Umbra — Próximos pasos: matrices y cámara

Estado actual: hito 1 del roadmap completo — ventana + contexto OpenGL +
triángulo dibujado con VBO/VAO/shaders, en clip space fijo (-1 a 1, sin
relación con ninguna cámara).

Este documento cubre el siguiente hito: dotar al motor de las matrices de
transformación y una cámara real, base indispensable antes de cargar modelos
o meter física.

---

## Por qué este paso, antes que cargar modelos

Ahora mismo el triángulo no tiene "posición en el mundo" — sus coordenadas
van directas del VBO a `gl_Position`. Cualquier modelo que carguemos después
se comportaría igual de estático si no resolvemos esto antes: necesitamos el
sistema que traduce "un objeto en un mundo 3D, visto desde una cámara" a las
coordenadas de -1 a 1 que la GPU espera.

---

## Conceptos que entran en este hito

### Las tres matrices del pipeline (Model — View — Projection)

- **Model** — dónde está el objeto en el mundo: posición, rotación, escala.
- **View** — dónde está la cámara y hacia dónde mira. Conceptualmente es
  "mover todo el mundo al revés de como se mueve la cámara".
- **Projection** — cómo se convierte el espacio 3D en algo que cabe en una
  pantalla 2D, con perspectiva (lo lejano se ve más pequeño).

Las tres se multiplican juntas (`projection * view * model * position`) y
ese resultado es lo que calcula el vertex shader para cada vértice, en vez
de usar la posición del VBO directamente.

### GLM — la librería de matemáticas

Aporta los tipos `glm::mat4`, `glm::vec3`, y funciones ya construidas para
generar estas matrices sin escribir el álgebra a mano: `glm::perspective`
(para Projection), `glm::lookAt` (para View), `glm::translate`/`rotate`/
`scale` (para Model).

### Uniforms

Hasta ahora el shader solo recibe datos por vértice (atributos, como la
posición). Las matrices se pasan como **uniforms**: variables globales del
shader, rellenadas desde C++ antes de cada `draw()`, con el mismo valor para
todos los vértices de esa llamada.

### Clase `Camera`

Guarda posición y orientación, y expone un método que devuelve la matriz
View ya calculada. Primero se deja fija para confirmar que las matrices
funcionan; después se le añade input de teclado/ratón para moverla en
tiempo real (estilo cámara libre FPS).

---

## Roadmap concreto de este hito

1. **Instalar GLM** — librería header-only, sin compilación necesaria.
2. **Ampliar `Shader`** para poder enviar uniforms — un método tipo
   `setMat4(nombre, matriz)` que internamente use `glGetUniformLocation` +
   `glUniformMatrix4fv`.
3. **Reescribir el vertex shader** — en vez de `gl_Position = vec4(aPos, 1.0)`,
   declarar los tres uniforms (`model`, `view`, `projection`) y aplicar
   `gl_Position = projection * view * model * vec4(aPos, 1.0)`.
4. **Crear la clase `Camera`** — posición fija de prueba, con un método
   `getViewMatrix()` (usando `glm::lookAt`) y otro `getProjectionMatrix()`
   (usando `glm::perspective`, necesita el aspect ratio de la ventana).
5. **Conectar todo en `Engine::render()`** — antes de `triangle->draw()`,
   calcular las tres matrices y mandarlas al shader vía los uniforms.
6. **Verificación visual** — el triángulo debería seguir viéndose en
   pantalla, pero ahora pasando por una cámara con perspectiva real. Buena
   prueba: cambiar la posición Z de la cámara en el código y comprobar que
   el triángulo se ve más grande/pequeño según la distancia — si eso
   funciona, las matrices están bien encadenadas.
7. **Input de cámara libre** — mover la cámara con teclado (WASD) y
   rotarla con el ratón. Aquí es donde el espacio 3D empieza a "sentirse"
   real, no solo matemáticamente correcto.

---

## Estructura de archivos que se añade

```
umbra/
├── src/
│   ├── core/
│   │   ├── window.hpp / .cpp
│   │   └── engine.hpp / .cpp
│   └── renderer/
│       ├── shader.hpp / .cpp      (se amplía: setMat4, etc.)
│       ├── triangle.hpp / .cpp
│       └── camera.hpp / .cpp      ← nuevo
└── vendor/
    └── glm/                        ← nuevo (header-only)
```

---

## Cómo se verifica el progreso en este hito

- Antes de tocar `Camera`, prueba las matrices con valores fijos hardcodeados
  en `render()` — más fácil de depurar que meter la clase completa a la
  primera.
- Si el triángulo desaparece de la pantalla al añadir las matrices, lo más
  probable es un problema de *near/far plane* en la proyección, o la cámara
  mirando en la dirección equivocada — antes de sospechar del VBO/VAO
  (que ya sabemos que funciona desde el hito anterior).
- Comprueba con `glGetError()` después de las llamadas nuevas
  (`glUniformMatrix4fv`, etc.) si algo no se ve como se espera — un location
  de uniform inválido (-1) no da crash, simplemente no hace nada, y es un
  bug silencioso fácil de perder de vista.
