#version 430
#extension GL_ARB_compute_shader : enable
#extension GL_ARB_shader_storage_buffer_object : enable

// Set the work group dimensions
layout(local_size_x = 16, local_size_y = 16) in; // Make each work group 1x1 items - a single pixel
layout(rgba8, binding = 0) uniform image2D image_output; // The image output - ensure the format matches the bound image!

//-------------------------------------BUFFER DATA--------------------------------------------

struct OctreeNode
{
	vec3 color;		// The color of the voxel
	int flags;
};

layout(binding=0, std140) uniform timeUBO
{
	float GAME_DELTA_TIME;
	float GAME_TOTAL_TIME;
	float SYSTEM_DELTA_TIME;
	float SYSTEM_TOTAL_TIME;
};

layout(binding=10, std140) buffer color_data
{
	ivec3 GRID_DIMENSIONS;
	int padding1;
	OctreeNode NODES[ ];
};	

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

	mat4 INVERSE_VIEW_PROJECTION;
};

/*layout(binding=11, std140) uniform cameraUBO
{
	vec3 CAMERA_ORIGIN;					// Where the camera is positioned
	float padding2;
	vec3 CAMERA_LOWER_LEFT_CORNER;			// The lower left corner in camera space of the view plane 383479236 306783392
	float padding3;
	vec3 CAMERA_HORIZONTAL;		// The "right" direction in screen space, NOT normalized
	float padding4;
	vec3 CAMERA_VERTICAL;		// The "up" direction in screen space, NOT normalized
	float padding5;
	vec3 CAMERA_U;					// Camera basis vectors
	float padding6;
	vec3 CAMERA_V;
	float padding7;
	vec3 CAMERA_W;
	float CAMERA_LENS_RADIUS;				// For depth of field effects
};*/


//-------------------------------------STRUCTS DATA--------------------------------------------

struct Ray
{
	vec3 m_position;
	vec3 m_direction;
};

struct RayHit
{
	vec3 position;
	vec3 normal;
	float t;
	vec3 color;
	bool hit;
	bool isFinal;
	int gridID;
	int level;
};

struct aabb3
{
	vec3 mins;
	vec3 maxs;
};

struct PendingHit
{
	int gridID;
	int level;
};

vec3 dimensionKeys[8] = vec3[]
(
	vec3(0.f, 0.f, 0.f),
	vec3(0.f, 0.f, 1.f),
	vec3(0.f, 1.f, 0.f),
	vec3(0.f, 1.f, 1.f),
	vec3(1.f, 0.f, 0.f),
	vec3(1.f, 0.f, 1.f),
	vec3(1.f, 1.f, 0.f),
	vec3(1.f, 1.f, 1.f)
);

//-------------------------------------FUNCTIONS--------------------------------------------

/*void AddHitsToStack(RayHit hits[8], int hitCount)
{
	for (int i = hitCount - 1; i >= 0; --i)
	{
		PendingHit pHit;
		pHit.gridID = hits[i].gridID;
		pHit.level = hits[i].level;

		g_hitStack[g_pendingHitCount] = pHit;
		g_pendingHitCount++;
	}
}

PendingHit GetNextPendingHit()
{
	g_pendingHitCount--;
	return g_hitStack[g_pendingHitCount]; 
}*/

// Utility Functions
float GetRandomFloatZeroToOne()
{
	vec2 seed = vec2(SYSTEM_TOTAL_TIME);
	return fract(sin(dot(seed.xy , vec2(12.9898,78.233))) * 43758.5453);
}

vec2 GetRandomPointWithinCircle()
{
	return vec2(GetRandomFloatZeroToOne());
}

Ray GetRay(float s, float t)
{
	vec3 origin = CAMERA_POSITION;

	float u = s * 2.0 - 1.0;
	float v = t * 2.0 - 1.0;

	vec4 endPointPreW = vec4(u, v, -1.0, 1.0);

	endPointPreW = INVERSE_VIEW_PROJECTION * endPointPreW;

	vec3 rayEnd = endPointPreW.xyz / endPointPreW.w;

	vec3 direction = rayEnd - origin;
	return Ray(origin, direction);
}

vec3 GetPointAtPosition(Ray ray, float t)
{
	return ray.m_position + t * ray.m_direction;
}


aabb3 GetBounds(int level, int gridID)
{
	if (level == 0)
	{
		aabb3 result;
		result.mins = vec3(0.0);
		result.maxs = vec3(GRID_DIMENSIONS.x);
		return result;
	}

	int ancestry[9];
	ancestry[level] = gridID;

	for (int i = level - 1; i >= 0; i--)
	{
		int childIndex = ancestry[i + 1];
		int parentIndex = (childIndex - 1) / 8;
		ancestry[i] = parentIndex;
	}

	aabb3 workingBounds;
	workingBounds.mins = vec3(0.0);
	workingBounds.maxs = vec3(GRID_DIMENSIONS.x);

	for (int i = 1; i <= level; i++)
	{
		int localOffset = ancestry[i] - (8 * ancestry[i - 1] + 1);
		vec3 dimensionKey = dimensionKeys[localOffset];

		float divisor = pow(2.0f, i);
		vec3 dimensions = vec3(GRID_DIMENSIONS.x / divisor, GRID_DIMENSIONS.y / divisor, GRID_DIMENSIONS.z / divisor);

		vec3 bottomLeft = workingBounds.mins;
		bottomLeft.x += dimensionKey.x * dimensions.x;
		bottomLeft.y += dimensionKey.y * dimensions.y;
		bottomLeft.z += dimensionKey.z * dimensions.z;

		workingBounds.mins = bottomLeft;
		workingBounds.maxs = bottomLeft + dimensions;
	}

	return workingBounds;



	/*int parentIndex = (gridID - 1) / 8;
	float divisor = pow(2.0f, level);

	vec3 dimensions = vec3(GRID_DIMENSIONS.x / divisor, GRID_DIMENSIONS.y / divisor, GRID_DIMENSIONS.z / divisor);

	aabb3 parentBounds = GetBounds(level - 1, parentIndex);

	int childIndex = gridID - (8 * parentIndex + 1);
	vec3 dimensionKey = dimensionKeys[childIndex];


	vec3 bottomLeft = parentBounds.mins;
	bottomLeft.x += dimensionKey.x * dimensions.x;
	bottomLeft.y += dimensionKey.y * dimensions.y;
	bottomLeft.z += dimensionKey.z * dimensions.z;

	aabb3 result;
	result.mins = bottomLeft;
	result.maxs = bottomLeft + dimensions;

	return result;*/
}

RayHit DoesRayIntersectBox(Ray ray, aabb3 box)
{
	float tmin = (box.mins.x - ray.m_position.x) / ray.m_direction.x;
	float tmax = (box.maxs.x - ray.m_position.x) / ray.m_direction.x;

	float oldMin = tmin;
	tmin = min(tmin, tmax);
	tmax = max(oldMin, tmax);

	/*if (tmin > tmax)
	{
		float temp = tmin;
		tmin = tmax;
		tmax = temp;
	}*/

	float tymin = (box.mins.y - ray.m_position.y) / ray.m_direction.y;
	float tymax = (box.maxs.y - ray.m_position.y) / ray.m_direction.y;

	float oldYMin = tymin;
	tymin = min(tymin, tymax);
	tymax = max(oldYMin, tymax);

	/*if (tymin > tymax)
	{
		float temp = tymin;
		tymin = tymax;
		tymax = temp;
	}*/

	if ((tmin > tymax) || (tymin > tmax))
	{
		RayHit hit;
		hit.hit = false;
		return hit;
	}

	tmin = max(tymin, tmin);

	/*if (tymin > tmin)
		tmin = tymin;*/

	tmax = min(tymax, tmax);

	/*if (tymax < tmax)
		tmax = tymax;*/

	float tzmin = (box.mins.z - ray.m_position.z) / ray.m_direction.z;
	float tzmax = (box.maxs.z - ray.m_position.z) / ray.m_direction.z;

	float oldZMin = tzmin;
	tzmin = min(tzmin, tzmax);
	tzmax = max(oldZMin, tzmax);

	/*if (tzmin > tzmax)
	{
		float temp = tzmin;
		tzmin = tzmax;
		tzmax = temp;
	}*/

	if ((tmin > tzmax) || (tzmin > tmax))
	{
		RayHit hit;
		hit.hit = false;
		return hit;
	}

	tmin = max(tzmin, tmin);

	/*if (tzmin > tmin)
		tmin = tzmin;*/

	tmax = min(tzmax, tmax);

	/*if (tzmax < tmax)
		tmax = tzmax;*/

	RayHit hit;    
	hit.t = tmin;
	hit.hit = true;
	
	// Find the normal and the position

	return hit;
}

RayHit GetRayHitInfo(Ray r, int level, int gridID)
{
	aabb3 bounds = GetBounds(level, gridID);

	RayHit hit = DoesRayIntersectBox(r, bounds);

	hit.color = NODES[gridID].color;
	hit.gridID = gridID;
	hit.level = level;

	hit.isFinal = (1 << level == GRID_DIMENSIONS.x);
	return hit;
}

void GetColorForRay(Ray r, int level, int voxelIndex, inout RayHit hits[8], inout int totalHits)
{
	totalHits = 0;

	if (1 << level == GRID_DIMENSIONS.x)
	{
		RayHit hit = GetRayHitInfo(r, level, voxelIndex);

		if (hit.hit)
		{
			hits[0] = hit;
			totalHits = 1;
			return;
		}
	}
	else
	{
		for (int i = 0; i < 8; ++i)
		{
			int childVoxelIndex = 8 * voxelIndex + 1 + i;
			if ((NODES[voxelIndex].flags & (1 << i)) != 0)
			{
				RayHit hit = GetRayHitInfo(r, level + 1, childVoxelIndex);

				if (hit.hit)
				{
					hits[totalHits] = hit;
					totalHits++;
				}
			}
		}

		// Sort
		for (int i = 0; i < totalHits - 1; ++i)
		{
			for (int j = i + 1; j < totalHits; ++j)
			{
				if (hits[j].t < hits[i].t)
				{
					RayHit temp = hits[j];
					hits[j] = hits[i];
					hits[i] = temp;
				}
			}
		}
	}
}



void main()
{
	// Get the index in the global work group
	ivec2 pixel_coords = ivec2(gl_GlobalInvocationID.xy);
	ivec2 imageDimensions = imageSize(image_output);

	// Do the pixel work here
	vec3 finalColor = vec3(0.f);

	float u = (pixel_coords.x + 0.5) / imageDimensions.x;
	float v = (pixel_coords.y + 0.5) / imageDimensions.y;

	Ray ray = GetRay(u, v);

	RayHit hits[8];
	int totalHits = 0;

	bool hitFinal = false;

	int pendingHitCount = 1;
	PendingHit hitStack[64];

	hitStack[0].gridID = 0;
	hitStack[0].level = 0;

	//int count = 0;
	while (pendingHitCount > 0)
	{
		pendingHitCount--;
		PendingHit pHit = hitStack[pendingHitCount]; 
		GetColorForRay(ray, pHit.level, pHit.gridID, hits, totalHits);
		// Check for early termination
		if (totalHits > 0 && hits[0].isFinal)
		{
			hitFinal = true;
			break;
		}

		for (int i = totalHits - 1; i >= 0; --i)
		{
			PendingHit NewPHit;
			NewPHit.gridID = hits[i].gridID;
			NewPHit.level = hits[i].level;
			hitStack[pendingHitCount] = NewPHit;
			pendingHitCount++;
		}


		//count++;
	}

	if (hitFinal)
	{
		finalColor += hits[0].color;
		//finalColor = vec3(1.0, 1.0, 0.0);
	}

	//finalColor = vec3(sqrt(finalColor.x), sqrt(finalColor.y), sqrt(finalColor.z));
	// Output the color to the image
	imageStore(image_output, pixel_coords, vec4(finalColor, 1.0));
}

