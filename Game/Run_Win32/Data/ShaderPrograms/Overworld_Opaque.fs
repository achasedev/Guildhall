#version 420 core											
																											
in vec2 passUV;												
in vec4 passLightValues;
in vec3 passCameraPosition;

layout(binding = 0) uniform sampler2D gTexDiffuse;			

layout(binding=9, std140) uniform fogUBO
{
	float FOG_MIN_DISTANCE;
	float FOG_MAX_DISTANCE;
	float FOG_MIN_FACTOR;
	float FOG_MAX_FACTOR;
};

																											
out vec4 outColor; 											
	
//-----------------------------------------------------------------------------
vec4 ApplyFog(vec4 finalColor, float fragDepth)
{
	float fogFactor = smoothstep(FOG_MIN_DISTANCE, FOG_MAX_DISTANCE, fragDepth);
	fogFactor = FOG_MIN_FACTOR + (FOG_MAX_FACTOR - FOG_MIN_FACTOR) * fogFactor;

	finalColor = mix(finalColor, vec4(1.f), fogFactor);

	return finalColor;
}


//-----------------------------------------------------------------------------
vec4 ApplyLighting(vec4 finalColor)
{
	return finalColor;
}



//-----------------------------------------------------------------------------
void main( void )											
{																																			
	vec4 diffuse = texture(gTexDiffuse, passUV);	
	vec4 finalColor = diffuse;
	vec4 finalColorWithLighting = ApplyLighting(finalColor);
	
	outColor = ApplyFog(finalColorWithLighting, passCameraPosition.z);	 				
}
