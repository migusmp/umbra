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
| 🧩 **ECS** | Entity-component registry (sparse array, view<T...>(), shared models) | ✅ Done |
| 🔄 **Engine** | Game loop (processEvents → update → render) with dt | ✅ Done |
| 🏗️ **Build** | Makefile, C++20, `pkg-config` deps | ✅ Done |
| 🖥️ **ImGui** | Debug UI (FPS, entity inspector, camera stats) | ✅ Done |

### What it does right now

```
main()
  └─ Engine("Umbra", 800, 600)
       ├─ Window (SDL2 + GL 4.5 Core context + depth test)
       ├─ Shader (inline GLSL 450 core)
       ├─ DebugUI (ImGui: FPS, entity inspector, camera stats)
       ├─ Registry                     ← ECS
       │   ├─ Entity 0 (TransformComponent + MeshRendererComponent)
       │   │   └─ Model("assets/models/test_cube.obj")  ← shared_ptr
       │   └─ Entity 1 (TransformComponent + MeshRendererComponent)
       │       └─ Model("assets/models/test_cube.obj")  ← shared_ptr (mismo)
       ├─ Camera(position, yaw, pitch, aspect)
       └─ run()
            └─ loop { processEvents → update(dt) → render }
                 ├─ processEvents: SDL_PollEvent → debugUI→processEvent
                 │        └─ mouse motion → camera (solo si ImGui no captura)
                 ├─ update: debugUI→buildUI(registry, camera)
                 │          SDL_SetRelativeMouseMode (toggle según ImGui)
                 │          processKeyboard → camera (solo si ImGui no captura)
                 │          rotation.y += 30°·dt → Entity 0 (ECS)
                 └─ render: clear → shader→use()
                      ├─ uniforms (view, projection, lightDir, lightColor, viewPos)
                      ├─ registry.view<Transform, MeshRenderer>()
                      │   └─ por cada entidad:
                      │        ├─ setMat4("model", transform.getModelMatrix())
                      │        ├─ setVec3("objectColor", ...)
                      │        └─ model→draw()
                      ├─ debugUI→render()
                      └─ SDL_GL_SwapWindow
```

Two cubes in real time — one rotating, one static — both sharing the same model via ECS. WASD to move, mouse to look around, Space/Ctrl to go up/down. Zero leaks, RAII end-to-end.

---

## Controls

| Key | Action |
|---|---|
| W / S | Forward / Backward |
| A / D | Strafe left / right |
| Space | Move up (Y axis) |
| Ctrl (left) | Move down (Y axis) |
| Mouse | Look around (pitch/yaw) |
| Escape | Quit (solo si ImGui no captura el teclado) |

---

## ECS Architecture

Custom header-only ECS in `src/ecs/`:

- **Entity** — `uint32_t` alias. Zero overhead, no base class.
- **ComponentArray\<T\>** — Dense contiguous vector + `Entity → index` map. Swap-and-pop for O(1) removal. Cache-friendly iteration.
- **Registry** — `createEntity()`, `destroyEntity()`, `addComponent<T>()`, `getComponent<T>()`, `view<Ts...>()` (fold-expression filter).
- **Components** — `TransformComponent` (pos/rot/scale + `getModelMatrix()`) and `MeshRendererComponent` (`shared_ptr<Model>`).

```
for (Entity e : registry.view<TransformComponent, MeshRendererComponent>()) {
    auto& t  = registry.getComponent<TransformComponent>(e);
    auto& mr = registry.getComponent<MeshRendererComponent>(e);
    shader->setMat4("model", t.getModelMatrix());
    mr.model->draw();
}
```

Modelo compartido entre entidades via `shared_ptr` — un solo `.obj` en VRAM, N transformaciones distintas.

---

## Project Structure

```
src/
├── main.cpp                   # Entry point
├── core/
│   ├── engine.hpp/.cpp        # Game loop, ECS iteration, uniforms, input
│   └── window.hpp/.cpp        # SDL2 window + OpenGL context
├── ecs/
│   ├── entity.hpp             # Entity = uint32_t
│   ├── component_array.hpp    # Dense array, O(1) swap-and-pop
│   ├── components.hpp         # TransformComponent + MeshRendererComponent
│   └── registry.hpp           # create/destroy/view<T...>(), add/get/removeComponent
├── renderer/
│   ├── shader.hpp/.cpp        # GLSL compile/link, setMat4, setVec3
│   ├── mesh.hpp/.cpp          # Generic VAO/VBO/EBO, Vertex struct, move-only
│   ├── model.hpp/.cpp         # Assimp loader (scene → nodes → meshes)
│   └── camera.hpp/.cpp        # 3D FPS camera, view/projection matrices
└── ui/
    └── debug_ui.hpp/.cpp      # ImGui wrapper: init, FPS, entity inspector, camera stats
assets/
└── models/
    └── test_cube.obj          # Test model with normals
vendor/
├── glad/                      # OpenGL 4.5 Core loader (generated)
└── imgui/                     # Dear ImGui core + SDL2/OpenGL3 backends
build/                         # Object files (gitignored)
compile_flags.txt              # clangd flags
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
- [ ] MaterialComponent (per-entity color)
- [ ] Model loading UI (drag & drop?)
- [ ] Lighting & materials editor
- [ ] PBR pipeline
- [ ] Entity/Scene graph
- [ ] Skybox / cubemaps

---

*Crafted with C++20, SDL2, OpenGL 4.5, GLM, Assimp, and too much coffee.*
