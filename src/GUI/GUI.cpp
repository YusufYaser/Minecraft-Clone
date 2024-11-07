#include "GUI.h"

GLfloat crosshairVertices[] = {
	-0.05f, -0.05f, -1.0f,
	 0.05f, -0.05f, -1.0f,
	 0.05f,  0.05f, -1.0f,
};

Crosshair::Crosshair()
{
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(crosshairVertices), crosshairVertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

Crosshair::~Crosshair()
{
	glDeleteBuffers(1, &VBO);
	glDeleteVertexArrays(1, &VAO);
}

void Crosshair::Render(GLuint shader)
{
	glUseProgram(shader);
	glBindVertexArray(VAO);
	glDrawArrays(GL_TRIANGLES, 0, sizeof(crosshairVertices) / sizeof(GLuint));
}
