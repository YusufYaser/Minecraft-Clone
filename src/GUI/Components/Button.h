#pragma once

#include "GUIComponent.h"
#include "Text.h"
#include "Image.h"

class Button : public GUIComponent {
public:
	Button();
	~Button();

	bool isHovered();
	bool isClicked();

	void setText(const char* content);
	void setEnabled(bool enabled) { m_enabled = enabled; };

	void setPosition(glm::vec4 pos) override;
	void setSize(glm::vec4 size) override;

	void render() override;

private:
	Text* m_text;
	Image* m_image;

	bool m_enabled;
};
