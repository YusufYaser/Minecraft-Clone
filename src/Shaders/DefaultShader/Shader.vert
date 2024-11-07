R"END(
#version 410 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 inTexCoord;

out vec3 color;
out vec2 texCoord;

uniform mat4 projection;
uniform mat4 view;
uniform vec3 blockPos;

void main() {
	gl_Position = projection * view * vec4(position + blockPos, 1.0f);

	texCoord = inTexCoord;
}
)END"