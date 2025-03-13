R"END(
#version 460 core

void main() {
	gl_Position = vec4(
		gl_VertexID % 2 == 0 ? -1.0f : 1.0f,
		gl_VertexID < 2 || gl_VertexID == 5 ? -1.0f : 1.0f,
		0, 1.0f
	);
}
)END"