/************************************************************************/
/* Project: SD1 C27 A2 - Asteroids
/* File: Renderer.cpp
/* Author: Andrew Chase
/* Date: September 3rd, 2017
/* Bugs: None
/* Description: Implementation of rendering functions used by Asteroids
/************************************************************************/
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Math/MathUtils.hpp"

#define WIN32_LEAN_AND_MEAN			// Always #define this before #including <windows.h>
#include <windows.h>
#include <gl/gl.h>					// Include basic OpenGL constants and function declarations
#pragma comment( lib, "opengl32" )	// Link in the OpenGL32.lib static library


//-----------------------------------------------------------------------------------------------
// Performs all Renderer procedures that occur at the beginning of the frame
//
void Renderer::BeginFrame()
{
}


//-----------------------------------------------------------------------------------------------
// Performs all Renderer procedures that occur at the end of the frame
//
void Renderer::EndFrame()
{
	// "Present" the backbuffer by swapping the front (visible) and back (working) screen buffers
	HWND activeWindow = GetActiveWindow();
	HDC displayContext = GetDC(activeWindow);
	SwapBuffers(displayContext); // Note: call this once at the end of each frame
}


//-----------------------------------------------------------------------------------------------
// Draws a line to the coordinate system give the positions and vertex colors
//
void Renderer::DrawLine(const Vector2& startPos, const Vector2& endPos)
{
	glBegin( GL_LINES );

	glVertex2f(startPos.x, startPos.y);
	glVertex2f(endPos.x, endPos.y);

	glEnd();
}


//-----------------------------------------------------------------------------------------------
// Draws a line to the coordinate system give the positions and vertex colors
//
void Renderer::DrawLineColor(const Vector2& startPos, const Rgba& startColor, const Vector2& endPos, const Rgba& endColor)
{
	glBegin( GL_LINES );

	SetDrawColor(startColor);
	glVertex2f(startPos.x, startPos.y);

	SetDrawColor(endColor);
	glVertex2f(endPos.x, endPos.y);

	// Set the color back to white (default)
	SetDrawColor(Rgba::WHITE);
	glEnd();
}


//-----------------------------------------------------------------------------------------------
// Draws a regular polygon to the coordinate system
//
void Renderer::DrawRegularPolygon(const Vector2& centerPos, int numSides, float orientationOffset, float radius) 
{
	// Draw numSides number of lines to draw the polygon
	for (int i = 0; i < numSides; i++) 
	{
		// Calculate start and end degree bounds for this side
		float degreesPerSide = (360.f / numSides);
		float startDegrees = (i * degreesPerSide);
		float endDegrees = (startDegrees + degreesPerSide);

		// calculate the start and end components for the two endpoints of this line
		float startX = centerPos.x + (radius * CosDegrees((startDegrees + orientationOffset)));
		float startY = centerPos.y + (radius * SinDegrees((startDegrees + orientationOffset)));
		float endX = centerPos.x + (radius * CosDegrees((endDegrees + orientationOffset)));
		float endY = centerPos.y + (radius * SinDegrees((endDegrees + orientationOffset)));

		// Draw a line between these two points
		DrawLineColor(Vector2(startX, startY), Rgba::WHITE, Vector2(endX, endY), Rgba::WHITE);
	}
}


//-----------------------------------------------------------------------------------------------
// Draws a dotted circle to the coordinate system
//
void Renderer::DrawDottedCircle(const Vector2& centerPos, float radius)
{
	// Draw numSides number of lines to draw the polygon
	for (int i = 0; i < 50; i++) 
	{
		if ((i % 2) != 0)
		{
			continue;
		}
		// Calculate start and end degree bounds for this side
		float degreesPerSide = (360.f / 50.f);
		float startDegrees = (i * degreesPerSide);
		float endDegrees = (startDegrees + degreesPerSide);

		// calculate the start and end components for the two endpoints of this line
		float startX = centerPos.x + (radius * CosDegrees(startDegrees));
		float startY = centerPos.y + (radius * SinDegrees(startDegrees));
		float endX = centerPos.x + (radius * CosDegrees(endDegrees));
		float endY = centerPos.y + (radius * SinDegrees(endDegrees));

		// Draw a line between these two points
		DrawLine(Vector2(startX, startY), Vector2(endX, endY));
	}
}


//-----------------------------------------------------------------------------------------------
// Changes the draw color for future draws
//
void Renderer::SetDrawColor(const Rgba& newColor)
{
	glColor4ub(newColor.r, newColor.g, newColor.b, newColor.a);
}


//-----------------------------------------------------------------------------------------------
// Clears the back buffer to a solid color
//
void Renderer::ClearBackBuffer(float red, float green, float blue, float alpha)
{
	glClearColor(red, green, blue, alpha);
	glClear(GL_COLOR_BUFFER_BIT);
}


//-----------------------------------------------------------------------------------------------
// Sets the coordinate system of the canvas to a simple XY-plane with x-values ranging from
// 0-maxXValue and y-values ranging from 0-MaxYValue
//
void Renderer::SetOrtho(const AABB2& worldBounds)
{
	glLoadIdentity();
	glOrtho(worldBounds.mins.x, worldBounds.maxs.x, worldBounds.mins.y, worldBounds.maxs.y, 0.f, 1.f );
}


//-----------------------------------------------------------------------------------------------
// Pushes a new transformation matrix on top of the matrix stack
//
void Renderer::PushMatrix()
{
	glPushMatrix();
}


//-----------------------------------------------------------------------------------------------
// Adds a translation to the top transformation matrix, so the coordinate system is now at
// (xPosition, yPosition, zPosition)
//
void Renderer::TranslateCoordinateSystem(float xPosition, float yPosition, float zPosition)
{
	glTranslatef(xPosition, yPosition, zPosition);
}


//-----------------------------------------------------------------------------------------------
// Rotates the coordinate system about the z-axis (for 2D rotations) by orientationAngle
//
void Renderer::RotateCoordinateSystem2D(float orientationAngle)
{
	glRotatef(orientationAngle, 0.f, 0.f, 1.0f);
}


//-----------------------------------------------------------------------------------------------
// Scales the coordinate system uniformly by scaleFactor
//
void Renderer::ScaleCoordinateSystem(float scaleFactor)
{
	glScalef(scaleFactor, scaleFactor, 1.0f);
}


//-----------------------------------------------------------------------------------------------
// Pops the top matrix of the transformation matrix stack
//
void Renderer::PopMatrix()
{
	glPopMatrix();
}


//-----------------------------------------------------------------------------------------------
// Sets the draw line width to the specified 'lineWidth'
//
void Renderer::SetLineWidth(float lineWidth)
{
	glLineWidth(lineWidth);
}


//-----------------------------------------------------------------------------------------------
// Enables the OpenGL macro GL_BLEND
//
void Renderer::EnableBlendMacro()
{
	glEnable(GL_BLEND);
}


//-----------------------------------------------------------------------------------------------
// Enables the OpenGL macro GL_LINE_SMOOTH
//
void Renderer::EnableSmoothLine()
{
	glEnable(GL_LINE_SMOOTH);
}


//-----------------------------------------------------------------------------------------------
// Calls the OpenGL function glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)
//
void Renderer::EnableAlphaBlendFunc()
{
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}
