#include "GUIComponent.h"
#include "../../Game/Game.h"

glm::vec2 GUIComponent::getGuiPos() {
	glm::ivec2 size = Game::getInstance()->getGameWindow()->getSize();

	return {
		m_pos.x + (m_pos.y / size.x),
		m_pos.z + (m_pos.w / size.y)
	};
}

glm::vec2 GUIComponent::getGuiSize() {
	glm::ivec2 wSize = Game::getInstance()->getGameWindow()->getSize();

	return {
		m_size.x + (m_size.y / wSize.x),
		m_size.z + (m_size.w / wSize.y)
	};
}

void GUIComponent::setZIndex(int zIndex) {
	assert(zIndex >= 0 && zIndex <= 1000);
	m_zIndex = zIndex;
}

bool GUIComponent::isRendered() {
	return m_lastRenderFrame == Game::getInstance()->getFrameNum();
}
