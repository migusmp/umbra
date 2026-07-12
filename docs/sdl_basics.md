# SDL2 — Conceptos básicos

Documento de referencia para entender qué hace cada pieza de SDL2 que vamos usando en
`umbra`. Se irá ampliando a medida que avancemos en el roadmap del engine.

---

## 1. ¿Qué es SDL2?

**SDL2** (Simple DirectMedia Layer) es una librería en C que actúa de **capa de
abstracción** entre tu programa y el sistema operativo/hardware. Sin SDL2, para abrir
una ventana tendrías que hablar directamente con Wayland o X11 (APIs muy distintas
entre sí y bastante verbosas). SDL2 te da una única API que funciona igual en Linux,
Windows y macOS, y por debajo traduce tus llamadas al sistema real.

Se encarga de:
- Crear y gestionar ventanas
- Capturar input (teclado, ratón, mando)
- Dar acceso a audio
- Crear el contexto necesario para dibujar (con OpenGL, Vulkan, o su propio renderer 2D)

SDL2 **no dibuja gráficos 3D por sí sola** — no sabe de triángulos ni shaders. Lo que
hace es prepararte el terreno (la ventana + un contexto gráfico) para que luego uses
OpenGL, Vulkan, o su renderer 2D interno para dibujar de verdad.

---

## 2. Inicializar SDL — `SDL_Init`

```cpp
if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    std::cerr << "Error initializing SDL: " << SDL_GetError() << std::endl;
    return 1;
}
```

SDL2 está dividido en **subsistemas** independientes: vídeo, audio, input de mando
(joystick), timers, etc. `SDL_Init` no arranca "todo SDL" de golpe — arranca solo los
subsistemas que le pidas, con flags que se combinan con `|`:

| Flag | Qué activa |
|---|---|
| `SDL_INIT_VIDEO` | Ventanas, superficies, contexto gráfico. Incluye el subsistema de eventos. |
| `SDL_INIT_AUDIO` | Reproducción de sonido |
| `SDL_INIT_JOYSTICK` | Mandos/joysticks |
| `SDL_INIT_TIMER` | Temporizadores de alta precisión |

Ejemplo activando varios: `SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)`.

**¿Por qué comprobamos el valor de retorno?** `SDL_Init` devuelve `0` si todo fue bien,
y un valor negativo si algo falló (por ejemplo, si no hay ningún driver de vídeo
disponible en el sistema). `SDL_GetError()` te da el mensaje de error humano — SDL2
guarda internamente el último error ocurrido, y esa función te lo devuelve como string.

**Al final del programa**, hay que liberar lo que `SDL_Init` reservó:
```cpp
SDL_Quit();
```
Esto cierra todos los subsistemas activos. Si no lo llamas, no crashea nada
inmediatamente, pero es un leak de recursos del sistema (memoria, file descriptors,
conexión con el compositor gráfico) — mala práctica.

---

## 3. La ventana — `SDL_Window`

```cpp
SDL_Window *window = SDL_CreateWindow(
    "First window",              // título
    SDL_WINDOWPOS_CENTERED,      // posición X
    SDL_WINDOWPOS_CENTERED,      // posición Y
    800, 600,                    // ancho, alto
    SDL_WINDOW_SHOWN             // flags
);
```

`SDL_Window` es un **puntero opaco** — no ves ni tocas sus campos internos, solo lo
pasas a otras funciones de SDL que saben qué hacer con él (`SDL_DestroyWindow`,
`SDL_GetWindowSurface`, etc). Es el mismo patrón que verás en casi toda la librería:
`SDL_Renderer*`, `SDL_Texture*`, `SDL_Surface*` — todos son "manejadores" (handles)
hacia recursos que SDL gestiona internamente.

`SDL_CreateWindow` devuelve `nullptr` si falla (por ejemplo, si el sistema no tiene
forma de crear una ventana). **Siempre hay que comprobarlo:**
```cpp
if (!window) {
    std::cerr << "Error creating window: " << SDL_GetError() << std::endl;
    return 1;
}
```

### Flags más usados de `SDL_CreateWindow`

| Flag | Qué hace |
|---|---|
| `SDL_WINDOW_SHOWN` | La ventana es visible al crearse (si no lo pones, se crea oculta) |
| `SDL_WINDOW_OPENGL` | Prepara la ventana para tener un contexto OpenGL asociado — **la necesitaremos en el siguiente paso del roadmap** |
| `SDL_WINDOW_RESIZABLE` | Permite redimensionar la ventana arrastrando el borde |
| `SDL_WINDOW_FULLSCREEN` | Pantalla completa |
| `SDL_WINDOW_BORDERLESS` | Sin barra de título ni bordes |

Se combinan igual que los flags de `SDL_Init`, con `|`: por ejemplo
`SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL`.

**Al final, hay que destruirla:**
```cpp
SDL_DestroyWindow(window);
```

---

## 4. ¿Qué es un `SDL_Surface`?

```cpp
SDL_Surface *surface = SDL_GetWindowSurface(window);
```

Un `SDL_Surface` es literalmente **un bloque de memoria RAM que representa una imagen**:
un array de píxeles con un formato concreto (RGB, RGBA...) y un ancho/alto. No tiene
nada que ver con la GPU — todo el trabajo de "pintar" sobre una surface lo hace la
**CPU**, escribiendo directamente en esa memoria.

`SDL_GetWindowSurface(window)` te da la surface asociada al contenido visible de la
ventana. Cuando escribes píxeles ahí (por ejemplo con `SDL_FillRect`), estás
modificando esa memoria, pero **la pantalla no se actualiza automáticamente** — hace
falta decirle explícitamente a SDL "ya terminé de dibujar, muestra esto":

```cpp
SDL_FillRect(surface, nullptr, SDL_MapRGB(surface->format, 30, 30, 30));
SDL_UpdateWindowSurface(window);
```

- `SDL_FillRect(surface, nullptr, color)` — rellena toda la surface (`nullptr` = sin
  restringir a un rectángulo concreto) de un color.
- `SDL_MapRGB(surface->format, r, g, b)` — convierte un color RGB (0-255 cada canal) al
  formato de píxel exacto que usa esa surface internamente (el formato de píxel puede
  variar según el sistema, por eso no escribes el valor a mano).
- `SDL_UpdateWindowSurface(window)` — copia el contenido de la surface a la pantalla
  real. **Esta llamada es la que hace que Wayland considere que la ventana tiene
  contenido que mostrar** — sin ella, como vimos, la ventana nunca llega a mapearse.

### Por qué esto es un callejón sin salida para un engine 3D

`SDL_Surface` es render por **software** (CPU, píxel a píxel). Es perfecto para
prototipos rapidísimos o herramientas simples, pero:
- No tiene ni idea de qué es un triángulo, una malla, o profundidad (depth buffer)
- No usa la GPU — para un engine 3D sería inutilizablemente lento
- No permite shaders, iluminación, ni nada del pipeline gráfico moderno

Por eso el siguiente paso del roadmap es sustituir esta superficie de software por un
**contexto OpenGL** — ahí es la GPU la que dibuja, y SDL solo se encarga de darte la
ventana y gestionar ese contexto, no de pintar píxeles ella misma.

---

## 5. El loop de eventos — `SDL_Event` y `SDL_PollEvent`

```cpp
bool running = true;
SDL_Event event;

while (running) {
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) running = false;
    }
    SDL_UpdateWindowSurface(window);
}
```

SDL2 mantiene internamente una **cola de eventos** — cada vez que pasa algo (mueves el
ratón, pulsas una tecla, cierras la ventana con la X, redimensionas...), SDL mete un
`SDL_Event` en esa cola.

- `SDL_PollEvent(&event)` — saca **un** evento de la cola y lo guarda en la variable
  `event` que le pasas por referencia. Devuelve `1` si había un evento, `0` si la cola
  está vacía.
- El `while (SDL_PollEvent(&event))` interno vacía **toda** la cola en cada iteración
  del loop principal — si en un frame se acumularon 5 eventos (varias teclas, un
  movimiento de ratón...), los procesas todos antes de pasar al siguiente frame.
- `event.type` te dice qué tipo de evento es. Algunos comunes:

| `event.type` | Qué significa |
|---|---|
| `SDL_QUIT` | El usuario cerró la ventana (la X, Alt+F4, etc.) |
| `SDL_KEYDOWN` / `SDL_KEYUP` | Tecla pulsada / soltada — el código de la tecla está en `event.key.keysym.sym` |
| `SDL_MOUSEMOTION` | El ratón se movió — posición en `event.motion.x/y` |
| `SDL_MOUSEBUTTONDOWN` / `UP` | Click del ratón |
| `SDL_WINDOWEVENT` | Cambios en la ventana (redimensionar, minimizar, foco...) |

`SDL_Event` es internamente una **unión** (`union` de C) — el mismo espacio de memoria
se interpreta como `event.key`, `event.motion`, `event.window`, etc. según lo que diga
`event.type`. Por eso siempre miras `event.type` primero antes de acceder a los campos
específicos.

### Por qué el loop tiene esta forma concreta (bucle dentro de bucle)

- **Loop externo** (`while (running)`) — es el *game loop*: se ejecuta una vez por
  frame, mientras el programa siga vivo.
- **Loop interno** (`while (SDL_PollEvent(...))`) — vacía la cola de eventos completa
  en cada frame. Si usaras un `if` en vez de `while`, solo procesarías un evento por
  frame y la cola se iría acumulando (bug clásico).

---

## 6. Resumen del ciclo de vida típico de un programa SDL2

```
SDL_Init()                    → arranca los subsistemas que necesitas
SDL_CreateWindow()            → crea la ventana (o falla → nullptr)
  [opcional] SDL_CreateRenderer() / SDL_GL_CreateContext()
      → según si vas a dibujar con el renderer 2D de SDL o con OpenGL/Vulkan

while (running) {
    procesar eventos           → SDL_PollEvent en bucle
    actualizar estado          → lógica del juego/engine
    dibujar el frame           → FillRect+UpdateWindowSurface, o draw calls de OpenGL
    presentar el frame         → SDL_UpdateWindowSurface() o SDL_GL_SwapBuffers()
}

SDL_DestroyWindow()           → libera la ventana
SDL_Quit()                    → cierra los subsistemas
```

La regla de oro: **todo lo que SDL te devuelve como puntero (`SDL_Window*`,
`SDL_Surface*`, `SDL_Renderer*`, `SDL_GLContext`) hay que liberarlo explícitamente**
al terminar, con su función `SDL_Destroy*` o `SDL_*_Free` correspondiente. SDL no tiene
recolector de basura — si no liberas, hay leak.

---

## Próximo capítulo: contexto OpenGL

Cuando pasemos del `SDL_Surface` (software) a `SDL_GLContext` (GPU), aparecerán
conceptos nuevos que documentaremos aquí:
- `SDL_GL_SetAttribute` — configurar qué versión de OpenGL pides antes de crear el contexto
- `SDL_GL_CreateContext` — el equivalente al "contexto gráfico" que la GPU usa para dibujar
- `SDL_GL_SwapBuffers` — el sustituto de `SDL_UpdateWindowSurface` cuando usas OpenGL
- Qué es *double buffering* y por qué existe
