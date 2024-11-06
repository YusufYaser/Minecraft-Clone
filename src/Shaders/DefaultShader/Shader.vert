R"END(
#version 410 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;

layout(location = 0) out vec3 outColor;

uniform mat4 projection;
uniform mat4 view;
uniform vec3 blockPos;

void main() {
	gl_Position = projection * view * vec4(position + blockPos, 1.0f);

	outColor = color;
}
)END"