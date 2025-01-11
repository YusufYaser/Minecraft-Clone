#include "SettingsMenu.h"
#include "../Game/Game.h"

SettingsMenu::SettingsMenu() {
	title = new Text();
	title->setText("Settings");
	title->setPosition({ .5f, 0, 0, 75 });
	title->setCentered(true);
	title->setScale(2.0f);

	fps = new Text();
	fps->setPosition({ .5f, -250, 0, 125 });

	iFps = new Button();
	iFps->setText("+");
	iFps->setSize({ 0, 32, 0, 32 });
	iFps->setPosition({ .5f, 250, 0, 125 + 8 });

	dFps = new Button();
	dFps->setText("-");
	dFps->setSize({ 0, 32, 0, 32 });
	dFps->setPosition({ .5f, 250 - 48, 0, 125 + 8 });

	renderDistance = new Text();
	renderDistance->setPosition({ .5f, -250, 0, 175 });

	iRenderDistance = new Button();
	iRenderDistance->setText("+");
	iRenderDistance->setSize({ 0, 32, 0, 32 });
	iRenderDistance->setPosition({ .5f, 250, 0, 175 + 8 });

	dRenderDistance = new Button();
	dRenderDistance->setText("-");
	dRenderDistance->setSize({ 0, 32, 0, 32 });
	dRenderDistance->setPosition({ .5f, 250 - 48, 0, 175 + 8 });

	back = new Button();
	back->setText("Back");
	back->setPosition({ .5f, 0, 1, -75 });
}

SettingsMenu::~SettingsMenu() {
	delete back;
	back = nullptr;
}

void SettingsMenu::render() {
	Game* game = Game::getInstance();

	title->render();
	back->render();

	// fps
	int tFps = game->getMaxFps();
	iFps->render();
	if (iFps->isClicked()) tFps += 5;

	dFps->render();
	dFps->setEnabled(tFps > 0);
	if (dFps->isClicked()) tFps -= 5;

	tFps = tFps - (tFps % 5);
	if (tFps < 0) tFps = 4;

	game->setMaxFps(tFps);

	fps->setText("Max FPS: " + std::to_string(game->getMaxFps()));
	if (game->getMaxFps() == 0) {
		fps->setText("Max FPS: unlimited");
	}
	fps->render();

	// render distance
	int tRenderDistance = game->getRenderDistance();

	iRenderDistance->render();
	if (iRenderDistance->isClicked()) tRenderDistance += 2;

	dRenderDistance->render();
	dRenderDistance->setEnabled(tRenderDistance > 4);
	if (dRenderDistance->isClicked()) tRenderDistance -= 2;

	tRenderDistance = tRenderDistance - (tRenderDistance % 2);
	if (tRenderDistance < 4) tRenderDistance = 4;

	game->setRenderDistance(tRenderDistance);

	renderDistance->setText("Render Distance: " + std::to_string(game->getRenderDistance()));
	renderDistance->render();

	if ((back->isClicked() || game->getKeyHandler()->keyHeld(GLFW_KEY_ESCAPE)) && !game->loadingWorld()) {
		m_isClosing = true;
		return;
	}
}
