R"END(
#version 460 core
precision highp float;

#define PI (acos(0) * 2)

out vec4 FragColor;

uniform ivec2 resolution;
uniform bool guiEnabled;

layout(binding = 0) uniform sampler2D colorTex;
layout(binding = 1) uniform sampler2D depthTex;
layout(binding = 2) uniform sampler3D colorLUT;

void main() {
    vec2 texCoord = gl_FragCoord.xy / vec2(resolution);

    vec3 color = texture(colorTex, texCoord).rgb;
    FragColor = texture(colorLUT, color);

    if (guiEnabled) {
        FragColor *= (sin(texCoord.x * PI) * .175f) + .825f;
        FragColor *= (sin(texCoord.y * PI) * .175f) + .825f;
    }
}
)END"