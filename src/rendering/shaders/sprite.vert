#version 450
#extension GL_EXT_buffer_reference:require

struct Vertex{
	vec4 position;
	vec2 tex;
};

layout(buffer_reference,std430)readonly buffer VertexBuffer{
	Vertex vertices[];
};

layout(set=0,binding=0)uniform Global{
	mat4 projection;
}global;

layout(set=1,binding=1)uniform PerObject{
	mat4 model;
}perObject;

layout(push_constant)uniform constants{
	VertexBuffer vertexBuffer;
}PushConstants;

layout(location=0)out vec2 outTexCoord;

void main(){
	Vertex v=PushConstants.vertexBuffer.vertices[gl_VertexIndex];
	gl_Position=global.projection*perObject.model*v.position;
	outTexCoord=v.tex;
}