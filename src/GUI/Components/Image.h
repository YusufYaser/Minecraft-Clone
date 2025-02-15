#pragma once

#include "GUIComponent.h"
#include "Text.h"
#include "../../Shaders/Shaders.h"
#include "../../Textures/Textures.h"
#include <glad/gl.h>

class Image : public GUIComponent {
public:
	Image(Texture* tex);

	void setColor(glm::vec4 color) { m_color = color; };
	void setTexture(Texture* tex) { m_tex = tex; };
	void setCrop(glm::vec2 crop) { m_crop = crop; };

	Texture* getTexture() const { return m_tex; }

	void render() override;

private:
	Texture* m_tex;

	glm::vec4 m_color;
	glm::vec2 m_crop;

	static Text* debugText;
};
