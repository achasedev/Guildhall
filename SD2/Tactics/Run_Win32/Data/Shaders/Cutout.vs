/************************************************************************/
/* File: Cutout.vs
/* Author: Andrew Chase
/* Date: March 23rd, 2017
/* Description: Vertex Shader - paired with discard alpha fragment shader
/************************************************************************/
#version 420 core												

uniform mat4 MODEL;																	
uniform mat4 PROJECTION;										
uniform mat4 VIEW;												
in vec3 POSITION;												
in vec4 COLOR;													
in vec2 UV;														
																
out vec2 passUV;												
out vec4 passColor;												
																													
void main( void )												
{																										
	vec4 world_pos = vec4( POSITION, 1 ); 						
	vec4 clip_pos = PROJECTION * VIEW * MODEL * world_pos; 				
																
	passUV = UV;												
	passColor = COLOR;											
	gl_Position = clip_pos; 									
}