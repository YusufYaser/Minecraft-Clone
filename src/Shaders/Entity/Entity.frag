R"END(
#version 460 core

out vec4 FragColor;

in vec2 fTexCoord;
in vec3 fNormal;
uniform float ambientLight;

void main() {
	FragColor = vec4(fNormal, 1.0f);

	FragColor = vec4(FragColor.xyz * ambientLight * length(fTexCoord), 1.0f);
	if (fNormal.x < 0 || fNormal.y < 0 || fNormal.z < 0) {
		FragColor = vec4(FragColor.xyz * -0.5f, 1.0f);
	}
}
)END"
