#include "Button.h"
#include "../../Game/Game.h"

GLfloat vertices[] = {
	-0.5f, -0.5f, -1.0f,		0.0f, 0.0f,
	 0.5f, -0.5f, -1.0f,		1.0f, 0.0f,
	 0.5f,  0.5f, -1.0f,		1.0f, 1.0f,
	 -0.5f,  0.5f, -1.0f,		0.0f, 1.0f,
};

GLuint indices[] = {
	0, 1, 2, 0, 2, 3
};

Button::Button() {
	m_text = new Text();
	m_text->setCentered(true);

	m_image = new Image(getTexture("button"));
	m_image->setSize({ 0, 256, 0, 32 });

	m_size = { 0, 256, 0, 32 };
}

Button::~Button() {
	delete m_text;
	m_text = nullptr;

	delete m_image;
	m_image = nullptr;
}

bool Button::isHovered() {
	GameWindow* gameWindow = Game::getInstance()->getGameWindow();
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
	return Game::getInstance()->getKeyHandler()->mouseClicked(GLFW_MOUSE_BUTTON_LEFT) && isHovered();
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
	if (isHovered()) {
		m_image->setColor({ .5f, .5f, .5f, 1.0f });
	} else {
		m_image->setColor(glm::vec4(1.0f));
	}

	m_image->render();
	m_text->render();
}
