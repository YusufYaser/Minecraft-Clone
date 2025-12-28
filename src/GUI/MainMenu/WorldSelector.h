#pragma once

#include "../Components/Text.h"
#include "../Components/Button.h"
#include "../../World/World.h"
#include "NewWorld.h"
#include <vector>

class WorldSelector {
public:
	WorldSelector();
	~WorldSelector();

	void render();
	bool isClosing() const { return m_isClosing; };

private:
	NewWorld* newWorldMenu = nullptr;

	Image* background;
	Text* title;
	Text* worldCount;
	Button* newWorld;
	Button* worldsDirectory;
	Button* back;
#ifdef GAME_DEBUG
	Button* debugWorld;
#endif

	Text* overflow;

	Text* upgradeText;
	bool m_showUpgradeText = false;

	bool m_isClosing = false;

	struct WorldEntry {
		std::string name;
		Button* playButton;
		bool willUpgrade;
	};

	std::vector<WorldEntry*> worlds;
};
