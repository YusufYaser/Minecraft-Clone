R"END(
#version 410 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 inTexCoord;
layout(location = 2) in float inFace;
layout(location = 3) in vec3 blockPosOffset;

out vec2 texCoord;
flat out float face;
flat out int instanceId;

uniform mat4 projection;
uniform mat4 view;
uniform bool isLiquidTop;

void main() {
	vec3 pos2 = position;
	if (isLiquidTop && pos2.y > 0) {
		pos2 -= vec3(0, .05f * 2, 0);
	}
	gl_Position = projection * view * vec4(pos2 + blockPosOffset, 1.0f);

	texCoord = inTexCoord;
	face = inFace;
	instanceId = gl_InstanceID;
}
)END"