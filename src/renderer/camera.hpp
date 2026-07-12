#pragma once
#include <glm/glm.hpp>

class Camera {
  public:
    Camera(glm::vec3 position, float aspectRatio);

    glm::mat4 getViewMatrix() const;
    glm::mat4 getProjectionMatrix() const;

  private:
    glm::vec3 position;
    glm::vec3 front; // dirección hacia la que mira
    glm::vec3 up;    // "arriba" del mundo, para orientar la cámara

    float aspectRatio;
    float fov; // campo de visión, en grados
    float nearPlane;
    float farPlane;
};
