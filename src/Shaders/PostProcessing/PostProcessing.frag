R"END(
#version 410 core

in vec2 texCoord;

out vec4 FragColor;

uniform sampler2D tex0;

void main() {
    FragColor = texture(tex0, texCoord);

    FragColor *= (sin(texCoord.x * 3.14159) * .175f) + .825f;
    FragColor *= (sin(texCoord.y * 3.14159) * .175f) + .825f;
}
)END"