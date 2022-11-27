#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in flat int fragTexId;

layout(binding = 1) uniform sampler2D texSampler[2];

layout(location = 0) out vec4 outColor;


void main() {
	if (fragTexCoord==vec2(0.0,0.0)){
		outColor = vec4(fragColor, 1.0);
	}
	else {
		outColor = texture(texSampler[fragTexId],fragTexCoord);
	}
	//outColor = vec4(fragTexCoord,0.0,1.0);
	//outColor = vec4(fragColor * texture(texSampler,fragTexCoord).rgb, 1.0);
}