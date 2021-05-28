#version 330 core

in vec4 vertexColor;
in vec2 texCoord;

uniform sampler2D diffuseMap;
uniform sampler2D transparency;
out vec4 outColor;

void main(){
	/*if (gl_FragCoord.x < 800) {*/
		vec4 texColor = texture(diffuseMap, texCoord);
		vec4 trans = texture(transparency, texCoord);
		    if(trans.a < 0.5)
		        discard;
		outColor = trans;
}
