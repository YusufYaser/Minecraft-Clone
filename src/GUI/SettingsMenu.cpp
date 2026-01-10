#include "SettingsMenu.h"
#include "../Game/Game.h"

size_t getMaxMemory();

SettingsMenu::SettingsMenu() {
	background = new Image(getTexture("background"));
	background->setSize({ 1, 0, 1, 0 });
	background->setPosition({ .5f, 0, .5f, 0 });

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

	worldRes = new Text();
	worldRes->setPosition({ .5f, -250, 0, 225 });

	iWorldRes = new Button();
	iWorldRes->setText("+");
	iWorldRes->setSize({ 0, 32, 0, 32 });
	iWorldRes->setPosition({ .5f, 250, 0, 225 + 8 });

	dWorldRes = new Button();
	dWorldRes->setText("-");
	dWorldRes->setSize({ 0, 32, 0, 32 });
	dWorldRes->setPosition({ .5f, 250 - 48, 0, 225 + 8 });

	mergeSize = new Text();
	mergeSize->setText("Merge Size: None");
	mergeSize->setPosition({ .5f, -250, 0, 267 });

	cMergeSize = new Button();
	cMergeSize->setText("Change");
	cMergeSize->setSize({ 0, 80, 0, 32 });
	cMergeSize->setPosition({ .5f, 226, 0, 275 + 8 });

	memoryUsage = new Text();
	memoryUsage->setPosition({ 0, 0, 1, -15 });

	back = new Button();
	back->setText("Back");
	back->setPosition({ .5f, 0, 1, -75 });
}

void SettingsMenu::render() {
	Game* game = Game::getInstance();

	if (game->getWorld() == nullptr) background->render();
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

	if (static_cast<unsigned long long>(tRenderDistance) * tRenderDistance * 2 * 1024 * 1024 > getMaxMemory() && getMaxMemory() != 0) {
		renderDistance->setColor({ 1.0f, 0.0f, 0.0f, 1.0f });
		memoryUsage->setColor({ 1.0f, 0.0f, 0.0f, 1.0f });
	} else {
		renderDistance->setColor({ 1.0f, 1.0f, 1.0f, 1.0f });
		memoryUsage->setColor({ 1.0f, 1.0f, 1.0f, 1.0f });
	}

	renderDistance->setText("Render Distance: " + std::to_string(game->getRenderDistance()));
	renderDistance->render();

	// 3d resolution
	int tWorldRes = int(round(game->getWorldResolution() * 100));

	iWorldRes->render();
	if (iWorldRes->isClicked()) tWorldRes += 5;

	dWorldRes->render();
	dWorldRes->setEnabled(tWorldRes > 0.01f);
	if (dWorldRes->isClicked()) tWorldRes -= 5;

	if (tWorldRes < 5) tWorldRes = 5;

	game->setWorldResolution(tWorldRes / 100.0f);

	worldRes->setText("3D Resolution: " + std::to_string(int(game->getWorldResolution() * 100)) + "%");
	worldRes->render();

	cMergeSize->render();
	if (cMergeSize->isClicked()) {
		game->setMergeSize(MergeSize(((uint8_t)game->getMergeSize() + 1) % 3));
	}

	switch (game->getMergeSize()) {
	case MergeSize::None:
		mergeSize->setText("Block Merge Size: None\nThis will help reduce lag spikes");
		break;
	case MergeSize::OneByTwo:
		mergeSize->setText("Block Merge Size: 1x2\nDefault and Recommended");
		break;
	case MergeSize::TwoByTwo:
		mergeSize->setText("Block Merge Size: 2x2\nNot recommended, not significantly better than 1x2");
		break;
	}
	mergeSize->render();

	std::stringstream memUsage;
	memUsage << "Approximate Memory Usage: " << round(tRenderDistance * tRenderDistance * 2) << " MB (inaccurate)";
	memoryUsage->setText(memUsage.str().c_str());
	memoryUsage->render();

	if ((back->isClicked() || game->getKeyHandler()->keyHeld(GLFW_KEY_ESCAPE)) && !game->loadingWorld()) {
		m_isClosing = true;
		return;
	}
}
