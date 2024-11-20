#include "Player.h"
#include "../Game/Game.h" 

Player::Player()
{
    Player::pos = glm::ivec3();
    Player::world = Game::getInstance()->getWorld();
    Player::originalPos = glm::vec3();
}

void Player::update(float delta)
{
    glm::ivec3 iPos = glm::ivec3(
        round(pos.x),
        round(pos.y),
        round(pos.z)
    );

    pos -= (fallSpeed * delta) * up;

    if (jumpSpeed == 0.0f) {
        Block* belowBlock = world->getBlock(glm::vec3(iPos.x, pos.y, iPos.z) - up * delta);
        if (belowBlock == nullptr || !belowBlock->hasCollision()) {
            fallSpeed += 15.0f * delta;
            if (fallSpeed > 50.0f) fallSpeed = 50.0f;
        }
        else {
            fallSpeed = 0.0f;
            pos.y = belowBlock->getPos().y + 1.0f;
        }
    } else if (jumpSpeed != 0.0f) {
        pos += (jumpSpeed * delta) * up;
        jumpSpeed -= 15.0f * delta;
        if (jumpSpeed <= 0.0f) {
            jumpSpeed = 0.0f;
            fallSpeed = 15.0f * delta;
        }
    }

    if (fallSpeed == 0.0f) {
        Block* collBlock = world->getBlock(iPos);
        if (collBlock != nullptr && collBlock->hasCollision()) {
            pos.y = collBlock->getPos().y + 1.0f;
        }
    }

    if (pos.y < -20.0f) {
        pos.y = 20.0f;
    }
}

void Player::checkInputs(GLFWwindow* window, float delta) {
    if (glfwGetWindowAttrib(window, GLFW_FOCUSED) == GLFW_FALSE) return;

    glm::vec3 orientation2 = glm::vec3(orientation.x, 0.0f, orientation.z);

    if (glfwGetKey(window, GLFW_KEY_F3) == GLFW_PRESS) {
        if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
            pos = { 0.0f, 30.0f, 0.0f };
        }
        if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS) {
            pos.y = 30.0f;
        }
    }

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        glm::vec3 change = (speed * delta) * glm::normalize(orientation2);
        glm::ivec3 newIPos = glm::ivec3(
            round(pos.x + change.x),
            round(pos.y + change.y),
            round(pos.z + change.z)
        );
        Block* block = world->getBlock(newIPos);
        if (block == nullptr || !block->hasCollision()) pos += change;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        glm::vec3 change = (-speed * delta) * glm::normalize(orientation2);
        glm::ivec3 newIPos = glm::ivec3(
            round(pos.x + change.x),
            round(pos.y + change.y),
            round(pos.z + change.z)
        );
        Block* block = world->getBlock(newIPos);
        if (block == nullptr || !block->hasCollision()) pos += change;
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        glm::vec3 change = (-speed * delta) * glm::normalize(glm::cross(orientation2, up));
        glm::ivec3 newIPos = glm::ivec3(
            round(pos.x + change.x),
            round(pos.y + change.y),
            round(pos.z + change.z)
        );
        Block* block = world->getBlock(newIPos);
        if (block == nullptr || !block->hasCollision()) pos += change;
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        glm::vec3 change = (speed * delta) * glm::normalize(glm::cross(orientation2, up));
        glm::ivec3 newIPos = glm::ivec3(
            round(pos.x + change.x),
            round(pos.y + change.y),
            round(pos.z + change.z)
        );
        Block* block = world->getBlock(newIPos);
        if (block == nullptr || !block->hasCollision()) pos += change;
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        if (fallSpeed == 0.0f && jumpSpeed == 0.0f) jumpSpeed = 5.5f;
    }

    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
        speed = PLAYER_RUN_SPEED;
    } else if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_RELEASE) {
        speed = PLAYER_SPEED;
    }

    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) selectedBlock = BLOCK_TYPE::STONE;
    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) selectedBlock = BLOCK_TYPE::GRASS;
    if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) selectedBlock = BLOCK_TYPE::DIRT;
    if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS) selectedBlock = BLOCK_TYPE::OAK_LOG;
    if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS) selectedBlock = BLOCK_TYPE::OAK_LEAVES;

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
        if (glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_NORMAL) brokeBlock = true; // prevent block breaking when capturing mouse
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

        if (!brokeBlock) {
            Block* target = nullptr;
            BLOCK_FACE face;
            getTargetBlock(&target, &face);

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

            if (target != nullptr) {
                glm::ivec3 iPos = glm::ivec3(
                    round(pos.x),
                    round(pos.y),
                    round(pos.z)
                );

                if (target->getPos() + getBlockFaceDirection(face) != iPos &&
                    target->getPos() + getBlockFaceDirection(face) != iPos + glm::ivec3(up)) {
                    world->setBlock(target->getPos() + getBlockFaceDirection(face), selectedBlock, false);
                    placedBlock = true;
                }
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

        float pitch = glm::degrees(glm::asin(glm::dot(orientation, up)));

        float maxPitch = 89.0f;
        if ((pitch - rotX) > maxPitch) {
            rotX = pitch - maxPitch;
        }
        else if ((pitch - rotX) < -maxPitch) {
            rotX = pitch + maxPitch;
        }

        glm::vec3 right = glm::normalize(glm::cross(orientation, up));
        glm::vec3 newOrientation = glm::rotate(orientation, glm::radians(-rotX), right);

        newOrientation = glm::rotate(newOrientation, glm::radians(-rotY), up);

        orientation = glm::normalize(newOrientation);

        glfwSetCursorPos(window, originalPos.x, originalPos.y);
    } else {
        wasHidden = false;
    }
}

void Player::getTargetBlock(Block** block, BLOCK_FACE* face) {
    glm::vec3 oldBlockPos;
    glm::vec3 cameraPos = getCameraPos();
    for (float i = 0; i < reachDistance; i += .01f) {
        glm::vec3 blockPosNoCeil = cameraPos + i * orientation;
        glm::vec3 blockPos = cameraPos + i * orientation;
        blockPos.x = round(blockPos.x);
        blockPos.y = round(blockPos.y);
        blockPos.z = round(blockPos.z);
        if (blockPos == oldBlockPos) continue;
        oldBlockPos = blockPos;
        Block* targetBlock = world->getBlock(blockPos);
        if (targetBlock != nullptr && targetBlock->hasCollision()) {
            if (block != nullptr) *block = targetBlock;
            if (face != nullptr) {
                BLOCK_FACE curFace = BLOCK_FACE::FRONT;
                float distance = 1000.0f;

                for (int i = 0; i < 6; i++) {
                    glm::ivec3 facePos = getBlockFaceDirection((BLOCK_FACE)i) + targetBlock->getPos();
                    float curDistance = glm::distance(glm::vec3(facePos), blockPosNoCeil);
                    if (curDistance < distance) {
                        curFace = (BLOCK_FACE)i;
                        distance = curDistance;
                    }
                }

                *face = curFace;
            }
            return;
        }
    }
    *block = nullptr;
}

glm::mat4 Player::getProjection() {
    static bool wasRunning = false;
    static double toggledRunning;
    double currentTime = glfwGetTime();

    glm::vec2 size = Game::getInstance()->getGameWindow()->getSize();

    bool isRunning = speed == PLAYER_RUN_SPEED;
    if (isRunning && !wasRunning) {
        toggledRunning = currentTime;
    }
    else if (!isRunning && wasRunning) { // stopped running
        toggledRunning = currentTime;
    }
    wasRunning = isRunning;
    float FOV = 45.0f;
    if (isRunning) {
        if (currentTime - toggledRunning > .1f) {
            FOV += 5.0f * speed / PLAYER_SPEED;
        }
        else {
            FOV += (currentTime - toggledRunning) * (5.0f * speed / PLAYER_SPEED) * 10.0f;
        }
    }
    else if (currentTime - toggledRunning < .1f) {
        FOV += 5.0f * PLAYER_RUN_SPEED / PLAYER_SPEED;
        FOV -= (currentTime - toggledRunning) * (5.0f * PLAYER_RUN_SPEED / PLAYER_SPEED) * 10.0f;
    }

    return glm::perspective(glm::radians(FOV), size.x / size.y, .1f, 1000.0f);
}

glm::mat4 Player::getView() {
    return glm::lookAt(getCameraPos(), getCameraPos() + orientation, up);
}
