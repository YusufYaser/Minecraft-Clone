#include "Image.h"
#include "../../Game/Game.h"

Text* Image::debugText = nullptr;

Image::Image(Texture* tex) {
	m_tex = tex;
	m_color = glm::vec4(1.0f);
	m_size = { 0, tex->width, 0, tex->height };
	m_crop = { 1.0f, 1.0f };

	if (debugText == nullptr) {
		debugText = new Text();
	}
}

void Image::render() {
	Game* game = Game::getInstance();
	if (!game->isGuiEnabled()) return;

	m_lastRenderFrame = game->getFrameNum();

	GLfloat vertices[] = {
		-0.5f, -0.5f,		0.0f, 0.0f,
		 0.5f, -0.5f,		1.0f, 0.0f,
		 0.5f,  0.5f,		1.0f, 1.0f,
		 -0.5f,  0.5f,		0.0f, 1.0f,
	};

	GLuint indices[] = {
		0, 1, 2, 0, 2, 3
	};

	static GLuint VAO;

	if (VAO == 0) {
		GLuint VBO, EBO;

		glGenVertexArrays(1, &VAO);
		glBindVertexArray(VAO);

		glGenBuffers(1, &VBO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, 4 * 4 * sizeof(GLfloat), vertices, GL_STATIC_DRAW);

		glGenBuffers(1, &EBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(GLuint), indices, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);

		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

	Shader* guiShader = game->getGuiShader();

	guiShader->activate();

	guiShader->setUniform("guiPos", getGuiPos());
	guiShader->setUniform("guiSize", getGuiSize());
	guiShader->setUniform("guiZIndex", getGuiZIndex());
	guiShader->setUniform("guiColor", m_color);
	guiShader->setUniform("guiCrop", m_crop);

	glBindTexture(GL_TEXTURE_2D, m_tex->id);
	glBindVertexArray(VAO);

	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

	if (game->getDebugLevel() == 2) {
		std::stringstream stream;
		stream << std::hex << this;
		debugText->setText(stream.str());
		stream.clear();
		debugText->setPosition(m_pos - (m_size / 2.0f) + glm::vec4(0, 0, m_size.z, m_size.w - 15));
		debugText->render();
	}
}
