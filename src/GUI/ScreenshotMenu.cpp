#include "ScreenshotMenu.h"
#include "../Game/Game.h"
#include <stb/stb_image_write.h>

ScreenshotMenu::ScreenshotMenu() {
	title = new Text();
	title->setText("Take Photo");
	title->setPosition({ 0, 90, 0, 15 });
	title->setScale(2.0f);

	screenshot = new Button();
	screenshot->setText("Take Photo");
	screenshot->setPosition({ 0, 150, 0, 140 });

	togglePostProcessed = new Button();
	togglePostProcessed->setText("Toggle Post Processing");
	togglePostProcessed->setPosition({ 0, 150, 0, 100 });

	resolution = new Text();
	resolution->setPosition({ 0, 22, 0, 160 });

	fileName = new Text();
	fileName->setPosition({ 0, 22, 0, 180 });

	back = new Button();
	back->setText("Back");
	back->setPosition({ 0, 50, 0, 30 });
	back->setSize({ 0, 55, 0, 25 });

	m_world = new Image(Game::getInstance()->getWorldTex());
	m_world->setPosition({ .5f, 0, .5f, 0 });
	m_world->setSize({ 1.0f, 0, 1.0f, 0 });
	m_world->setZIndex(2);

	m_worldp = new Image(Game::getInstance()->getWorldPostProcessedTex());
	m_worldp->setPosition({ .5f, 0, .5f, 0 });
	m_worldp->setSize({ 1.0f, 0, 1.0f, 0 });
	m_worldp->setZIndex(2);

	black = new Image(getTexture("black"));
	black->setSize({ 1.0f, 0, 1.0f, 0 });
	black->setZIndex(600);
}

ScreenshotMenu::~ScreenshotMenu() {
	delete title;
	title = nullptr;

	delete screenshot;
	screenshot = nullptr;

	delete togglePostProcessed;
	togglePostProcessed = nullptr;

	delete m_world;
	m_world = nullptr;

	delete m_worldp;
	m_worldp = nullptr;

	delete black;
	black = nullptr;

	delete resolution;
	resolution = nullptr;

	delete fileName;
	fileName = nullptr;

	delete back;
	back = nullptr;
}

void ScreenshotMenu::render() {
	Game* game = Game::getInstance();
	double currentTime = glfwGetTime();

	Image* img = nullptr;
	if (m_postProcessed) {
		img = m_worldp;
	} else {
		img = m_world;
	}

	glm::ivec2 size = {
		img->getTexture()->width,
		img->getTexture()->height
	};

	resolution->setText("Resolution: " + std::to_string(size.x) + "x" + std::to_string(size.y));

	img->render();

	if (currentTime - tookPhoto < .5) {
		black->setPosition({ .5f, 0, (currentTime - tookPhoto) * 2 * 0.5f - .5f, 0 });
		black->render();
		black->setPosition({ .5f, 0, 1.5f - (currentTime - tookPhoto) * 2 * 0.5f, 0 });
		black->render();
	} else if (currentTime - tookPhoto < 1) {
		black->setPosition({ .5f, 0, (currentTime - tookPhoto + 1) * 2 * 0.5f - .5f, 0 });
		black->render();
		black->setPosition({ .5f, 0, 1.5f - (currentTime - tookPhoto + 1) * 2 * 0.5f, 0 });
		black->render();
	} else {
		if (tookPhoto > 0) {
			glBindTexture(GL_TEXTURE_2D, img->getTexture()->id);

			char* pixels = new char[size.x * size.y * 4];
			glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

			char* pixelsFlipped = new char[size.x * size.y * 4];
			for (int y = 0; y < size.y; y++) {
				memcpy(pixelsFlipped + y * size.x * 4, pixels + (size.y - 1 - y) * size.x * 4, size.x * 4);
			}

			std::filesystem::create_directory("screenshots");

			time_t timestamp = time(nullptr);
			tm datetime;
#ifdef _WIN32
			localtime_s(&datetime, &timestamp);
#else
			localtime_r(&timestamp, &datetime);
#endif

			std::ostringstream name;
			name << "screenshots/" << std::put_time(&datetime, "%Y-%m-%d %H-%M-%S") << ".png";

			int res = stbi_write_png(name.str().c_str(), size.x, size.y, 4, pixelsFlipped, size.x * 4);

			if (res == 1) {
				print("Saved screenshot at", name.str());
				fileName->setText("Saved photo at " + name.str());
			}

			delete[] pixels;
			pixels = nullptr;

			delete[] pixelsFlipped;
			pixelsFlipped = nullptr;

			tookPhoto = -10;
		}

		togglePostProcessed->render();

		if (togglePostProcessed->isClicked()) {
			m_postProcessed = !m_postProcessed;
		}

		screenshot->render();
		back->render();
		title->render();

		resolution->render();
		fileName->render();

		if (screenshot->isClicked()) {
			tookPhoto = currentTime;
		}
	}

	if ((back->isClicked() || game->getKeyHandler()->keyHeld(GLFW_KEY_ESCAPE)) && !game->loadingWorld()) {
		m_isClosing = true;
		return;
	}
}
