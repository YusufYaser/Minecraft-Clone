#include "Crosshair.h"

GLfloat crosshairVertices[] = {
	-0.05f, -0.05f, -1.0f,		0.0f, 0.0f,
	 0.05f, -0.05f, -1.0f,		1.0f, 0.0f,
	 0.05f,  0.05f, -1.0f,		1.0f, 1.0f,
	 -0.05f,  0.05f, -1.0f,		0.0f, 1.0f,
};

GLuint crosshairIndices[] = {
	0, 1, 2, 0, 2, 3
};

Crosshair::Crosshair() {
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(crosshairVertices), crosshairVertices, GL_STATIC_DRAW);

	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(crosshairIndices), crosshairIndices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

Crosshair::~Crosshair() {
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
	glDeleteVertexArrays(1, &VAO);
}

void Crosshair::render(Shader* shader) {
	shader->activate();
	glUniformMatrix4fv(shader->getUniformLoc("model"), 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0f)));
	glUniform2fv(shader->getUniformLoc("guiPos"), 1, glm::value_ptr(glm::vec2()));
	glUniform1f(shader->getUniformLoc("guiScale"), .75f);
	glBindTexture(GL_TEXTURE_2D, getTexture("crosshair"));
	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, sizeof(crosshairIndices) / sizeof(GLuint), GL_UNSIGNED_INT, 0);
}
