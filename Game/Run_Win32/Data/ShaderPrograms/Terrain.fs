#version 420 core											
	#define MAX_LIGHTS 8
																										
	layout(binding = 0) uniform sampler2D gTexDiffuse;			
	layout(binding = 1) uniform sampler2D gTexNormal;
	layout(binding = 8) uniform sampler2D gShadowDepth;

	struct Light
	{
		vec3 m_position;
		float m_dotOuterAngle;
		vec3 m_direction;
		float m_dotInnerAngle;
		vec3 m_attenuationFactors;
		float m_directionFactor;
		vec4 m_color;
		mat4 m_shadowVP;
		vec3 m_padding;
		float m_castsShadows;
	};
	
	layout(binding=3, std140) uniform lightUBO
	{
		vec4 AMBIENT;							// xyz color, w intensity
		Light LIGHTS[MAX_LIGHTS];
	};	

	layout(binding=8, std140) uniform specularUBO
	{
		float SPECULAR_AMOUNT;
		float SPECULAR_POWER;
		vec2 PADDING_4;
	};
	
	in vec2 passUV;												
	in vec4 passColor;											
	
	in vec3 passEyePosition;
	in vec3 passWorldPosition;
	in mat4 passTBNTransform;
	
	out vec4 outColor;
	
	vec2 poissonDisk[4] = vec2[](
  		vec2( -0.94201624, -0.39906216 ),
  		vec2( 0.94558609, -0.76890725 ),
  		vec2( -0.094184101, -0.92938870 ),
  		vec2( 0.34495938, 0.29387760 )
	);

	//---------------------------------------------------Functions-----------------------------------------------------------------
	
	// Calculates the normal given the surface normal color sample and the TBN transformation
	vec3 CalculateWorldNormal(vec4 color)
	{
		// Range map the values (Z should be between 0.5 and 1.0 always
		vec3 surfaceNormal = normalize(2.f * color.xyz - vec3(1));
		vec3 worldNormal = (passTBNTransform * vec4(surfaceNormal, 0.f)).xyz;
	
		return worldNormal;
	}
	
	// Calculates the attenuation factor for the given light information
	float CalculateAttenuation(vec3 lightPosition, vec3 attenuationFactors, float intensity)
	{
		float distance = length(lightPosition - passWorldPosition);
		float denominator = attenuationFactors.x + attenuationFactors.y * distance + attenuationFactors.z * distance * distance;
		float attenuation = (intensity / denominator);
	
		return attenuation;
	} 	
	
	
	// Calculates the cone factor (= 1.f for directional and point lights, 0.f <= x <= 1.f for spot lights)
	float CalculateConeFactor(vec3 lightPosition, vec3 lightDirection, float outerDotThreshold, float innerDotThreshold)
	{
		float dotFactor = dot(normalize(passWorldPosition - lightPosition), lightDirection);
		float coneFactor = smoothstep(outerDotThreshold, innerDotThreshold, dotFactor);
	
		return coneFactor;
	}			
	
	
	// Calculates the diffuse factor for the 
	vec3 CalculateDot3(vec3 directionToLight, vec3 normal, vec4 lightColor, float attenuation, float coneFactor)
	{
		vec3 dot3 = (max(0.f, dot(directionToLight, normal)) * lightColor.xyz * lightColor.w * attenuation * coneFactor);
		return dot3;
	}
	
	vec3 CalculateSpecular(vec3 directionToLight, vec3 normal, vec3 directionToEye, vec4 lightColor, float attenuation, float coneFactor)
	{
		vec3 reflect = reflect(-directionToLight, normal);
		float factor = max(0, dot(directionToEye, reflect));
		vec3 specular = SPECULAR_AMOUNT * pow(factor, SPECULAR_POWER) * lightColor.xyz * lightColor.w * attenuation * coneFactor;
		
		return specular;
	}
	
	float CalculateShadowFactor(vec3 fragPosition, vec3 normal, Light light)
	{
		if (light.m_castsShadows == 0.f)
		{
			return 1.0f;
		}

		vec4 clipPos = light.m_shadowVP * vec4(fragPosition, 1.0f);
		vec3 ndcPos = clipPos.xyz / clipPos.w;

		ndcPos = (ndcPos + vec3(1)) * 0.5f;

		float shadowDepth = texture(gShadowDepth, ndcPos.xy).r;

		float bias = 0.0001;
		return ndcPos.z - bias > shadowDepth ? 0.f : 1.f;
	}
	
	// Entry point															
	void main( void )											
	{				
		//----------------------------SET UP VALUES-------------------------------		
		vec4 surfaceColor = texture(gTexDiffuse, passUV);
		vec3 directionToEye = normalize(passEyePosition - passWorldPosition);
	
		// Get the normal from the normal map, and transform it into TBN space
		vec4 normalColor = texture(gTexNormal, passUV);
		vec3 worldNormal = CalculateWorldNormal(normalColor);	
	
		// Calculate the direction TO light, attenuation, and cone factor for each light
		// Set up accumulation variables
		vec3 surfaceLight = vec3(0);	// How much light is hitting the surface
		vec3 reflectedLight = vec3(0);	// How much light is being reflected back
	
		//----------STEP 1: Add in the ambient light to the surface light----------
		surfaceLight = AMBIENT.xyz * AMBIENT.w;
	
		for (int lightIndex = 0; lightIndex < MAX_LIGHTS; ++lightIndex)
		{
			// Directions to the light
			vec3 directionToLight = mix(-LIGHTS[lightIndex].m_direction, normalize(LIGHTS[lightIndex].m_position - passWorldPosition), LIGHTS[lightIndex].m_directionFactor);
	
			// Attenuation
			float attenuation = CalculateAttenuation(LIGHTS[lightIndex].m_position, LIGHTS[lightIndex].m_attenuationFactors, LIGHTS[lightIndex].m_color.w);
	
			// Cone factor
			float coneFactor = CalculateConeFactor(LIGHTS[lightIndex].m_position, LIGHTS[lightIndex].m_direction, LIGHTS[lightIndex].m_dotOuterAngle, LIGHTS[lightIndex].m_dotInnerAngle);
	
	
			//-------------STEP 2: Add in the diffuse light from all lights------------	
			float shadowFactor = CalculateShadowFactor(passWorldPosition, worldNormal, LIGHTS[lightIndex]);

			surfaceLight += shadowFactor * CalculateDot3(directionToLight, worldNormal, LIGHTS[lightIndex].m_color, attenuation, coneFactor);
			
			//-----STEP 3: Calculate and add in specular lighting from all lights----------
			reflectedLight += shadowFactor * CalculateSpecular(directionToLight, worldNormal, directionToEye, LIGHTS[lightIndex].m_color, attenuation, coneFactor);
		}
	
	
		// Clamp the surface light, since it alone shouldn't blow out the surface (either it's fully lit, or not fully lit)
		surfaceLight = clamp(surfaceLight, vec3(0), vec3(1));
	
	
		//---------------------STEP 4: CALCULATE FINAL COLOR-----------------------
		// Calculate the final color, surface gets w = 1 since it is multiplied in, reflected gets w = 0 since it is added in
		vec4 finalColor = vec4(surfaceLight, 1) * surfaceColor * passColor + vec4(reflectedLight, 0);
	
		// Clamp the color
		finalColor = clamp(finalColor, vec4(0), vec4(1));
		
		outColor = finalColor; 				
	};