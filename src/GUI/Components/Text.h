#pragma once

#include "GUIComponent.h"
#include <glad/gl.h>
#include <gltext/gltext.h>
#include <string>
#include <glm/glm.hpp>

class Text : public GUIComponent {
public:
	Text();
	~Text();

	void setText(std::string content);
	void setColor(glm::vec4 color) { m_color = color; };
	void setScale(float scale) { m_scale = scale; };
	void setCentered(bool centered) { m_centered = centered; };

	virtual void render();

protected:
	GLTtext* m_text;

	glm::vec4 m_color;
	float m_scale;
	bool m_centered;
};
