#version 450

//shader input
layout (location = 0) in vec3 inColor;
layout (location = 1) in vec2 inTexCoord;

//texture uniform
layout (binding = 0) uniform sampler2D texSampler;

//output write
layout (location = 0) out vec4 outFragColor;

void main() 
{
	//sample texture and multiply with vertex color for tinting
	vec4 texColor = texture(texSampler, inTexCoord);
	outFragColor = texColor * vec4(inColor, 1.0f);
}