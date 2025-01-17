#pragma once

#include <glm/glm.hpp>

class GUIComponent {
public:
	/*
	* @param pos xScale, xOffset, yScale, yOffset
	*/
	virtual void setPosition(glm::vec4 pos) { m_pos = pos; };
	virtual void setSize(glm::vec4 size) { m_size = size; };
	virtual void setZIndex(int zIndex);

	/*
	* @return pos xScale, xOffset, yScale, yOffset
	*/
	virtual glm::vec4 getPosition() const { return m_pos; };

	virtual void render() { assert("No rendering function set to this GUI component"); };

	bool isRendered();

protected:
	glm::vec4 m_pos = glm::vec4();
	glm::vec4 m_size = glm::vec4(1.0f, 0.0f, 1.0f, 0.0f);
	int m_zIndex = 500;

	glm::vec2 getGuiPos();
	glm::vec2 getGuiSize();
	int getGuiZIndex() const { return m_zIndex; };

	int m_lastRenderFrame = 0;
};
