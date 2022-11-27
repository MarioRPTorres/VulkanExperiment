#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 outColor;

//push constants block
layout( push_constant ) uniform constants
{
    vec2 iResolution;
	float iTime;
} PushConstants; 

void main() {
//outColor = vec4(1.0, 0.0, 0.0, 1.0);
    // Normalized pixel coordinates (from 0 to 1)
    vec2 uv = vec2(gl_FragCoord)/PushConstants.iResolution;

    // Time varying pixel color
    vec3 col = 0.5 + 0.5*cos(PushConstants.iTime + uv.xyx+vec3(0,2,4));

    // Output to screen
    outColor = vec4(col,1.0);
}
