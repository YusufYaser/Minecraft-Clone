#pragma once

#include "Text.h"

class DebugText {
public:
	static void initialize();
	static void cleanup();
	static void render();

private:
	static Text* m_debugText;
};
