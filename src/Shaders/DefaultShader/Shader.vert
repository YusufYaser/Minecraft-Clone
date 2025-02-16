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

uniform mat4 projection;
uniform mat4 view;
uniform vec3 playerPos;

void main() {
	int blockType = int(inBlockData.w);
	vec3 inBlockPosOffset = inBlockData.xyz - playerPos;

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
}
)END"