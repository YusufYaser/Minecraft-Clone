#pragma once

#include "GUIComponent.h"
#include "../../Shaders/Shaders.h"
#include <glad/gl.h>

class Image : public GUIComponent {
public:
	Image(GLuint tex);

	void setColor(glm::vec4 color) { m_color = color; };

	void render() override;

private:
	GLuint m_tex;

	glm::vec4 m_color;
};
