#version 450 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 inTexCoord;
layout(location = 2) in float inFace;
layout(location = 3) in vec3 inBlockPos;
layout(location = 4) in uint inBlockType;
layout(location = 5) in vec3 inExtend;

out vec2 texCoord;
out vec3 vertex;
flat out float face;
flat out int instanceId;
flat out vec3 blockPosOffset;
flat out vec3 fPlayerPos;
flat out uint fBlockType;
flat out float fFogSize;
flat out int fRenderDistance;
flat out vec3 fExtend;

uniform mat4 projection;
uniform mat4 view;
uniform vec3 playerPos;
uniform float fogSize;
uniform int renderDistance;

void main() {
	vec3 inBlockPosOffset = inBlockPos - playerPos;

    float dist = length(inBlockPosOffset) - ((renderDistance * 2 - fogSize * 2) * 8.0f);
    float val = min(max(dist / ((fogSize * 2 - 0.5f) * (16.0f / 2.0f)), 0.0f), 1.0f);

    if (val >= 1) {
		gl_Position = vec4(0/0);
		return;
    }

	vec3 pos2 = position;
	if (inBlockType == 7 && pos2.y > 0) {
		pos2 -= vec3(0, .05f * 2, 0);
	}

	vec3 extend = vec3(0);
	if (inExtend.x > 0 && pos2.x > 0) extend.x += inExtend.x;
	if (inExtend.z > 0 && pos2.z > 0) extend.z += inExtend.z;
	gl_Position = projection * view * vec4(pos2 + inBlockPosOffset + extend, 1.0f);

	texCoord = inTexCoord;
	face = inFace;
	instanceId = gl_InstanceID;
	blockPosOffset = inBlockPosOffset;
	fPlayerPos = playerPos;
	fBlockType = inBlockType;
	fFogSize = fogSize;
	fRenderDistance = renderDistance;
	fExtend = inExtend;
	vertex = position;
}
