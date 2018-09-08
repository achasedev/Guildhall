/************************************************************************/
/* File: Copy.vs
/* Author: Andrew Chase
/* Date: September 8th, 2018
/* Description: Vertex Shader - Converts position into clip space normally
/************************************************************************/
#version 420 core															

in vec3 POSITION;												
in vec2 UV;														

out vec2 passUV;												
																													
void main(void)											
{																				
	// Pass values to fragment shader															
	passUV = UV;												

	// Set the position								
	gl_Position = vec4(POSITION, 1.0f); 									
};