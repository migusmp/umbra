```
██╗   ██╗███╗   ███╗██████╗ ██████╗  █████╗
██║   ██║████╗ ████║██╔══██╗██╔══██╗██╔══██╗
██║   ██║██╔████╔██║██████╔╥██████╔╥███████║
██║   ██║██║╚██╔╝██║██╔══██╗██╔══██╗██╔══██║
╚██████╔╝██║ ╚═╝ ██║██████╔╝██║  ██║██║  ██║
 ╚═════╝ ╚═╝     ╚═╝╚═════╝ ╚═╝  ╚═╝╚═╝  ╚═╝
```

A **C++20** OpenGL 4.5 Core game engine. Currently a triangle in a window — soon, so much more.

---

## Current State

| Layer | Component | Status |
|---|---|---|
| 🪟 **Window** | SDL2 window + OpenGL 4.5 Core context | ✅ Done |
| 🔺 **Renderer** | Vertex/Fragment shader compilation & linking | ✅ Done |
| 🔺 **Renderer** | Triangle rendering (VAO/VBO, clip space) | ✅ Done |
| 🔄 **Engine** | Game loop (processEvents → update → render) | ✅ Done |
| 🏗️ **Build** | Makefile, C++20, `pkg-config` deps | ✅ Done |

### What it does right now

```
main()
  └─ Engine("Umbra", 800, 600)
       ├─ SDL_Init → Window → GL context → glad
       ├─ Shader (inline GLSL 450 core)
       └─ Triangle (clip-space VBO + VAO)
       └─ run()
            └─ loop { events → update → render }
                 └─ render: clear → shader→use() → triangle→draw()
```

One orange triangle on a dark grey background. Close the window, it quits clean — zero leaks, RAII end-to-end.

---

## Project Structure

```
src/
├── main.cpp                 # Entry point
├── core/
│   ├── engine.hpp/.cpp      # Game loop, lifecycle orchestration
│   └── window.hpp/.cpp      # SDL2 window + OpenGL context
└── renderer/
    ├── shader.hpp/.cpp      # GLSL compile/link (inline sources)
    └── triangle.hpp/.cpp    # VAO/VBO setup, draw call
vendor/
└── glad/                    # OpenGL 4.5 Core loader (generated)
build/                       # Object files (gitignored)
docs/
├── sdl_basics.md            # SDL2 primer
└── next_steps.md            # Hito 2 plan
```

---

## Build & Run

```bash
# Build
make

# Build and run
make run

# Clean
make clean
```

Requires: `g++`, `SDL2`, `libdl`.

---

## Next Steps

### 🔜 Hito 2 — 3D Camera + Matrices

- [ ] Install GLM (header-only math lib)
- [ ] MVP matrices (Model-View-Projection) as shader uniforms
- [ ] `setMat4()` on the Shader class
- [ ] Camera class (position → lookAt → projection)
- [ ] Full 3D transforms — triangle becomes a spinning cube

### 🔮 Future Horizons

- [ ] ECS architecture
- [ ] Model loading (assimp?)
- [ ] Lighting & materials
- [ ] Editor layer
- [ ] ??

---

*Crafted with C++20, SDL2, OpenGL 4.5, and too much coffee.*
