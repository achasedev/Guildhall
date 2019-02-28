#version 420 core												
	
layout(binding=1, std140) uniform cameraUBO
{
	mat4 VIEW;
	mat4 PROJECTION;

	mat4 CAMERA_MATRIX;

	vec3	CAMERA_RIGHT;
	float	PADDING_0;
	vec3	CAMERA_UP;
	float	PADDING_1;
	vec3	CAMERA_FORWARD;
	float	PADDING_2;
	vec3	CAMERA_POSITION;
	float	PADDING_3;
};
																											
in vec3 POSITION;												
in vec4 COLOR;													
in vec2 UV;														
																
out vec2 passUV;												
out vec4 passLightValues;												
out vec3 passCameraPosition;

void main( void )												
{																										
	vec4 worldPosition = vec4( POSITION, 1 ); 						
	vec4 clip_pos = PROJECTION * VIEW * worldPosition; 				
																
	passUV = UV;												
	passLightValues = COLOR;
	passCameraPosition = (VIEW * worldPosition).xyz;

	gl_Position = clip_pos; 								
}