/************************************************************************/
/* File: Copy.fs
/* Author: Andrew Chase
/* Date: September 8th, 2018
/* Description: Fragment Shader - Samples and draws
/************************************************************************/
#version 420 core										
														
// Inputs from vs		
in vec2 passUV;																			

// Texture bindings  									
layout(binding = 0) uniform sampler2D gTexColor;		
														
// Outputs												
out vec4 outColor; 										
			
// Entry Point											
void main( void )										
{							
	vec4 textureColor = texture(gTexColor, passUV);
	outColor = textureColor;
};