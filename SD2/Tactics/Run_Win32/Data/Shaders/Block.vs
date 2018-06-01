/************************************************************************/
/* File: Block.vs
/* Author: Andrew Chase
/* Date: February 22nd, 2017
/* Description: Vertex Shader - used to shade blocks in a map with border
				texture
/************************************************************************/
#version 420 core															
																
uniform mat4 PROJECTION;								
uniform mat4 VIEW;												
in vec3 POSITION;												
in vec4 COLOR;													
in vec2 UV;														
																
out vec2 passUV;												
out vec4 passColor;	
out vec2 passBorderUV;

// Used for pseudo UVs - index into it based on vertex's gl_VertexID (index)
vec2 borderUVs[] = { vec2(0.0), vec2(1.0, 0.0), vec2(1.0, 1.0), vec2(1.0, 0.0) };										
																													
void main(void)											
{																
	// Get the clip position								
	vec4 world_pos = vec4( POSITION, 1 ); 						
	vec4 clip_pos = PROJECTION * VIEW * world_pos; 				
	
	// Pass values to fragment shader															
	passUV = UV;												
	passColor = COLOR;	
	passBorderUV = borderUVs[gl_VertexID % 4];	

	// Set the position								
	gl_Position = clip_pos; 									
};