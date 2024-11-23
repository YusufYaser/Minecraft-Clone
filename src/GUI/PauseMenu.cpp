#include "PauseMenu.h"

PauseMenu::PauseMenu() {
	title = new Text();
	title->setText("Game Paused");
	title->setPosition({ .5f, 0, .5f, 0 });
	title->setCentered(true);
	title->setScale(2.0f);
}

PauseMenu::~PauseMenu() {
	delete title;
	title = nullptr;
}

void PauseMenu::render() {
	title->render();
}
