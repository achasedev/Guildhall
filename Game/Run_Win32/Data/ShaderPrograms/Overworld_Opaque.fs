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


layout(binding=10, std140) uniform lightColorUBO
{
	vec3 	INDOOR_LIGHT_COLOR;
	float 	LIGHT_UBO_PADDING_0;
	vec3 	OUTDOOR_LIGHT_COLOR;
	float 	LIGHT_UBO_PADDING_1;
	vec3 	SKY_COLOR;
	float 	LIGHT_UBO_PADDING_2;
};

																											
out vec4 outColor; 											
	
//-----------------------------------------------------------------------------
vec4 ApplyFog(vec4 finalColor, float fragDistanceFromCamera)
{
	float fogFactor = smoothstep(FOG_MIN_DISTANCE, FOG_MAX_DISTANCE, fragDistanceFromCamera);
	fogFactor = FOG_MIN_FACTOR + (FOG_MAX_FACTOR - FOG_MIN_FACTOR) * fogFactor;

	finalColor = mix(finalColor, vec4(SKY_COLOR, 1.0f), fogFactor);

	return finalColor;
}


//-----------------------------------------------------------------------------
vec4 ApplyLighting(vec4 finalColor)
{
	vec4 indoorExposure = passLightValues.x * vec4(INDOOR_LIGHT_COLOR, 1.0f);
	vec4 outdoorExposure = passLightValues.y * vec4(OUTDOOR_LIGHT_COLOR, 1.0f);

	vec4 finalLightColor = max(indoorExposure, outdoorExposure);

	return finalColor * finalLightColor;
}



//-----------------------------------------------------------------------------
void main( void )											
{																																			
	vec4 diffuse = texture(gTexDiffuse, passUV);	
	vec4 finalColor = diffuse;
	vec4 finalColorWithLighting = ApplyLighting(finalColor);
	
	float fragDistanceFromCamera = length(passCameraPosition);
	outColor = ApplyFog(finalColorWithLighting, fragDistanceFromCamera);	 				
}
