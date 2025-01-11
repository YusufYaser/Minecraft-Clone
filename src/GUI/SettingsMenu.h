#pragma once

#include "Components/Text.h"
#include "Components/Button.h"

class SettingsMenu {
public:
	SettingsMenu();
	~SettingsMenu();

	void render();
	bool isClosing() const { return m_isClosing; };

private:
	Text* title;
	Button* back;

	Text* fps;
	Button* iFps;
	Button* dFps;

	Text* renderDistance;
	Button* iRenderDistance;
	Button* dRenderDistance;

	bool m_isClosing = false;
};
