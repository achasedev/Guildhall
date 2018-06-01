#version 420 core												

layout(binding=1, std140) uniform cameraUBO
{
	mat4 VIEW;
	mat4 PROJECTION;
};

layout(binding=2, std140) uniform modelUBO
{
	mat4 MODEL;
};
																											
in vec3 POSITION;																																					
																
out vec3 passWorldPosition;																							

void main( void )												
{																										
	// 1, since I don't want to translate
   	vec4 local_pos = vec4(POSITION, 0.0f);	

   	vec4 world_pos = MODEL * local_pos;
   	vec4 clip_pos = PROJECTION * VIEW * world_pos; 				
																
	passWorldPosition = world_pos;												

	gl_Position = clip_pos.xyww; 								
}
