#pragma once

#define VERTEX_SIZE 6

inline GLfloat blockVertices[] = {
	// positions          // texture coords // face number
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
	 0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 2.0f, // bottom-left
	 0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 2.0f, // top-left
	 0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 2.0f, // top-right
	 0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 2.0f, // bottom-right

	 // left face
	 -0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 3.0f, // bottom-right
	 -0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 3.0f, // top-right
	 -0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 3.0f, // top-left
	 -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 3.0f, // bottom-left

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

inline uint8_t blockIndices[] = {
	0, 1, 2, 0, 2, 3
};
