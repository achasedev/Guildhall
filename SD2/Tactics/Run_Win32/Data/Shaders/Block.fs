/************************************************************************/
/* File: Block.fs
/* Author: Andrew Chase
/* Date: February 22nd, 2017
/* Description: Fragment Shader - used to shade blocks in a map with border
				texture
/************************************************************************/
#version 420 core										
														
// Inputs from vs										
in vec2 passUV;											
in vec4 passColor;										
in vec2 passBorderUV;

// Texture bindings  									
layout(binding = 0) uniform sampler2D gBlockTexture;		
layout(binding = 1) uniform sampler2D gBorderTexture;														
														
// Outputs												
out vec4 outColor; 										
														
// Entry Point											
void main( void )										
{																					
	// Sample the two textures													
	vec4 blockColor = texture(gBlockTexture, passUV);
	vec4 borderColor = texture(gBorderTexture, passBorderUV);
	
	// Mix them together
	vec3 finalColor = mix(blockColor.xyz, borderColor.xyz, borderColor.w);

	// Multiply by the passed color tint for the final color
	outColor = vec4(finalColor.xyz, blockColor.w) * passColor;	
};