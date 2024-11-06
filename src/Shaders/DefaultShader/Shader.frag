R"END(
#version 410 core

layout(location = 0) in vec3 inColor;

layout(location = 0) out vec4 color;

void main() {
    color = vec4(1, 1, 1, 1);
    color.rgb = inColor;
}

)END"