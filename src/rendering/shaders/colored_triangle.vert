#version 450
#extension GL_EXT_buffer_reference:require

struct Vertex{
	vec4 position;
	vec2 tex;
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

layout(location=0)out vec2 outTexCoord;

void main(){
	Vertex v=PushConstants.vertexBuffer.vertices[gl_VertexIndex];
	gl_Position=ubo.model*v.position;
	outTexCoord=v.tex;
}