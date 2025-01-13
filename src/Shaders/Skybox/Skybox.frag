R"END(
#version 410 core

in vec2 texCoord;
flat in float face;

out vec4 FragColor;

uniform sampler2D tex0;
uniform bool gamePaused;
uniform float ambientLight;
uniform double time;
uniform bool clouds;

void main() {
    if (!clouds) {
        FragColor = texture(tex0, vec2(texCoord.x, (texCoord.y + float(face)) / 6.0f));
    } else {
        FragColor = texture(tex0, vec2(texCoord.x, texCoord.y + (time / 120.0)) * 2.0f);
    }

    FragColor *= vec4(vec3(ambientLight), 1.0f);

    if (clouds) {
        FragColor *= sin((texCoord.x) * 3.1415);
        FragColor *= sin((texCoord.y) * 3.1415);
    }

    if (gamePaused) {
        FragColor *= vec4(.5f, .5f, .5f, 1.0f);
    }
}
)END"
