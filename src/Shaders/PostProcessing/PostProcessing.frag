R"END(
#version 410 core

#define PI (acos(0) * 2)

in vec2 texCoord;

out vec4 FragColor;

uniform sampler2D tex0;
uniform bool gamePaused;

void main() {
    FragColor = texture(tex0, texCoord);

    FragColor *= (sin(texCoord.x * PI) * .175f) + .825f;
    FragColor *= (sin(texCoord.y * PI) * .175f) + .825f;

    if (gamePaused) FragColor.xyz *= .5f;
}
)END"