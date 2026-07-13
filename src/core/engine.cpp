#include "engine.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

#include "../ecs/components.hpp"
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

// Datos que el fragment shader necesita para calcular la luz por píxel:
// posición y normal, ambas en ESPACIO DE MUNDO (no en clip space, que es
// donde acaba gl_Position — esos dos espacios son distintos y con
// propósitos distintos).
out vec3 FragPos;
out vec3 Normal;

void main() {
    FragPos = vec3(model * vec4(aPos, 1.0));

    // La normal NO se transforma con "model" directamente — se usa la
    // matriz normal (transpuesta de la inversa de model). Si el objeto
    // tuviera escala no uniforme (por ejemplo, estirado solo en X), usar
    // "model" tal cual dejaría las normales inclinadas de forma incorrecta.
    // Con escala uniforme (como nuestro cubo ahora) no se notaría el bug,
    // pero calcularlo bien desde ya evita un dolor de cabeza futuro.
    Normal = mat3(transpose(inverse(model))) * aNormal;

    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
)";

static const char* fragmentShaderSrc = R"(
#version 450 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;

uniform vec3 lightDir;      // dirección HACIA DONDE viaja la luz (ej: hacia abajo)
uniform vec3 viewPos;       // posición de la cámara, en espacio de mundo
uniform vec3 objectColor;
uniform vec3 lightColor;

void main() {
    // --- Ambiental: una base de luz mínima, siempre presente ---
    float ambientStrength = 0.15;
    vec3 ambient = ambientStrength * lightColor;

    // --- Difusa: depende del ángulo entre la normal y la luz ---
    vec3 norm = normalize(Normal);
    vec3 lightDirection = normalize(-lightDir); // hacia la luz, no hacia donde viaja
    float diff = max(dot(norm, lightDirection), 0.0);
    vec3 diffuse = diff * lightColor;

    // --- Especular: el brillo, depende también de dónde está la cámara ---
    float specularStrength = 0.5;
    vec3 viewDirection = normalize(viewPos - FragPos);
    vec3 reflectDirection = reflect(-lightDirection, norm);
    float spec = pow(max(dot(viewDirection, reflectDirection), 0.0), 32.0);
    vec3 specular = specularStrength * spec * lightColor;

    vec3 result = (ambient + diffuse + specular) * objectColor;
    FragColor = vec4(result, 1.0);
}
)";

Engine::Engine(const std::string& title, int width, int height)
    : width(width), height(height), running(false), lastTime(0) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "Error initializing SDL: " << SDL_GetError() << std::endl;
        std::exit(1);
    }

    window = std::make_unique<Window>(title, width, height);
    debugUI = std::make_unique<DebugUI>(window->getHandle(), window->getGLContext());

    // Shader y Triangle se crean AQUÍ, después de que Window ya haya
    // cargado glad — si los creas antes, las funciones de OpenGL que usan
    // por dentro todavía no existirían (mismo problema de orden que ya
    // vimos con SDL_Init).
    shader = std::make_unique<Shader>(vertexShaderSrc, fragmentShaderSrc);

    // El modelo se carga UNA vez como shared_ptr, y varias entidades
    // pueden apuntar a la misma instancia — no hace falta releer el
    // archivo ni duplicar los buffers de GPU por cada entidad que use
    // el mismo cubo.
    auto cubeModel = std::make_shared<Model>("assets/models/test_cube.obj");

    cubeEntity = registry.createEntity();
    registry.addComponent<TransformComponent>(cubeEntity, TransformComponent{});
    registry.addComponent<MeshRendererComponent>(cubeEntity, MeshRendererComponent{cubeModel});

    // Segundo cubo, desplazado en X, reutilizando el MISMO modelo — la
    // prueba real de que el ECS gestiona varias entidades independientes:
    // cada una con su propio Transform, pero compartiendo la malla.
    Entity secondCube = registry.createEntity();
    TransformComponent secondTransform;
    secondTransform.position = glm::vec3(2.0f, 0.0f, 0.0f);
    registry.addComponent<TransformComponent>(secondCube, secondTransform);
    registry.addComponent<MeshRendererComponent>(secondCube, MeshRendererComponent{cubeModel});

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
        debugUI->processEvent(event);

        if (event.type == SDL_QUIT) {
            running = false;
        }

        if (event.type == SDL_MOUSEMOTION && !debugUI->wantsCaptureMouse()) {
            // yrel se invierte: en pantalla, "abajo" es positivo, pero
            // queremos que mover el ratón hacia ARRIBA incline la cámara
            // hacia arriba (pitch positivo).
            camera->processMouseMovement(static_cast<float>(event.motion.xrel),
                                         static_cast<float>(-event.motion.yrel));
        }

        if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) {
            if (!debugUI->wantsCaptureKeyboard())
                running = false;
        }
    }
}

void Engine::update(float dt) {
    debugUI->buildUI(registry, *camera);

    // Cuando ImGui quiere el ratón (hover sobre panel, slider activo, etc.)
    // desactivamos el modo relativo para que el cursor del SO sea visible
    // y se pueda interactuar con la UI.
    SDL_SetRelativeMouseMode(debugUI->wantsCaptureMouse() ? SDL_FALSE : SDL_TRUE);

    // SDL_GetKeyboardState nos da el estado ACTUAL de cada tecla (pulsada
    // o no) en cada frame — a diferencia de SDL_KEYDOWN/UP en el loop de
    // eventos, que solo avisa del CAMBIO de estado. Para movimiento
    // continuo (mientras la tecla siga pulsada), esto es lo correcto.
    const Uint8* keys = SDL_GetKeyboardState(nullptr);

    if (!debugUI->wantsCaptureKeyboard()) {
        if (keys[SDL_SCANCODE_W])
            camera->processKeyboard(CameraMovement::FORWARD, dt);
        if (keys[SDL_SCANCODE_S])
            camera->processKeyboard(CameraMovement::BACKWARD, dt);
        if (keys[SDL_SCANCODE_A])
            camera->processKeyboard(CameraMovement::LEFT, dt);
        if (keys[SDL_SCANCODE_D])
            camera->processKeyboard(CameraMovement::RIGHT, dt);
        if (keys[SDL_SCANCODE_SPACE])
            camera->processKeyboard(CameraMovement::UP, dt);
        if (keys[SDL_SCANCODE_LCTRL])
            camera->processKeyboard(CameraMovement::DOWN, dt);
    }

    // Demostración de que el ECS gestiona estado real que cambia con el
    // tiempo: el primer cubo rota lentamente. Esto es lógica de "sistema"
    // (actúa sobre datos de un componente), aunque de momento vive aquí
    // mismo en vez de en una clase System separada — con solo una regla
    // de movimiento no compensa la abstracción extra todavía.
    auto& transform = registry.getComponent<TransformComponent>(cubeEntity);
    transform.rotation.y += 30.0f * dt;  // 30 grados por segundo
}

void Engine::render() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    shader->use();

    // Estos tres uniforms son iguales para TODA la escena en este frame
    // (misma cámara, misma luz) — se mandan una sola vez, fuera del bucle
    // de entidades, en vez de repetirlos por cada una.
    shader->setMat4("view", camera->getViewMatrix());
    shader->setMat4("projection", camera->getProjectionMatrix());
    shader->setVec3("lightDir", glm::vec3(-0.4f, -1.0f, -0.3f));
    shader->setVec3("lightColor", glm::vec3(1.0f, 1.0f, 1.0f));
    shader->setVec3("viewPos", camera->getPosition());

    // Esto es, en esencia, un "sistema de renderizado": recorre todas las
    // entidades que tengan AMBOS componentes (Transform + MeshRenderer),
    // y dibuja cada una según sus propios datos. Añadir una entidad nueva
    // a la escena no requiere tocar esta función en absoluto — basta con
    // crearla en algún sitio con los componentes adecuados.
    for (Entity entity : registry.view<TransformComponent, MeshRendererComponent>()) {
        auto& transform = registry.getComponent<TransformComponent>(entity);
        auto& meshRenderer = registry.getComponent<MeshRendererComponent>(entity);

        shader->setMat4("model", transform.getModelMatrix());
        // objectColor de momento fijo aquí — cuando exista un
        // MaterialComponent, esto vendrá de ahí en vez de un valor fijo.
        shader->setVec3("objectColor", glm::vec3(0.9f, 0.4f, 0.2f));

        meshRenderer.model->draw();
    }

    debugUI->render();

    SDL_GL_SwapWindow(window->getHandle());
}
