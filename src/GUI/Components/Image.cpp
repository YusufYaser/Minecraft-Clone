#include "Image.h"
#include "../../Game/Game.h"

Image::Image(Texture* tex) {
	m_tex = tex;
	m_color = glm::vec4(1.0f);
	m_size = { 0, tex->width, 0, tex->height };
}

void Image::render() {
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

	static Shader* guiShader = nullptr;
	if (guiShader == nullptr) {
		guiShader = Game::getInstance()->getGuiShader();
	}

	guiShader->activate();

	guiShader->setUniform("guiPos", getGuiPos());
	guiShader->setUniform("guiSize", getGuiSize());
	guiShader->setUniform("guiColor", m_color);

	glBindTexture(GL_TEXTURE_2D, m_tex->id);
	glBindVertexArray(VAO);

	glDrawElements(GL_TRIANGLES, sizeof(indices) / sizeof(GLuint), GL_UNSIGNED_INT, 0);
}
