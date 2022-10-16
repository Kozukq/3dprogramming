#version 330 core

in float height;
in vec2 UV;
out vec3 color;
uniform sampler2D textureSamplerGrass;
uniform sampler2D textureSamplerRock;
uniform sampler2D textureSamplerSnowrock;

void main(){

	if(height < 0.20) {

    	color = texture(textureSamplerGrass,UV).rgb;
	}

	else if(height < 0.40) {

    	color = texture(textureSamplerRock,UV).rgb;
	}

	else {

    	color = texture(textureSamplerSnowrock,UV).rgb;
	}
}
