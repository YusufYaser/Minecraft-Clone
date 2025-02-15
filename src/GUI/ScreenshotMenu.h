#pragma once

#include "./Components/Text.h"
#include "./Components/Button.h"
#include "./Components/Image.h"

class ScreenshotMenu {
public:
	ScreenshotMenu();
	~ScreenshotMenu();

	bool isClosing() const { return m_isClosing; };

	void render();

private:
	Text* title;
	Button* screenshot;
	Button* togglePostProcessed;
	Button* back;

	Text* resolution;
	Text* fileName;

	Image* m_world;
	Image* m_worldp;

	Image* black;

	double tookPhoto = -10;

	bool m_postProcessed = true;

	bool m_isClosing = false;
};
