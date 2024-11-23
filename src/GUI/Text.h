#pragma once

#include <glad/gl.h>
#define GLT_IMPORTS
#include <gltext/gltext.h>
#include <string>
#include <glm/glm.hpp>
#include "../Player/Player.h"

class Text {
public:
	Text();
	~Text();
	/*
	* @param pos xScale, xOffset, yScale, yOffset
	*/
	void setPosition(glm::vec4 pos) { m_pos = pos; };
	void setText(const char* content);
	void setColor(glm::vec4 color) { m_color = color; };
	void setScale(float scale) { m_scale = scale; };
	void setCentered(bool centered) { m_centered = centered; };

	void render();

private:
	GLTtext* m_text;

	glm::vec4 m_pos;
	glm::vec4 m_color;
	float m_scale;
	bool m_centered;
};
