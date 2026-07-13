```
██╗   ██╗███╗   ███╗██████╗ ██████╗  █████╗
██║   ██║████╗ ████║██╔══██╗██╔══██╗██╔══██╗
██║   ██║██╔████╔██║██████╔╥██████╔╥███████║
██║   ██║██║╚██╔╝██║██╔══██╗██╔══██╗██╔══██║
╚██████╔╝██║ ╚═╝ ██║██████╔╝██║  ██║██║  ██║
 ╚═════╝ ╚═╝     ╚═╝╚═════╝ ╚═╝  ╚═╝╚═╝  ╚═╝
```

A **C++20** OpenGL 4.5 Core game engine. From scratch, step by step — no frameworks, no shortcuts.

---

## Current State

| Layer | Component | Status |
|---|---|---|
| 🪟 **Window** | SDL2 window + OpenGL 4.5 Core context | ✅ Done |
| 🎮 **Input** | Keyboard (WASD + Space/Shift) + Mouse (pitch/yaw) | ✅ Done |
| 📐 **Camera** | 3D FPS camera (yaw/pitch, lookAt, perspective) | ✅ Done |
| 🔺 **Mesh** | Generic VAO/VBO/EBO with move semantics | ✅ Done |
| 📦 **Model** | Assimp .obj loader (recursive scene traversal) | ✅ Done |
| 💡 **Lighting** | Phong model (ambient + diffuse + specular) | ✅ Done |
| 🔄 **Engine** | Game loop (processEvents → update → render) with dt | ✅ Done |
| 🏗️ **Build** | Makefile, C++20, `pkg-config` deps | ✅ Done |

### What it does right now

```
main()
  └─ Engine("Umbra", 800, 600)
       ├─ Window (SDL2 + GL 4.5 Core context + depth test)
       ├─ Shader (inline GLSL 450 core)
       ├─ Model("assets/models/test_cube.obj")  ← Assimp
       └─ Camera(position, yaw, pitch, aspect)
       └─ run()
            └─ loop { events → update(dt) → render }
                 └─ render: clear → shader→use()
                      ├─ MVP uniforms (model/view/projection)
                      ├─ Normal matrix (transpose(inverse(model)))
                      ├─ Lighting uniforms (Phong: ambient + diffuse + specular)
                      └─ model→draw() → mesh→draw() → glDrawElements
```

Navigate a lit 3D cube in real time. WASD to move, mouse to look around, Space/Shift to go up/down. Close the window, it quits clean — zero leaks, RAII end-to-end.

---

## Controls

| Key | Action |
|---|---|
| W / S | Forward / Backward |
| A / D | Strafe left / right |
| Space | Move up (Y axis) |
| Shift (left) | Move down (Y axis) |
| Mouse | Look around (pitch/yaw) |
| Escape | Quit |

---

## Project Structure

```
src/
├── main.cpp                   # Entry point
├── core/
│   ├── engine.hpp/.cpp        # Game loop, input processing, uniforms
│   └── window.hpp/.cpp        # SDL2 window + OpenGL context
└── renderer/
    ├── shader.hpp/.cpp        # GLSL compile/link, setMat4, setVec3
    ├── mesh.hpp/.cpp          # Generic VAO/VBO/EBO, Vertex struct, move-only
    ├── model.hpp/.cpp         # Assimp loader (scene → nodes → meshes)
    └── camera.hpp/.cpp        # 3D FPS camera, view/projection matrices
assets/
└── models/
    └── test_cube.obj          # Test model with normals
vendor/
└── glad/                      # OpenGL 4.5 Core loader (generated)
build/                         # Object files (gitignored)
docs/
├── sdl_basics.md              # SDL2 primer
└── next_steps.md              # Roadmap
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

Requires: `g++`, `SDL2`, `glm`, `assimp`, `libdl`.

---

## Lighting Model (Phong)

All computed per-fragment in world space:

- **Ambient**: `0.15 × lightColor`
- **Diffuse**: `max(dot(normal, lightDir), 0.0) × lightColor`
- **Specular**: `pow(dot(viewDir, reflectDir), 32) × 0.5 × lightColor`

Directional light from `vec3(-0.4, -1.0, -0.3)`. Normal matrix computed as `transpose(inverse(model))` in the vertex shader.

---

## Next Steps

- [ ] Texture mapping
- [ ] Multiple lights (point, spot)
- [ ] ECS architecture
- [ ] Model loading UI (drag & drop?)
- [ ] Lighting & materials editor
- [ ] PBR pipeline
- [ ] Entity/Scene graph
- [ ] Skybox / cubemaps

---

*Crafted with C++20, SDL2, OpenGL 4.5, GLM, Assimp, and too much coffee.*
