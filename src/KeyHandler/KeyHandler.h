#pragma once

#include <GLFW/glfw3.h>
#include <unordered_map>
#include "../GameWindow/GameWindow.h"

class KeyHandler {
public:
	KeyHandler();

	void update();

	bool keyHeld(int key);

	// If the key was pressed in the last frame, this will return false
	bool keyClicked(int key);

	// If it wasn't held in the last frame, this will return false
	bool keyReleased(int key);

	bool mouseHeld(int key);
	bool mouseClicked(int key);
	bool mouseReleased(int key);

private:
	GameWindow* m_gameWindow;

	std::unordered_map<int, bool> keyboardStatesToSet;
	std::unordered_map<int, bool> prevKeyboardStates;

	std::unordered_map<int, bool> mouseStatesToSet;
	std::unordered_map<int, bool> prevMouseStates;
};
