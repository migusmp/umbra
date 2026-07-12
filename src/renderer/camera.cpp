#include "camera.hpp"
#include <glm/gtc/matrix_transform.hpp>

Camera::Camera(glm::vec3 position, float aspectRatio)
    : position(position),
      front(glm::vec3(0.0f, 0.0f,
                      -1.0f)), // mira hacia -Z, convención estándar en OpenGL
      up(glm::vec3(0.0f, 1.0f, 0.0f)), aspectRatio(aspectRatio), fov(45.0f),
      nearPlane(0.1f), farPlane(100.0f) {}

glm::mat4 Camera::getViewMatrix() const {
    // lookAt necesita: dónde está la cámara, hacia qué PUNTO mira (no
    // dirección, punto — por eso sumamos position + front), y qué
    // dirección es "arriba" para no dejar la cámara torcida.
    return glm::lookAt(position, position + front, up);
}

glm::mat4 Camera::getProjectionMatrix() const {
    return glm::perspective(glm::radians(fov), aspectRatio, nearPlane,
                            farPlane);
}
