#version 450
#extension GL_EXT_buffer_reference:require

struct Vertex{
	vec4 position;
};

layout(buffer_reference,std430)readonly buffer VertexBuffer{
	Vertex vertices[];
};

layout(set=1,binding=0)uniform PerObject{
	mat4 model;
}ubo;

layout(push_constant)uniform constants{
	VertexBuffer vertexBuffer;
}PushConstants;

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
	Vertex v=PushConstants.vertexBuffer.vertices[gl_VertexIndex];
	gl_Position=v.position*ubo.model;
	outColor=colors[gl_VertexIndex];
	outTexCoord=texCoords[gl_VertexIndex];
}