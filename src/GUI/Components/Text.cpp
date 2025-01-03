#include "Text.h"
#include "../../Game/Game.h"

Text::Text() {
	m_text = gltCreateText();
	m_scale = 1.0f;
	m_color = glm::vec4(1.0f);
	m_centered = false;
}

Text::~Text() {
	gltDeleteText(m_text);
}

void Text::setText(const char* content) {
	gltSetText(m_text, content);
}

void Text::render() {
	m_lastRenderFrame = Game::getInstance()->getFrameNum();

	glm::ivec2 size = Game::getInstance()->getGameWindow()->getSize();

	gltBeginDraw();

	gltColor(m_color.x, m_color.y, m_color.z, m_color.w);
	glm::vec2 pos = {
		(m_pos.x * size.x) + m_pos.y,
		(m_pos.z * size.y) + m_pos.w,
	};

	if (m_centered) {
		pos.x -= round(gltGetTextWidth(m_text, m_scale) / 2);
		pos.y -= round(gltGetTextHeight(m_text, m_scale) / 2);
	}

	gltDrawText2D(m_text, pos.x, pos.y, m_scale);

	gltEndDraw();
}
