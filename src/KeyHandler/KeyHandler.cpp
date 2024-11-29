#include "KeyHandler.h"
#include "../Game/Game.h"

KeyHandler::KeyHandler() {
    m_window = Game::getInstance()->getGlfwWindow();
}

void KeyHandler::update() {
    prevKeyboardStates = keyboardStatesToSet;
    prevMouseStates = mouseStatesToSet;
}

bool KeyHandler::keyHeld(int key) {
    if (glfwGetWindowAttrib(m_window, GLFW_FOCUSED) == GLFW_FALSE) return false;

    return glfwGetKey(m_window, key) == GLFW_PRESS;
}

bool KeyHandler::keyClicked(int key) {
    bool held = keyHeld(key);

    if (!held) {
        keyboardStatesToSet[key] = false;
        return false;
    }

    // was held in previous frame
    if (prevKeyboardStates[key] && held) return false;

    keyboardStatesToSet[key] = true;

    return true;
}

bool KeyHandler::keyReleased(int key) {
    // if it is not held and it was pressed in the previous frame
    return !keyHeld(key) && prevKeyboardStates[key];
}

bool KeyHandler::mouseHeld(int key) {
    return glfwGetMouseButton(m_window, key) == GLFW_PRESS;
}

bool KeyHandler::mouseClicked(int key) {
    bool held = mouseHeld(key);

    if (!held) {
        mouseStatesToSet[key] = false;
        return false;
    }

    // was held in previous frame
    if (prevMouseStates[key] && held) return false;

    mouseStatesToSet[key] = true;

    return true;
}

bool KeyHandler::mouseReleased(int key) {
    // if it is not held and it was pressed in the previous frame
    return !mouseHeld(key) && prevMouseStates[key];
}