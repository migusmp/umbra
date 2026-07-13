#pragma once
#pragma once
#include <glm/glm.hpp>

enum class CameraMovement { FORWARD, BACKWARD, LEFT, RIGHT, UP, DOWN };

class Camera {
   public:
    Camera(glm::vec3 position, float aspectRatio);

    glm::mat4 getViewMatrix() const;
    glm::mat4 getProjectionMatrix() const;
    glm::vec3 getPosition() const {
        return position;
    }
    glm::vec3 getFront() const {
        return front;
    }
    float getYaw() const {
        return yaw;
    }
    float getPitch() const {
        return pitch;
    }

    void processKeyboard(CameraMovement direction, float dt);
    void processMouseMovement(float xoffset, float yoffset);

   private:
    void updateVectors();

    glm::vec3 position;
    glm::vec3 front;
    glm::vec3 up;
    glm::vec3 right;
    glm::vec3 worldUp;  // referencia fija de "arriba" del mundo, no cambia

    // Ángulos de orientación, en grados. yaw = giro horizontal,
    // pitch = giro vertical (mirar arriba/abajo).
    float yaw;
    float pitch;

    float aspectRatio;
    float fov;
    float nearPlane;
    float farPlane;

    float movementSpeed;
    float mouseSensitivity;
};
