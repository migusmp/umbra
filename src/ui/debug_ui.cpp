#include "debug_ui.hpp"

#include <backends/imgui_impl_opengl3.h>
#include <backends/imgui_impl_sdl2.h>
#include <imgui.h>

#include "../ecs/components.hpp"

DebugUI::DebugUI(SDL_Window* window, SDL_GLContext glContext) : window(window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    // Dos backends distintos, cada uno responsable de una mitad del
    // trabajo: SDL2 traduce eventos de input a lo que ImGui entiende,
    // OpenGL3 sabe cómo dibujar lo que ImGui construye internamente.
    ImGui_ImplSDL2_InitForOpenGL(window, glContext);
    ImGui_ImplOpenGL3_Init("#version 450");
}

DebugUI::~DebugUI() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
}

void DebugUI::processEvent(const SDL_Event& event) {
    ImGui_ImplSDL2_ProcessEvent(&event);
}

void DebugUI::buildUI(Registry& registry, const Camera& camera) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    // --- Panel de rendimiento ---
    ImGui::SetNextWindowSize(ImVec2(340, 240), ImGuiCond_Appearing);
    ImGui::SetNextWindowPos(ImVec2(5, 5), ImGuiCond_Appearing);
    ImGui::Begin("Umbra Debug");
    ImGui::Text("FPS: %.0f", ImGui::GetIO().Framerate);
    ImGui::SeparatorText("Cámara");
    ImGui::Text("Pos: %.2f, %.2f, %.2f", camera.getPosition().x, camera.getPosition().y, camera.getPosition().z);
    ImGui::Text("Yaw: %.1f  Pitch: %.1f", camera.getYaw(), camera.getPitch());
    ImGui::Text("Front: %.2f, %.2f, %.2f", camera.getFront().x, camera.getFront().y, camera.getFront().z);
    ImGui::Separator();

    // --- Inspector de entidades ---
    // Recorre TODAS las entidades con Transform, sin que este panel sepa
    // nada de "cubos" en concreto — funciona igual con cualquier entidad
    // que exista, presente o futura.
    ImGui::Text("Entidades:");
    for (Entity entity : registry.view<TransformComponent>()) {
        ImGui::PushID(static_cast<int>(
            entity));  // evita que ImGui confunda sliders repetidos entre entidades

        ImGui::Text("Entity %u", entity);
        auto& transform = registry.getComponent<TransformComponent>(entity);

        ImGui::DragFloat3("Position", &transform.position.x, 0.05f);
        ImGui::DragFloat3("Rotation", &transform.rotation.x, 1.0f);
        ImGui::DragFloat3("Scale", &transform.scale.x, 0.05f, 0.01f, 10.0f);
        ImGui::Separator();

        ImGui::PopID();
    }

    ImGui::End();
}

void DebugUI::render() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

bool DebugUI::wantsCaptureMouse() const {
    return ImGui::GetIO().WantCaptureMouse;
}

bool DebugUI::wantsCaptureKeyboard() const {
    return ImGui::GetIO().WantCaptureKeyboard;
}
