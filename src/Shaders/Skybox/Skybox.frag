R"END(
#version 410 core

in vec2 texCoord;
flat in float face;
flat in int fType;

out vec4 FragColor;

uniform sampler2D tex0;
uniform float ambientLight;
uniform int tick;
uniform vec3 playerPos;

void main() {
    switch (fType) {
    case 0: // skybox
        FragColor = texture(tex0, vec2(texCoord.x, (texCoord.y + float(face)) / 6.0f));
        break;

    case 1: // clouds
        FragColor = texture(tex0, vec2(texCoord.x + (playerPos.x / 1000.0), texCoord.y + (tick / 2400.0) + (playerPos.z / 1000.0)) * .03f);
        
        float dist = length(texCoord - vec2(.5f));

        FragColor *= .75f;
        FragColor *= 1.0f - (2.0f * dist);
        break;

    case 2: // sun
        FragColor = texture(tex0, vec2(texCoord.x, texCoord.y));
        break;
    }

    if (fType != 2) FragColor *= vec4(vec3(ambientLight), 1.0f);
}
)END"
