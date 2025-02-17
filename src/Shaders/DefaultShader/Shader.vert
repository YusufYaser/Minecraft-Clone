R"END(
#version 460 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 inTexCoord;
layout(location = 2) in float inFace;
layout(location = 3) in vec4 inBlockData;

out vec2 texCoord;
flat out float face;
flat out int instanceId;
flat out vec3 blockPosOffset;
flat out vec3 fPlayerPos;
flat out int fBlockType;
flat out float fFogSize;
flat out int fRenderDistance;

uniform mat4 projection;
uniform mat4 view;
uniform vec3 playerPos;
uniform float fogSize;
uniform int renderDistance;

void main() {
	vec3 inBlockPosOffset = inBlockData.xyz - playerPos;

    float dist = length(inBlockPosOffset) - ((renderDistance * 2 - fogSize * 2) * 8.0f);
    float val = min(max(dist / ((fogSize * 2 - 0.5f) * (16.0f / 2.0f)), 0.0f), 1.0f);

    if (val >= 1) {
		gl_Position = vec4(2.0f, 2.0f, 2.0f, 1.0f);
		return;
    }

	int blockType = int(inBlockData.w);

	vec3 pos2 = position;
	if (blockType == 7 && pos2.y > 0) {
		pos2 -= vec3(0, .05f * 2, 0);
	}
	gl_Position = projection * view * vec4(pos2 + inBlockPosOffset, 1.0f);

	texCoord = inTexCoord;
	face = inFace;
	instanceId = gl_InstanceID;
	blockPosOffset = inBlockPosOffset;
	fPlayerPos = playerPos;
	fBlockType = blockType;
	fFogSize = fogSize;
	fRenderDistance = renderDistance;
}
)END"