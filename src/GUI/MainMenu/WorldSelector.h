#pragma once

#include "../Components/Text.h"
#include "../Components/Button.h"
#include "../../World/World.h"

class WorldSelector {
public:
	WorldSelector();
	~WorldSelector();

	void render();
	bool isClosing() const { return m_isClosing; };

private:
	Text* title;
	Button* newWorld;
	Button* back;

	bool m_isClosing = false;
};
