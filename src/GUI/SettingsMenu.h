#pragma once

#include "Components/Text.h"
#include "Components/Button.h"

class SettingsMenu {
public:
	SettingsMenu();

	void render();
	bool isClosing() const { return m_isClosing; };

private:
	Image* background;

	Text* title;
	Button* back;

	Text* fps;
	Button* iFps;
	Button* dFps;

	Text* renderDistance;
	Button* iRenderDistance;
	Button* dRenderDistance;

	Text* worldRes;
	Button* iWorldRes;
	Button* dWorldRes;

	Text* mergeSize;
	Button* cMergeSize;

	Text* memoryUsage;

	bool m_isClosing = false;
};
