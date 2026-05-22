#version 450
#extension GL_EXT_buffer_reference:require

struct Vertex{
	vec4 position;
	vec4 color;
};

layout(buffer_reference,std430)readonly buffer VertexBuffer{
	Vertex vertices[];
};

layout(set=0,binding=0)uniform Global{
	mat4 projection;
}global;

layout(push_constant)uniform constants{
	VertexBuffer vertexBuffer;
}PushConstants;

layout(location=0)out vec4 outColor;

void main(){
	Vertex v=PushConstants.vertexBuffer.vertices[gl_VertexIndex];
	gl_Position=global.projection*v.position;
	outColor=v.color;
}
