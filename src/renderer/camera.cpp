#include "camera.hpp"
#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>

Camera::Camera(glm::vec3 position, float aspectRatio)
    : position(position), worldUp(glm::vec3(0.0f, 1.0f, 0.0f)),
      yaw(-90.0f), // -90 grados = mirando hacia -Z, igual que antes con front
                   // fijo
      pitch(0.0f), aspectRatio(aspectRatio), fov(45.0f), nearPlane(0.1f),
      farPlane(100.0f), movementSpeed(2.5f), mouseSensitivity(0.1f) {
    updateVectors();
}

void Camera::updateVectors() {
    // Reconstruye "front" a partir de los ángulos yaw/pitch usando
    // trigonometría esférica: dado un ángulo horizontal y uno vertical,
    // esto da el vector de dirección correspondiente.
    glm::vec3 newFront;
    newFront.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    newFront.y = sin(glm::radians(pitch));
    newFront.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    front = glm::normalize(newFront);

    // right y up se recalculan a partir de front, usando producto vectorial
    // (cross product): "right" es perpendicular a front y al "arriba" del
    // mundo; "up" (el de la cámara, no el del mundo) es perpendicular a
    // ambos. Así la cámara nunca queda torcida de forma rara.
    right = glm::normalize(glm::cross(front, worldUp));
    up = glm::normalize(glm::cross(right, front));
}

glm::mat4 Camera::getViewMatrix() const {
    return glm::lookAt(position, position + front, up);
}

glm::mat4 Camera::getProjectionMatrix() const {
    return glm::perspective(glm::radians(fov), aspectRatio, nearPlane,
                            farPlane);
}

void Camera::processKeyboard(CameraMovement direction, float dt) {
    float velocity = movementSpeed * dt;

    if (direction == CameraMovement::FORWARD)
        position += front * velocity;
    if (direction == CameraMovement::BACKWARD)
        position -= front * velocity;
    if (direction == CameraMovement::LEFT)
        position -= right * velocity;
    if (direction == CameraMovement::RIGHT)
        position += right * velocity;
}

void Camera::processMouseMovement(float xoffset, float yoffset) {
    xoffset *= mouseSensitivity;
    yoffset *= mouseSensitivity;

    yaw += xoffset;
    pitch += yoffset;

    // Sin este límite, pasar el pitch de 90 grados invierte la cámara de
    // golpe (el clásico "camera flip" incómodo de los primeros FPS caseros).
    pitch = std::clamp(pitch, -89.0f, 89.0f);

    updateVectors();
}
