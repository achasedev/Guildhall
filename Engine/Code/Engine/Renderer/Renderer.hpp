/************************************************************************/
/* Project: SD1 C27 A2 - Asteroids
/* File: Renderer.hpp
/* Author: Andrew Chase
/* Date: September 3rd, 2017
/* Bugs: None
/* Description: Class used to use OpenGL functions to draw to screen
/************************************************************************/
#pragma once
#include "Engine/Math/Vector2.hpp"
#include "Engine/Core/Rgba.hpp"
#include "Engine/Math/AABB2.hpp"


class Renderer
{
public:

	// Sets up the frame for rendering
	void BeginFrame();

	// Finishes render processes and swaps the buffers
	void EndFrame();

	// Draws a straight line between two points
	void DrawLine(const Vector2& startPos, const Vector2& endPos);

	// Draws a colored straight line between two points
	void DrawLineColor(const Vector2& startPos, const Rgba& startColor, const Vector2& endPos, const Rgba& endColor);

	// Draws a regular polygon
	void DrawRegularPolygon(const Vector2& centerPos, int numSides, float orientationOffset, float radius); 

	// Draws a dotted circle
	void DrawDottedCircle(const Vector2& centerPos, float radius);

	// Changes the draw color for future draws
	void SetDrawColor(const Rgba& newColor);

	// Clears the canvas to a single color
	void ClearBackBuffer(float red, float green, float blue, float alpha);

	// Sets the drawing coordinate system to an XY-plane defined by worldBounds
	void SetOrtho(const AABB2& worldBounds);

	// Pushes a new transformation matrix onto the OpenGL matrix stack
	void PushMatrix();

	// Move the coordinate system over by xPosition, yPosition, and zPosition
	void TranslateCoordinateSystem(float xPosition, float yPosition, float zPosition);

	// Rotate the coordinate system about the z-axis by orientationAngle
	void RotateCoordinateSystem2D(float orientationAngle);

	// Scales the coordinate system by scaleFactor
	void ScaleCoordinateSystem(float scaleFactor);

	// Removes and returns the matrix at the top of the OpenGL matrix stack
	void PopMatrix();

	// Enables the gl macro
	void EnableBlendMacro();

	// Enables the smooth line macro
	void EnableSmoothLine();

	// Sets the draw line width
	void SetLineWidth(float lineWidth);

	// Enables the alpha blend function (alpha, one_minus_alpha)
	void EnableAlphaBlendFunc();
};
