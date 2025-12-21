#pragma once
#include "../Components/Button.h"
#include "../Components/Text.h"
#include "../../World/World.h"

class NewWorld {
public:
	NewWorld();
	~NewWorld();

	void render();
	bool isClosing() const { return m_isClosing; };

private:
	Image* background;
	Text* title;

	bool m_isClosing = false;

	Text* generatorText;
	Button* defaultGenerator;
	Button* flatGenerator;
	Button* voidGenerator;
	Generator selectedGenerator = Generator::Default;

	Text* structuresText;
	Button* enableStructures;
	Button* disableStructures;
	bool structuresEnabled = true;

	Button* createWorldButton;
	Button* back;
};
