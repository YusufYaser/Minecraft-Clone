R"END(
#version 410 core

in vec2 texCoord;
flat in float face;
flat in int fType;

out vec4 FragColor;

uniform sampler2D tex0;
uniform bool gamePaused;
uniform float ambientLight;
uniform int tick;
uniform vec3 playerPos;

void main() {
    switch (fType) {
    case 0: // skybox
        FragColor = texture(tex0, vec2(texCoord.x, (texCoord.y + float(face)) / 6.0f));
        break;

    case 1: // clouds
        FragColor = texture(tex0, vec2(texCoord.x + (playerPos.x / 1000.0), texCoord.y + (tick / 2400.0) + (playerPos.z / 1000.0)));
        FragColor *= sin((texCoord.x) * 3.1415);
        FragColor *= sin((texCoord.y) * 3.1415);
        FragColor *= .75f;
        break;

    case 2: // sun
        FragColor = texture(tex0, vec2(texCoord.x, texCoord.y));
        break;
    }

    if (fType != 2) FragColor *= vec4(vec3(ambientLight), 1.0f);

    if (gamePaused) {
        FragColor *= vec4(.5f, .5f, .5f, 1.0f);
    }
}
)END"
