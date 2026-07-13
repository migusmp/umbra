#pragma once
#include <SDL2/SDL.h>

#include "../ecs/registry.hpp"
#include "../renderer/camera.hpp"

// DebugUI centraliza toda la integración con Dear ImGui: inicialización,
// el ciclo "nuevo frame -> construir la UI -> renderizar", y el cierre.
// El resto del motor no necesita saber nada de las llamadas internas de
// ImGui — solo llama a los métodos públicos de aquí.
class DebugUI {
   public:
    DebugUI(SDL_Window* window, SDL_GLContext glContext);
    ~DebugUI();

    // Debe llamarse con CADA evento de SDL, para que ImGui sepa de clics,
    // teclas, scroll, etc. dirigidos a sus propios paneles.
    void processEvent(const SDL_Event& event);

    // Construye los paneles de este frame (FPS, inspector de entidades...)
    // usando el estado actual del Registry. No dibuja nada todavía en la
    // GPU — eso lo hace render().
    void buildUI(Registry& registry, const Camera& camera);

    // Envía los draw calls reales de ImGui a OpenGL. Debe llamarse
    // DESPUÉS de dibujar la escena 3D, para que la UI quede por encima.
    void render();

    // Indica si el ratón/teclado está siendo "capturado" por un panel de
    // ImGui en este momento — útil para que la cámara no se mueva
    // mientras el usuario interactúa con un slider, por ejemplo.
    bool wantsCaptureMouse() const;
    bool wantsCaptureKeyboard() const;

   private:
    SDL_Window* window;
};
