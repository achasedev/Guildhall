/************************************************************************/
/* Project: MP1 C27 A1 - Vector2, Basics, MathUtils
/* File: MathUtils.cpp
/* Author: Andrew Chase
/* Date: August 30th, 2017
/* Bugs: None
/* Description: Implementation of the MathUtils class
/************************************************************************/
#include "Engine/Math/MathUtils.hpp"
#include <math.h>
#include <cstdlib>


//-----------------------------------------------------------------------------------------------
// Converts an angle in radians to its degree measure and returns it
//
float ConvertRadiansToDegrees(float radians) 
{
	float pi = 3.1415926535897932384626433832795f;
	return radians * (180.f / pi);
}


//-----------------------------------------------------------------------------------------------
// Converts an angle in degrees to its radian measure and returns it
//
float ConvertDegreesToRadians(float degrees)
{
	float pi = 3.1415926535897932384626433832795f;
	return degrees * (pi / 180.f);
}


//-----------------------------------------------------------------------------------------------
// Calculates the cosine of an angle in degrees and returns it
//
float CosDegrees(float degrees)
{
	float radians = ConvertDegreesToRadians(degrees);

	return cosf(radians);
}


//-----------------------------------------------------------------------------------------------
// Calculates the sine of an angle in degrees and returns it
//
float SinDegrees(float degrees) 
{
	float radians = ConvertDegreesToRadians(degrees);

	return sinf(radians);
}


//-----------------------------------------------------------------------------------------------
// Calculates the arctan(y,x), expressed in degrees
//
float Atan2Degrees(float y, float x) 
{
	float radians = atan2f(y, x);

	return ConvertRadiansToDegrees(radians);
}


void ConvertCartesianToPolar(float x, float y, float& out_orientation, float& out_magnitude)
{
	out_orientation = Atan2Degrees(y, x);

	out_magnitude = sqrtf((x * x) + (y * y));
}




//-----------------------------------------------------------------------------------------------
// Generates a random float between zero and one (inclusively) and returns it
//
float GetRandomFloatZeroToOne()
{
	return static_cast<float>(rand()) * (1.f / static_cast<float>(RAND_MAX));
}


//-----------------------------------------------------------------------------------------------
// Generates a random float between minInclusive and maxInclusive and returns it
//
float GetRandomFloatInRange(float minInclusive, float maxInclusive)
{
	float ratio = GetRandomFloatZeroToOne();

	return ((maxInclusive - minInclusive) * ratio) + minInclusive;
}


//-----------------------------------------------------------------------------------------------
// Generates a random integer between zero (inclusive) and maxNotInclusive
//
int GetRandomIntLessThan(int maxNotInclusive)
{
	return rand() % maxNotInclusive;
}


//-----------------------------------------------------------------------------------------------
// Generates a random integer between minInclusive and maxInclusive and returns it
//
int GetRandomIntInRange(int minInclusive, int maxInclusive)
{
	return (rand() % (maxInclusive - minInclusive + 1)) + minInclusive; 
}


//-----------------------------------------------------------------------------------------------
// Randomly returns true or false (essentially a coin flip)
//
bool GetRandomTrueOrFalse()
{
	return ((rand() % 2) == 0);
}

float Clamp01(float inValue)
{
	if (inValue > 1.0f)
	{
		return 1.0f;
	}
	else if (inValue < 0.f)
	{
		return 0.f;
	}
	else
	{
		return inValue;
	}
}

float ClampFloat(float inValue, float minInclusive, float maxInclusive)
{
	if (inValue > maxInclusive)
	{
		return maxInclusive;
	}
	else if (inValue < minInclusive)
	{
		return minInclusive;
	}
	else
	{
		return inValue;
	}
}


float RangeMapFloat(float inValue, float inStart, float inEnd, float outStart, float outEnd)
{
	// If in range has size 0, just return the average of the out range
	if (inStart == inEnd)
	{
		return (outStart + outEnd) * 0.5f;
	}

	float inRange = inEnd - inStart;
	float outRange = outEnd - outStart;

	float inRelativeToStart = inValue - inStart;

	float fractionIntoRange = inRelativeToStart / inRange;
	float outRelativeToStart = fractionIntoRange * outRange;

	return outRelativeToStart + outStart;
}


//-----------------------------------------------------------------------------------------------
// Returns a scalar corresponding to the shorter degree turning direction in 2D
//
int GetShortestRotationDirection2D(float startAngle, float endAngle)
{
	if (startAngle == endAngle)
	{
		return 0;
	}

	while (startAngle < 0)
	{
		startAngle += 360.f;
	}

	while (endAngle < 0)
	{
		endAngle += 360.f;
	}

	float leftDegrees = -1;
	float rightDegrees = -1;

	if (startAngle < endAngle)
	{
		leftDegrees = endAngle - startAngle;
		rightDegrees = (360.f - leftDegrees);
	}
	else
	{
		rightDegrees = (startAngle - endAngle);
		leftDegrees = (360.f - rightDegrees);
	}

	// Return 1 if the positive direction is shorter, -1 if negative is shorter
	if (leftDegrees < rightDegrees)
	{
		return 1;
	}
	else 
	{
		return -1;
	}
}
