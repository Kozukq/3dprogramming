#version 330 core

layout(location = 0) in vec3 vertices_position_modelspace;
layout(location = 1) in vec2 vertexUV;
out vec2 UV;
out float height;
uniform mat4 MVP;
uniform sampler2D hmapSampler;

void main(){

	gl_Position = vec4(vertices_position_modelspace,1);
	gl_Position.z = texture(hmapSampler,vertexUV).r;
	height = gl_Position.z;
	gl_Position = MVP * gl_Position;
	UV = vertexUV;
}

