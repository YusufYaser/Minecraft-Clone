R"END(
#version 410 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 inTexCoord;
layout(location = 2) in float inFace;

out vec2 texCoord;

uniform mat4 projection;
uniform mat4 view;

void main() {
	gl_Position = projection * view * vec4(position, 1.0f);

	texCoord = inTexCoord;
}
)END"