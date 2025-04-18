#pragma once

#include "../Components/Text.h"
#include "../Components/Button.h"
#include "../../World/World.h"
#include <vector>

class WorldSelector {
public:
	WorldSelector();
	~WorldSelector();

	void render();
	bool isClosing() const { return m_isClosing; };

private:
	Image* background;
	Text* title;
	Button* newWorld;
	Button* worldsDirectory;
	Button* back;
#ifdef GAME_DEBUG
	Button* debugWorld;
#endif

	Text* overflow;

	bool m_isClosing = false;

	struct WorldEntry {
		std::string name;
		Button* playButton;
	};

	std::vector<WorldEntry*> worlds;
};
