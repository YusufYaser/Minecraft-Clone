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
	Text* title;
	Button* newWorld;
	Button* worldsDirectory;
	Button* back;

	Text* overflow;

	bool m_isClosing = false;

	struct WorldEntry {
		std::string name;
		Button* playButton;
	};

	std::vector<WorldEntry*> worlds;
};
