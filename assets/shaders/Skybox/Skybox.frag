#version 450 core

#define PI (acos(0) * 2)

in vec2 texCoord;
flat in float face;
flat in int fType;

out vec4 FragColor;

layout(binding = 0) uniform sampler2D tex0;
layout(binding = 1) uniform sampler2D tex1;
uniform float ambientLight;
uniform int tick;
uniform vec3 playerPos;

void main() {
    switch (fType) {
    case 0: // skybox
        vec4 color1 = texture(tex0, vec2(texCoord.x, (texCoord.y + float(face)) / 6.0f));
        vec4 color2 = texture(tex1, vec2(texCoord.x, (texCoord.y + float(face)) / 6.0f));
        float transition = (ambientLight - 0.1) / 0.9;
        FragColor = mix(color2, color1, transition);
        break;

    case 1: // clouds
        FragColor = texture(tex0, vec2(texCoord.x + (playerPos.x / 1000.0), texCoord.y + (tick / 2400.0) - (playerPos.z / 1000.0)) * .03f);
        
        float dist = length(texCoord - vec2(.5f));

        FragColor *= .75f;
        FragColor *= 1.0f - (2.0f * dist);
        FragColor *= vec4(vec3(ambientLight), 1.0f);

        FragColor *= sin(max(min((156 - playerPos.y) / 4.0f, 1.0f), 0.0f) * PI/2.0f);
        break;

    case 2: // sun
        FragColor = texture(tex0, vec2(texCoord.x, texCoord.y));
        break;
    }
}
