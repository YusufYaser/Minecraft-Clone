R"END(
#version 450 core

out vec4 FragColor;

in vec2 fTexCoord;
in vec3 fNormal;

uniform sampler2D tex0;
uniform float ambientLight;

void main() {
	FragColor = texture(tex0, fTexCoord);

	FragColor = vec4(FragColor.xyz * ambientLight, 1.0f);
}
)END"
