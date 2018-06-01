/************************************************************************/
/* File: FlyoutText.cpp
/* Author: Andrew Chase
/* Date: March 19th, 2018
/* Description: Implementation of the FlyoutText class
/************************************************************************/
#include "Game/Game.hpp"
#include "Game/FlyoutText.hpp"
#include "Engine/Math/Matrix44.hpp"
#include "Engine/Core/Stopwatch.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/OrbitCamera.hpp"


//-----------------------------------------------------------------------------------------------
// Constructor
//
FlyoutText::FlyoutText(const std::string& text, const Vector3& position, const Rgba& color, float durationSeconds /*=1.f*/)
	: m_text(text)
	, m_position(position)
	, m_startColor(color)
	, m_duration(durationSeconds)
{
	m_stopwatch = new Stopwatch(Game::GetGameClock());
	m_stopwatch->SetTimer(m_duration);
}


//-----------------------------------------------------------------------------------------------
// Renders - Above the actor's head, updating position based on the stopwatch
//
void FlyoutText::Render() const
{
	// For mesh building
	Vertex3D_PCU* vertices = (Vertex3D_PCU*) malloc(sizeof(Vertex3D_PCU) * m_text.size() * 4);
	unsigned int* indices = (unsigned int*) malloc(sizeof(Vertex3D_PCU) * m_text.size() * 6);
	int numVertices = 0;
	int numIndices = 0;

	// Get orientation vector for billboarding
	OrbitCamera* gameCamera = Game::GetGameCamera();
	Vector3 right = gameCamera->GetRightVector().GetNormalized();

	// Get resources
	Renderer* renderer = Renderer::GetInstance();
	BitmapFont* font = renderer->CreateOrGetBitmapFont("SquirrelFixedFont");

	// Set the draw position based on the stopwatch
	Vector3 bottomLeftOffset = (-0.5f * font->GetStringWidth(m_text, 1.0f, 1.0f) + font->GetGlyphAspect() * 0.5f) * right;
	Vector3 stopwatchOffset = Vector3(0.f, 2.f, 0.f) * m_stopwatch->GetElapsedTimeNormalized();

	Vector3 drawPosition = m_position + bottomLeftOffset + stopwatchOffset;

	// Scale the font color with the grayscale lerp value
	Rgba color = m_startColor;
	color.ScaleAlpha(1.0f - m_stopwatch->GetElapsedTimeNormalized());

	// Mesh build the characters
	for (int charIndex = 0; charIndex < (int) m_text.size(); ++charIndex)
	{
		unsigned char currChar = m_text[charIndex];
		
		// Skip spaces
		if (currChar != ' ')
		{
			AABB2 charUVs = font->GetGlyphUVs(currChar);
			renderer->AppendAABB2Vertices3D(vertices, numVertices, indices, numIndices, drawPosition, Vector2::ONES, charUVs, right, Vector3::DIRECTION_UP, color);
		}

		// Update the next letter's draw position
		drawPosition += (font->GetGlyphAspect() * right);
	}

	// Draw the mesh
	renderer->BindTexture(0, font->GetSpriteSheet().GetTexture().GetHandle());
	renderer->SetCurrentShaderProgram("Cutout");
	renderer->DrawMeshImmediate(vertices, numVertices, PRIMITIVE_TRIANGLES, indices, numIndices);
}


//-----------------------------------------------------------------------------------------------
// Returns whether the stopwatch timer elapsed
//
bool FlyoutText::IsFinished() const
{
	return m_stopwatch->HasIntervalElapsed();
}
