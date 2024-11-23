#pragma once

#include "Text.h"

class PauseMenu {
public:
	PauseMenu();
	~PauseMenu();

	void render();

private:
	Text* title;
};
