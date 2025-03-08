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
	Button* changeMode;
	Button* togglePostProcessed;
	Button* back;

	Text* resolution;
	Text* fileName;

	Image* m_world;
	Image* m_worldp;

	Image* black;

	double tookPhoto = -10;

	enum SCREENSHOT_MODE : uint8_t {
		DEFAULT = 0,
		ORTHO
	};

	SCREENSHOT_MODE mode = SCREENSHOT_MODE::DEFAULT;
	bool m_postProcessed = true;

	bool m_isClosing = false;
};
