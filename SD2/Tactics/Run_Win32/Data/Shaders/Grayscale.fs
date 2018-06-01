/************************************************************************/
/* File: Grayscale.fs
/* Author: Andrew Chase
/* Date: February 27th, 2018
/* Description: Fragment Shader - Converts the sample texture color to
				grayscale
/************************************************************************/
#version 420 core										
														
// Inputs from vs		
in vec2 passUV;																			
in vec4 passColor;										
in float passLerp;

// Texture bindings  									
layout(binding = 0) uniform sampler2D gTexColor;		
														
// Outputs												
out vec4 outColor; 										
			
mat4 GRAYSCALE_TRANSFORM = mat4(
	vec4(0.2126, 0.2126, 0.2126, 0), 
	vec4(0.7152, 0.7152, 0.7152, 0),
	vec4(0.0722, 0.0722, 0.0722, 0),
	vec4(0, 	 0, 	 0, 	 1)
	);	

// Entry Point											
void main( void )										
{							
	vec4 textureColor = texture(gTexColor, passUV);
	vec4 grayscaleColor = GRAYSCALE_TRANSFORM * textureColor;														
	outColor = mix(textureColor, grayscaleColor, passLerp);
};