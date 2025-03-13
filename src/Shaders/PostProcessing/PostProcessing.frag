R"END(
#version 460 core

#define PI (acos(0) * 2)

out vec4 FragColor;

uniform ivec2 resolution;

layout(binding = 0) uniform sampler2D colorTex;
layout(binding = 1) uniform sampler2D depthTex;

void main() {
    vec2 texCoord = gl_FragCoord.xy / vec2(resolution);

    FragColor = texture(colorTex, texCoord);

    FragColor *= (sin(texCoord.x * PI) * .175f) + .825f;
    FragColor *= (sin(texCoord.y * PI) * .175f) + .825f;
}
)END"