#version 420 core											
																																																
in vec3 passWorldPosition;

layout(binding = 0) uniform samplerCube gTexSky;			
																											
out vec4 outColor; 											
															
// Entry Point												
void main( void )											
{	
   	vec3 normal = normalize(passWorldPosition); 
	vec4 tex_color = texture(gTexSky, normal);	
	outColor = tex_color;	 				
}
