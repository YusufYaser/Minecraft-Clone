#include "Button.h"
#include "../../Game/Game.h"

Button::Button() {
	m_text = new Text();
	m_text->setCentered(true);

	m_image = new Image(getTexture("button"));
	m_image->setSize({ 0, 256, 0, 32 });

	m_size = { 0, 256, 0, 32 };

	m_enabled = true;
}

Button::~Button() {
	delete m_text;
	m_text = nullptr;

	delete m_image;
	m_image = nullptr;
}

bool Button::isHovered() {
	if (!m_enabled) return false;
	if (!isRendered()) return false;

	GameWindow* gameWindow = Game::getInstance()->getGameWindow();
	if (!gameWindow->isFocused()) return false;
	double posX, posY;
	glfwGetCursorPos(gameWindow->getGlfwWindow(), &posX, &posY);
	glm::ivec2 size = gameWindow->getSize();
	glm::ivec2 center = size / 2;

	glm::vec2 guiSize = getGuiSize();
	glm::vec2 guiPos = getGuiPos();

	glm::vec2 posS = {
		posX / size.x,
		posY / size.y
	};

	return (
		posS.x >= guiPos.x - guiSize.x / 2 && posS.x <= guiPos.x + guiSize.x / 2 &&
		posS.y >= guiPos.y - guiSize.y / 2 && posS.y <= guiPos.y + guiSize.y / 2
		);
}

bool Button::isClicked() {
	if (Game::getInstance()->getKeyHandler()->mouseClicked(GLFW_MOUSE_BUTTON_LEFT) && isHovered()) {
		Game::getInstance()->getSoundEngine()->playSound(SOUND_TYPE::BUTTON);
		return true;
	}

	return false;
}

void Button::setText(const char* content) {
	m_text->setText(content);
}

void Button::setPosition(glm::vec4 pos) {
	m_text->setPosition(pos);
	m_image->setPosition(pos);

	GUIComponent::setPosition(pos);
}

void Button::setSize(glm::vec4 size) {
	m_image->setSize(size);

	GUIComponent::setSize(size);
}

void Button::render() {
	Game* game = Game::getInstance();
	if (!game->isGuiEnabled()) return;

	m_lastRenderFrame = game->getFrameNum();

	if (isHovered()) {
		m_image->setColor({ .5f, .5f, .5f, 1.0f });
	} else {
		m_image->setColor(glm::vec4(1.0f));
	}
	if (m_enabled) {
		m_text->setColor(glm::vec4(1.0f));
	} else {
		m_text->setColor({ .75f, .75f, .75f, 1.0f });
	}

	m_image->render();
	m_text->render();
}
