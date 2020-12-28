#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inline1;
layout(location = 3) in vec3 inline2;
layout(location = 4) in vec3 inline3;

layout(location = 0) out vec3 fragColor;

void main(){
	mtrans = mat3(inline1,inline2,inline3);
	gl_Position = vec4( vec2(vec3(inPosition,1.0)*mtrans), 0.0,1.0);
	fragColor = inColor;
}