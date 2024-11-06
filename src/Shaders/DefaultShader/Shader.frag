R"END(
#version 410 core

in vec3 color;
in vec2 texCoord;

out vec4 FragColor;

uniform sampler2D tex0;

void main() {
    FragColor = texture(tex0, texCoord);
}

)END"