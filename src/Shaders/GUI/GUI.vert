R"END(
#version 410 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 inTexCoord;

out vec2 texCoord;

uniform mat4 viewport;
uniform mat4 model;
uniform vec2 guiPos;
uniform float guiScale;

void main() {
	gl_Position = viewport * model * vec4((position.x*guiScale) + guiPos.x, (position.y*guiScale) + guiPos.y, position.z*guiScale, 1.0f);
	
	texCoord = inTexCoord;
}
)END"