R"END(
#version 410 core

in vec2 texCoord;

out vec4 FragColor;

uniform sampler2D tex0;
uniform vec4 guiColor;

void main() {
    FragColor = texture(tex0, texCoord);

    FragColor *= guiColor;
}
)END"