/************************************************************************/
/* Project: MP1 C27 A1 - Vector2, Basics, MathUtils
/* File: MathUtils.hpp
/* Author: Andrew Chase
/* Date: August 30th, 2017
/* Bugs: None
/* Description: Math Utility Class for my personal game engine
/************************************************************************/
#pragma once

float ConvertRadiansToDegrees(float radians);							// Converts an angle in radians to degrees
float ConvertDegreesToRadians(float degrees);							// converts an angle in degrees to radians
float CosDegrees(float degrees);										// Calculates the cosine of an angle in degrees
float SinDegrees(float degrees);										// Calculates the sine of an angle in degrees
float Atan2Degrees(float y, float x);									// Calculates arctan(y, x) expressed in degrees

void ConvertCartesianToPolar(float x, float y, float& out_orientation, float& out_magnitude);

float GetRandomFloatInRange(float minInclusive, float maxInclusive);	// Generates a random float between minInclusive and maxInclusive
int GetRandomIntInRange(int minInclusive, int maxInclusive);			// Generates a random int between minInclusive and maxInclusive
float GetRandomFloatZeroToOne();										// Generates a random float between zero and one
int GetRandomIntLessThan(int maxExclusive);								// Generates a random int between zero (inclusive) and maxExclusive
bool GetRandomTrueOrFalse();											// Randomly returns true or false


float ClampFloat(float inValue, float minInclusive, float maxInclusive);	// Clamps inValue to be between min and max, inclusive
float Clamp01(float inValue);												// Clamps inValue to be between 0 and 1, inclusive

float RangeMapFloat(float inValue, float inStart, float inEnd, float outStart, float outEnd);	// Maps invalue to a new scaled range

int GetShortestRotationDirection2D(float startAngle, float endAngle);		// Determines the shorter direction to turn from start to end angle
