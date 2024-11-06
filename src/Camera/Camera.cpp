#include "Camera.h"

Camera::Camera(glm::vec3 pos)
{
    Camera::pos = pos;
}

void Camera::checkInputs(GLFWwindow* window, float delta) {
    glm::vec3 orientation2 = glm::vec3(orientation.x, 0.0f, orientation.z);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        pos += (speed * delta) * orientation2;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        pos -= (speed * delta) * orientation2;
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        pos -= (speed * delta) * glm::normalize(glm::cross(orientation2, up));
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        pos += (speed * delta) * glm::normalize(glm::cross(orientation2, up));
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        pos += (speed * delta) * up;
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
        pos -= (speed * delta) * up;
    }
    
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    }

    if (glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_HIDDEN) {
        double posX, posY;
        glfwGetCursorPos(window, &posX, &posY);

        int width, height;
        glfwGetWindowSize(window, &width, &height);

        if (!wasHidden) {
            wasHidden = true;
            originalPos = glm::vec2(posX, posY);
        }

        float rotX = 100.0f * (float)(posY - originalPos.y) / height;
        float rotY = 100.0f * (float)(posX - originalPos.x) / width;

        glm::vec3 newOrientation = glm::rotate(orientation, glm::radians(-rotX), glm::normalize(glm::cross(orientation, up)));

        if (abs(glm::angle(newOrientation, up) - glm::radians(90.0f)) <= glm::radians(85.0f))
        {
            orientation = newOrientation;
        }

        orientation = glm::rotate(orientation, glm::radians(-rotY), up);

        glfwSetCursorPos(window, originalPos.x, originalPos.y);
    } else {
        wasHidden = false;
    }
}
