R"END(
#version 410 core

layout(location = 0) in vec3 position;

uniform mat4 viewport;

void main() {
	gl_Position = viewport * vec4(position, 1.0f);
}
)END"