R"END(
#version 410 core

in vec2 texCoord;
in float face;

out vec4 FragColor;

uniform sampler2D tex0;
uniform bool highlighted;
uniform bool gamePaused;
uniform float ambientLight;

const float BORDER_SIZE = .05f;

void main() {
    FragColor = texture(tex0, texCoord);

    if (highlighted && !(texCoord.x < (1 - BORDER_SIZE) && texCoord.x > BORDER_SIZE &&
       texCoord.y < (1 - BORDER_SIZE) && texCoord.y > BORDER_SIZE)) {

        FragColor = vec4(FragColor.rgb, 1.0f);
        FragColor *= vec4(.25f, .25f, .25f, 1.0f);
    }

    float lightPercentage;

    if (face == 5) { // top face
        lightPercentage = 1.0f;
    } else if (face == 4) { // bottom face
        lightPercentage = .25f;
    } else { // sides
        lightPercentage = .5f;
    }

    FragColor *= vec4(vec3(ambientLight * lightPercentage), 1.0f);

    if (gamePaused) {
        FragColor *= vec4(.5f, .5f, .5f, 1.0f);
    }
}

)END"