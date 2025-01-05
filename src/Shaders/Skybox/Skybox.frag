R"END(
#version 410 core

in vec2 texCoord;
flat in float face;

out vec4 FragColor;

uniform sampler2D tex0;
uniform bool gamePaused;
uniform float ambientLight;

void main() {
    FragColor = texture(tex0, vec2(texCoord.x, (texCoord.y + float(face)) / 6.0f));

    FragColor *= vec4(vec3(ambientLight), 1.0f);

    if (gamePaused) {
        FragColor *= vec4(.5f, .5f, .5f, 1.0f);
    }
}
)END"
