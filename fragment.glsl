// ==========================================================================
// Vertex program for barebones GLFW boilerplate
//
// Author:  Sonny Chan, University of Calgary
// Date:    December 2015
// ==========================================================================
#version 410

// interpolated colour received from vertex stage

// first output is mapped to the framebuffer's colour index by default
out vec4 FragmentColour;

in vec3 FragNormal;
in vec2 FragUV;
in vec4 worldPos;

uniform sampler2D sphereTex;
uniform bool isDiffuse;

void main(void)
{
	vec4 planetCol = texture(sphereTex, FragUV);
	if(isDiffuse){
		vec4 sunCol = vec4(1);
		vec3 lightRay = normalize(vec3(0) - worldPos.xyz);
		FragmentColour = planetCol * sunCol * max(0.1, dot(FragNormal, lightRay));
	}
	else
		FragmentColour = planetCol;
}
