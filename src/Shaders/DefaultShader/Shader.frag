R"END(
#version 410 core

in vec2 texCoord;

out vec4 FragColor;

uniform sampler2D tex0;
uniform bool highlighted;

void main() {
    FragColor = texture(tex0, texCoord);

    if (highlighted && !(texCoord.x < .95f && texCoord.x > .05f &&
       texCoord.y < .95f && texCoord.y > .05f)) {

        FragColor *= vec4(.25f, .25f, .25f, 1.0f);
    }
}

)END"