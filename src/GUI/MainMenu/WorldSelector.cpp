#include "WorldSelector.h"
#include "../../Game/Game.h"

WorldSelector::WorldSelector() {
	title = new Text();
	title->setText("Select World");
	title->setPosition({ .5f, 0, 0, 75 });
	title->setCentered(true);
	title->setScale(2.0f);

	newWorld = new Button();
	newWorld->setText("Create New World");
	newWorld->setPosition({ .5f, 0, .5f, 0 });
	newWorld->setSize({ 0, 512, 0, 64 });

	back = new Button();
	back->setText("Back");
	back->setPosition({ .5f, 0, 1, -75 });
}

WorldSelector::~WorldSelector() {
	delete title;
	title = nullptr;

	delete newWorld;
	newWorld = nullptr;

	delete back;
	back = nullptr;
}

void WorldSelector::render() {
	Game* game = Game::getInstance();
	if (game->loadingWorld()) {
		if (game->getWorld() != nullptr) {
			int progress = static_cast<int>((16 - game->getWorld()->chunkLoadQueueCount()) / 16.0f * 100);
			std::stringstream str;
			str << "Loading World (" << progress << "%)";
			newWorld->setText(str.str().c_str()); // str :P
		} else {
			newWorld->setText("Loading World");
		}
	} else {
		newWorld->setText("Create New World");
	}

	newWorld->setEnabled(!game->loadingWorld());
	back->setEnabled(!game->loadingWorld());

	title->render();
	newWorld->render();
	back->render();

	if (newWorld->isClicked()) {
		WorldSettings settings;
		settings.generator = Generator::Default;
		game->loadWorld(settings);
	}

	if (back->isClicked()) {
		m_isClosing = true;
		return;
	}
}
