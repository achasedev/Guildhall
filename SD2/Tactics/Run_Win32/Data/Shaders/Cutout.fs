/************************************************************************/
/* File: Cutout.fs
/* Author: Andrew Chase
/* Date: March 23rd, 2017
/* Description: Fragment Shader - used to draw sprites, discarding low alpha
/************************************************************************/
#version 420 core											
																										
in vec2 passUV;												
in vec4 passColor;											
															  										
layout(binding = 0) uniform sampler2D gTexDiffuse;			
																											
out vec4 outColor; 											
															
// Entry Point												
void main( void )											
{																				
	vec4 diffuse = texture(gTexDiffuse, passUV);
	vec4 finalColor = diffuse * passColor;

	// If low alpha, throw out and don't draw
	if (finalColor.a <= 0.5f)
	{
		discard;
	}

	outColor = diffuse * passColor;	 						
}