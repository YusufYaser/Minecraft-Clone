R"END(
#version 460 core

#define PI (acos(0) * 2)

in vec2 texCoord;

out vec4 FragColor;

layout(binding = 0) uniform sampler2D colorTex;
layout(binding = 1) uniform sampler2D depthTex;

void main() {
    FragColor = texture(colorTex, texCoord);

    FragColor *= (sin(texCoord.x * PI) * .175f) + .825f;
    FragColor *= (sin(texCoord.y * PI) * .175f) + .825f;
}
)END"