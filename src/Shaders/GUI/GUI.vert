R"END(
#version 410 core

layout(location = 0) in vec2 position;
layout(location = 1) in vec2 inTexCoord;

out vec2 texCoord;

uniform vec2 guiPos;
uniform vec2 guiSize;

void main() {
	gl_Position = vec4(
		(position.x * guiSize.x * 2) + guiPos.x * 2 - 1,
		(position.y * guiSize.y * 2) - guiPos.y * 2 + 1,
		1.0f,
		1.0f
	);
	
	texCoord = inTexCoord;
}
)END"