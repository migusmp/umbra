#include "engine.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

#include "../renderer/camera.hpp"

// De momento el GLSL vive aquí mismo, como texto (raw string literal, con
// R"(...)"). Más adelante, cuando tengamos un AssetManager, esto pasará a
// cargarse desde archivos .vert/.frag en assets/shaders/.

static const char* vertexShaderSrc = R"(
#version 450 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
)";

static const char* fragmentShaderSrc = R"(
#version 450 core
out vec4 FragColor;

void main() {
    FragColor = vec4(0.9, 0.4, 0.2, 1.0); // naranja
}
)";

Engine::Engine(const std::string& title, int width, int height)
    : width(width), height(height), running(false), lastTime(0) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "Error initializing SDL: " << SDL_GetError() << std::endl;
        std::exit(1);
    }

    window = std::make_unique<Window>(title, width, height);

    // Shader y Triangle se crean AQUÍ, después de que Window ya haya
    // cargado glad — si los creas antes, las funciones de OpenGL que usan
    // por dentro todavía no existirían (mismo problema de orden que ya
    // vimos con SDL_Init).
    shader = std::make_unique<Shader>(vertexShaderSrc, fragmentShaderSrc);

    // Se acabó construir vértices a mano — Model se encarga de leer el
    // archivo y montar la geometría real por dentro.
    model = std::make_unique<Model>("assets/models/test_cube.obj");

    float aspectRatio = static_cast<float>(width) / static_cast<float>(height);
    // Cámara colocada un poco atrás en Z, mirando hacia el origen (donde
    // está el triángulo) por la dirección por defecto (-Z).
    camera = std::make_unique<Camera>(glm::vec3(0.0f, 0.0f, 3.0f), aspectRatio);

    // Modo relativo: oculta el cursor y lo "reencierra" en el centro cada
    // frame, dándonos el movimiento del ratón como delta (xrel/yrel) en vez
    // de una posición absoluta en pantalla — justo lo que necesita una
    // cámara estilo FPS.
    SDL_SetRelativeMouseMode(SDL_TRUE);

    lastTime = SDL_GetTicks64();
    running = true;
}

Engine::~Engine() {
    SDL_Quit();
}

void Engine::run() {
    while (running) {
        Uint64 currentTime = SDL_GetTicks64();
        float dt = (currentTime - lastTime) / 1000.0f;
        lastTime = currentTime;

        processEvents();
        update(dt);
        render();
    }
}

void Engine::processEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            running = false;
        }

        if (event.type == SDL_MOUSEMOTION) {
            // yrel se invierte: en pantalla, "abajo" es positivo, pero
            // queremos que mover el ratón hacia ARRIBA incline la cámara
            // hacia arriba (pitch positivo).
            camera->processMouseMovement(static_cast<float>(event.motion.xrel),
                                         static_cast<float>(-event.motion.yrel));
        }

        if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) {
            running = false;
        }
    }
}

void Engine::update(float dt) {
    // SDL_GetKeyboardState nos da el estado ACTUAL de cada tecla (pulsada
    // o no) en cada frame — a diferencia de SDL_KEYDOWN/UP en el loop de
    // eventos, que solo avisa del CAMBIO de estado. Para movimiento
    // continuo (mientras la tecla siga pulsada), esto es lo correcto.
    const Uint8* keys = SDL_GetKeyboardState(nullptr);

    if (keys[SDL_SCANCODE_W])
        camera->processKeyboard(CameraMovement::FORWARD, dt);
    if (keys[SDL_SCANCODE_S])
        camera->processKeyboard(CameraMovement::BACKWARD, dt);
    if (keys[SDL_SCANCODE_A])
        camera->processKeyboard(CameraMovement::LEFT, dt);
    if (keys[SDL_SCANCODE_D])
        camera->processKeyboard(CameraMovement::RIGHT, dt);
}

void Engine::render() {
    glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    shader->use();

    // Model: de momento el triángulo se queda quieto en el origen, sin
    // rotación ni escala — matriz identidad.
    glm::mat4 modelMatrix = glm::mat4(1.0f);

    shader->setMat4("model", modelMatrix);
    shader->setMat4("view", camera->getViewMatrix());
    shader->setMat4("projection", camera->getProjectionMatrix());

    model->draw();

    SDL_GL_SwapWindow(window->getHandle());
}
