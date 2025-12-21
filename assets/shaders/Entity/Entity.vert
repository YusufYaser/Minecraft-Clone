#version 450 core

layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 texCoord;
layout(location = 2) in vec3 normal;

out vec2 fTexCoord;
out vec3 fNormal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
	gl_Position = projection * view * model * vec4(pos, 1.0f);

	fTexCoord = texCoord;
	fNormal = normal;
}
