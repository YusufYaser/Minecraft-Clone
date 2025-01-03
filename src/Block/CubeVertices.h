#pragma once

#define VERTEX_SIZE 6

GLfloat blockVertices[] = {
	// positions          // texture coords
	// front face
	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 0.0f, // bottom-left
	 0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f, // bottom-right
	 0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 0.0f, // top-right
	-0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 0.0f, // top-left

	// back face
	-0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 1.0f, // bottom-left
	 0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 1.0f, // bottom-right
	 0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 1.0f, // top-right
	-0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 1.0f, // top-left

	// right face
	 0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 2.0f, // bottom-left
	 0.5f,  0.5f, -0.5f,  1.0f, 0.0f, 2.0f, // top-left
	 0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 2.0f, // top-right
	 0.5f, -0.5f,  0.5f,  0.0f, 1.0f, 2.0f, // bottom-right

	 // left face
	 -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 3.0f, // bottom-right
	 -0.5f,  0.5f, -0.5f,  1.0f, 0.0f, 3.0f, // top-right
	 -0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 3.0f, // top-left
	 -0.5f, -0.5f,  0.5f,  0.0f, 1.0f, 3.0f, // bottom-left

	 // bottom face
	 -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 4.0f, // top-right
	  0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 4.0f, // top-left
	  0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 4.0f, // bottom-left
	 -0.5f, -0.5f,  0.5f,  0.0f, 1.0f, 4.0f, // bottom-right

	 // top face
	 -0.5f,  0.5f, -0.5f,  0.0f, 0.0f, 5.0f, // top-left
	  0.5f,  0.5f, -0.5f,  1.0f, 0.0f, 5.0f, // top-right
	  0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 5.0f, // bottom-right
	 -0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 5.0f // bottom-left
};

GLuint blockIndices[] = {
	// front face
	0, 1, 2, 0, 2, 3,
	// back face
	4, 5, 6, 4, 6, 7,
	// right face
	8, 9, 10, 8, 10, 11,
	// left face
	12, 13, 14, 12, 14, 15,
	// bottom face
	16, 17, 18, 16, 18, 19,
	// top face
	20, 21, 22, 20, 22, 23
};

