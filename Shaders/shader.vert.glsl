#version 330 core

layout (location=0) in vec3 pos;
layout (location=1) in vec4 col;
layout (location=2) in vec2 tex;

out vec4 vertexColor;
out vec2 texCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main(){
	vertexColor = col;
	texCoord = tex;
	gl_Position = projection * view * model  *vec4(pos, 1.0f);
}
