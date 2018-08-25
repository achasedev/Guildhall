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
	
#define GRID_SIZE 128

in vec3 POSITION;
in vec4 INSTANCE_COLOR;
out vec4 passColor;
																								
void main( void )												
{	
	int y = gl_InstanceID / (GRID_SIZE * GRID_SIZE); // Number in each layer

	int leftOverInLayer = gl_InstanceID % (GRID_SIZE * GRID_SIZE);
	int z = leftOverInLayer / GRID_SIZE;
	int x = leftOverInLayer % GRID_SIZE;

    vec4 world_pos = vec4(vec3(x, y, z) + POSITION, 1); 						
	vec4 clip_pos = PROJECTION * VIEW * world_pos; 				
																												
	passColor = INSTANCE_COLOR;											
	gl_Position = clip_pos;  								
};