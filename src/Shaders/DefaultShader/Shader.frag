R"END(
#version 410 core

in vec2 texCoord;

out vec4 FragColor;

uniform sampler2D tex0;
uniform bool highlighted;
uniform bool gamePaused;

const float BORDER_SIZE = .05f;

void main() {
    FragColor = texture(tex0, texCoord);

    if (highlighted && !(texCoord.x < (1 - BORDER_SIZE) && texCoord.x > BORDER_SIZE &&
       texCoord.y < (1 - BORDER_SIZE) && texCoord.y > BORDER_SIZE)) {

        FragColor = vec4(FragColor.rgb, 1.0f);
        FragColor *= vec4(.25f, .25f, .25f, 1.0f);
    }

    if (gamePaused) {
        FragColor *= vec4(.5f, .5f, .5f, 1.0f);
    }
}

)END"