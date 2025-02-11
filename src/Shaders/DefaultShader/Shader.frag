R"END(
#version 410 core

in vec2 texCoord;
flat in float face;
flat in int instanceId;
flat in vec3 blockPosOffset;

out vec4 FragColor;

uniform sampler2D tex0;
uniform int highlighted;
uniform float ambientLight;
uniform double time;
uniform int animationFrameCount;
uniform int renderDistance;
uniform float fogSize;

const float BORDER_SIZE = .02f;

void main() {
    FragColor = texture(tex0, vec2((texCoord.x + float(int(time) % animationFrameCount)) / float(animationFrameCount), (texCoord.y + float(face)) / 6.0f));

    if (highlighted == instanceId && !(texCoord.x < (1 - BORDER_SIZE) && texCoord.x > BORDER_SIZE &&
       texCoord.y < (1 - BORDER_SIZE) && texCoord.y > BORDER_SIZE)) {

        FragColor = vec4(FragColor.rgb, 1.0f);
        FragColor *= vec4(.25f, .25f, .25f, 1.0f);
    }

    float lightPercentage;

    if (face == 5) { // top face
        lightPercentage = 1.0f;
    } else if (face == 4) { // bottom face
        lightPercentage = .5f;
    } else { // sides
        lightPercentage = .75f;
    }

    FragColor *= vec4(vec3(ambientLight * lightPercentage), 1.0f);

    vec4 SkyColor = vec4(vec3(78.0f / 255.0f, 78.0f / 255.0f, 251.0f / 255.0f) * ambientLight, 1.0f);

    float dist = length(blockPosOffset) - ((renderDistance - fogSize * 2) * 8.0f);
    float val = min(max(dist / ((fogSize * 2 - 0.5f) * (16.0f / 2.0f)), 0.0f), 1.0f);
    FragColor = mix(FragColor, SkyColor, val);
}
)END"
