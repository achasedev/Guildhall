/************************************************************************/
/* File: Grayscale.vs
/* Author: Andrew Chase
/* Date: February 27th, 2018
/* Description: Vertex Shader - Converts position into clip space normally
/************************************************************************/
#version 420 core															
																
uniform mat4 PROJECTION;								
uniform mat4 VIEW;	

uniform float LERP;

in vec3 POSITION;												
in vec4 COLOR;													
in vec2 UV;														
																
out vec2 passUV;												
out vec4 passColor;	
out float passLerp;
																													
void main(void)											
{																
	// Get the clip position								
	vec4 world_pos = vec4( POSITION, 1 ); 						
	vec4 clip_pos = PROJECTION * VIEW * world_pos; 				
	
	// Pass values to fragment shader															
	passUV = UV;												
	passColor = COLOR;
	passLerp = LERP;

	// Set the position								
	gl_Position = clip_pos; 									
};