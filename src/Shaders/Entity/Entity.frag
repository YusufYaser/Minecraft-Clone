R"END(
#version 460 core

out vec4 FragColor;

uniform float ambientLight;

void main() {
	FragColor = vec4(1.0f);

	FragColor = vec4(FragColor.xyz * ambientLight, 1.0f);
}
)END"
