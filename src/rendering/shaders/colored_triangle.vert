#version 450

layout(set=1,binding=0)uniform PerObject{
	mat4 model;
}ubo;

layout(location=0)out vec3 outColor;
layout(location=1)out vec2 outTexCoord;

vec3 positions[3]=vec3[](
	vec3(1.,1.,0.),
	vec3(-1.,1.,0.),
	vec3(0.,-1.,0.)
);

vec3 colors[3]=vec3[](
	vec3(1.,0.,0.),
	vec3(0.,1.,0.),
	vec3(0.,0.,1.)
);

vec2 texCoords[3]=vec2[](
	vec2(1.,0.),
	vec2(0.,0.),
	vec2(.5,1.)
);

void main(){
	gl_Position=vec4(positions[gl_VertexIndex],1.)*ubo.model;
	outColor=colors[gl_VertexIndex];
	outTexCoord=texCoords[gl_VertexIndex];
}