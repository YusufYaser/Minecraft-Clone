#include "Camera.h"

Camera::Camera(World* world, glm::vec3 pos)
{
    Camera::pos = pos;
    Camera::world = world;
    Camera::originalPos = glm::vec3();
}

void Camera::checkInputs(GLFWwindow* window, float delta) {
    if (glfwGetWindowAttrib(window, GLFW_FOCUSED) == GLFW_FALSE) return;

    glm::vec3 orientation2 = glm::vec3(orientation.x, 0.0f, orientation.z);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        pos += (speed * delta) * glm::normalize(orientation2);
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        pos -= (speed * delta) * glm::normalize(orientation2);
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
        if (glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_NORMAL) brokeBlock = true; // prevent block breaking when capturing mouse
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

        if (!brokeBlock) {
            Block* target = nullptr;
            BLOCK_FACE face;
            getTargetBlock(&target, &face);
            if (face == BLOCK_FACE::TOP) std::cout << "top";
            if (face == BLOCK_FACE::BOTTOM) std::cout << "bottom";
            if (face == BLOCK_FACE::FRONT) std::cout << "FRONT";
            if (face == BLOCK_FACE::LEFT) std::cout << "LEFT";
            if (face == BLOCK_FACE::RIGHT) std::cout << "RIGHT";
            if (face == BLOCK_FACE::BACK) std::cout << "BACK";

            if (target != nullptr) {
                world->setBlock(target->getPos(), BLOCK_TYPE::AIR);
                brokeBlock = true;
            }
        }
    }
    else {
        brokeBlock = false;
    }

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS &&
        glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_HIDDEN) {

        if (!placedBlock) {
            Block* target = nullptr;
            BLOCK_FACE face;
            getTargetBlock(&target, &face);
            if (face == BLOCK_FACE::TOP) std::cout << "top";
            if (face == BLOCK_FACE::BOTTOM) std::cout << "bottom";
            if (face == BLOCK_FACE::FRONT) std::cout << "FRONT";
            if (face == BLOCK_FACE::LEFT) std::cout << "LEFT";
            if (face == BLOCK_FACE::RIGHT) std::cout << "RIGHT";
            if (face == BLOCK_FACE::BACK) std::cout << "BACK";

            if (target != nullptr) {
                world->setBlock(target->getPos() + getBlockFaceDirection(face), BLOCK_TYPE::STONE, false);
                placedBlock = true;
            }
        }
    }
    else {
        placedBlock = false;
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

        if (abs(glm::angle(newOrientation, up) - glm::radians(90.0f)) <= glm::radians(90.0f))
        {
            orientation = newOrientation;
        }

        orientation = glm::rotate(orientation, glm::radians(-rotY), up);

        glfwSetCursorPos(window, originalPos.x, originalPos.y);
    } else {
        wasHidden = false;
    }
}

void Camera::getTargetBlock(Block** block, BLOCK_FACE* face)
{
    for (int i = 0; i < 10; i++) {
        glm::vec3 blockPosNoCeil = pos + (float)i * orientation;
        glm::vec3 blockPos = pos + (float)i * orientation;
        blockPos.x = ceil(blockPos.x);
        blockPos.y = ceil(blockPos.y);
        blockPos.z = ceil(blockPos.z);
        Block* targetBlock = world->getBlock(blockPos);
        if (targetBlock != nullptr) {
            if (block != nullptr) *block = targetBlock;
            if (face != nullptr) {
                glm::vec3 ceiled = blockPosNoCeil - blockPos;
                glm::vec3 dir = glm::normalize(orientation - (targetBlock->getPos() + ceiled - pos));

                if (abs(dir.x) > abs(dir.y) && abs(dir.x) > abs(dir.y)) {
                    if (dir.x > 0) *face = BLOCK_FACE::RIGHT;
                    if (dir.x < 0) *face = BLOCK_FACE::LEFT;
                }

                if (abs(dir.y) > abs(dir.x) && abs(dir.y) > abs(dir.z)) {
                    if (dir.y > 0) *face = BLOCK_FACE::TOP;
                    if (dir.y < 0) *face = BLOCK_FACE::BOTTOM;
                }

                if (abs(dir.z) > abs(dir.x) && abs(dir.z) > abs(dir.x)) {
                    if (dir.z > 0) *face = BLOCK_FACE::FRONT;
                    if (dir.z < 0) *face = BLOCK_FACE::BACK;
                }

                std::cout << "X: " << dir.x << " Y: " << dir.y << " Z: " << dir.z << "\n";
            }
            return;
        }
    }
    *block = nullptr;
}
