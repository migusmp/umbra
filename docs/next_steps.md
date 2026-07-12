# Umbra — Siguiente hito: cargar modelos 3D reales

Estado actual: ventana + contexto OpenGL + triángulo con VBO/VAO/shaders +
sistema completo de matrices (Model/View/Projection) + cámara libre movible
con WASD y ratón.

Este documento cubre el siguiente bloque: pasar de un triángulo hardcodeado
a cargar modelos 3D reales desde archivo, usando `assimp`.

---

## 1. Concepto nuevo: índices (Element Buffer Object / EBO)

### El problema que resuelve

Ahora mismo, `Triangle` guarda 3 vértices en el VBO — uno por cada esquina,
sin repetir nada, porque un triángulo no comparte vértices con nada. Pero en
cualquier modelo real, la mayoría de vértices son **compartidos entre varios
triángulos**.

Ejemplo: un cuadrado se dibuja como dos triángulos. Sin compartir vértices,
tendrías que repetir 2 de las 4 esquinas (6 vértices en el VBO para solo 4
puntos distintos). En un modelo con miles de vértices, esa duplicación
desperdicia memoria de GPU de forma notable — y en modelos reales, cada
vértice suele compartirse entre 4-6 triángulos de media.

### La solución: separar "vértices únicos" de "cómo se conectan"

En vez de repetir vértices, se guardan dos buffers distintos:

- **VBO** — la lista de vértices únicos (posición, normal, UV de cada uno),
  sin repetición.
- **EBO** (Element Buffer Object, a veces llamado *index buffer*) — una
  lista de **números enteros**, donde cada grupo de 3 índices dice "forma
  un triángulo con el vértice número X, el Y, y el Z del VBO".

Para el cuadrado del ejemplo: el VBO tiene solo 4 vértices (las 4 esquinas),
y el EBO tiene 6 índices: `0, 1, 2, 2, 3, 0` — describiendo los dos
triángulos reutilizando las esquinas compartidas.

### Cómo se usa en la práctica

Al dibujar, en vez de `glDrawArrays` (que ya conoces, del triángulo actual —
dibuja los vértices del VBO en orden, tal cual), se usa `glDrawElements`,
que le dice a la GPU: "dibuja usando este EBO como guía de qué vértices
conectar en qué orden". El EBO se asocia al VAO igual que el VBO, así que
una vez configurado, dibujar sigue siendo una sola llamada.

Esto es exactamente lo que asimp te va a dar ya preparado al cargar un
modelo: una lista de vértices únicos + una lista de índices que describe
la conectividad — por eso este concepto hay que entenderlo antes de tocar
la carga de modelos.

---

## 2. Instalar assimp

En Arch, está disponible directamente en los repositorios oficiales:

```bash
sudo pacman -S assimp
```

Esto instala tanto la librería compartida (`libassimp.so`) como los headers
de desarrollo necesarios para compilar contra ella. Comprueba que `pkg-config`
la detecta correctamente:

```bash
pkg-config --cflags --libs assimp
```

Si devuelve algo tipo `-I/usr/include -lassimp`, está lista para usarse.
Como ya haces con SDL2, el `Makefile` se actualizará para incluir estos
flags automáticamente en la compilación y el enlazado.

---

## 3. Roadmap concreto de este hito

1. **Generalizar `Triangle` → `Mesh`**
   Convertir la clase actual, hardcodeada a 3 vértices fijos, en una clase
   genérica que reciba cualquier lista de vértices (con posición y, más
   adelante, normal y UV) más una lista de índices, y construya VBO + VAO +
   EBO a partir de eso. El triángulo actual pasa a ser, simplemente, el caso
   de prueba más simple posible de `Mesh` (3 vértices, 3 índices).

2. **Conseguir un modelo de prueba simple**
   Antes de cargar algo complejo, un cubo o una esfera baja en polígonos en
   formato `.obj` (el formato más simple de parsear y depurar visualmente).
   Se puede exportar uno desde Blender, o descargar un `.obj` de prueba de
   dominio público.

3. **Crear la clase `Model`**
   Usa la API de assimp (`Assimp::Importer`) para leer el archivo, recorrer
   su estructura interna (una escena con nodos y mallas), y por cada malla
   encontrada extraer vértices, normales, UVs e índices — construyendo un
   `Mesh` (del paso 1) por cada una.

4. **Ajustar el vertex shader**
   Añadir un segundo atributo de entrada para la normal de cada vértice
   (`layout (location = 1) in vec3 aNormal`), aunque de momento no se use
   todavía — se necesitará en el siguiente hito (iluminación), y es más
   simple dejarlo ya preparado en el VAO ahora que se está tocando esta
   parte del código.

5. **Cargar el modelo de prueba en `Engine`**
   Sustituir `Triangle` por un `Model` cargado desde el archivo `.obj`, y
   confirmar visualmente que aparece con la forma correcta, navegable con
   la cámara libre que ya funciona.

6. **Verificación**
   - El modelo debería verse con las proporciones correctas comparado con
     abrirlo en un visor externo (Blender, o cualquier visor de `.obj`).
   - Si aparece con partes "hacia dentro" o negras en zonas que deberían
     verse, suele ser un problema de orientación de triángulos (*winding
     order*, `glFrontFace`) o de *culling* — se documentará si aparece.
   - Si el modelo aparece pero deformado o estirado, normalmente es un
     problema de cómo se están leyendo los índices o el stride de los
     atributos en el VAO — revisar contra el `Triangle` actual como
     referencia de qué configuración sí funciona.

---

## Estructura de archivos que se añade

```
umbra/
├── src/
│   └── renderer/
│       ├── shader.hpp / .cpp
│       ├── mesh.hpp / .cpp        ← sustituye a triangle.hpp / .cpp
│       ├── model.hpp / .cpp       ← nuevo
│       └── camera.hpp / .cpp
└── assets/
    └── models/
        └── test_cube.obj           ← modelo de prueba
```

`assets/` se usa por primera vez en este hito — es donde vivirán los
recursos que no son código (modelos, texturas, shaders si algún día se
mueven a archivo en vez de raw strings).
