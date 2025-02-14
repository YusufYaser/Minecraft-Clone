R"END(
#version 460 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 inTexCoord;
layout(location = 2) in float inFace;

out vec2 texCoord;
flat out float face;
flat out int fType;

uniform mat4 model;
uniform mat4 projection;
uniform mat4 view;
uniform vec3 blockPos;
uniform int type;

void main() {
	if (type != 2) {
		gl_Position = projection * view * vec4(position + blockPos, 1.0f);
	} else { // sun
		vec3 pos2 = position;
		pos2.x *= .075f;
		pos2.z *= .075f;
		gl_Position = projection * view * model * vec4(pos2 + blockPos, 1.0f);
	}

	texCoord = inTexCoord;
	face = inFace;
	fType = type;
}
)END"