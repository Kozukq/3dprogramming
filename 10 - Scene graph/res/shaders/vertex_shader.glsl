#version 330 core

layout(location = 0) in vec3 vertices;
layout(location = 1) in vec2 _uv;
uniform mat4 MVP;
out vec2 uv;

void main() {

	gl_Position = vec4(vertices,1);
	
	gl_Position = MVP * gl_Position;

	uv = _uv;
}