#include "Game/Dialogue.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Math/MathUtils.hpp"

// Default Dialogue box to use when one isn't specified
const AABB2 Dialogue::DEFAULT_TEXT_BOX = AABB2(Vector2(1.f, 1.0f), Vector2(15.f, 4.f));

Dialogue::Dialogue(const std::string& text, const AABB2& textBox/*=DEFAULT_TEXT_BOX*/)
	: m_text(text)
	, m_textBox(textBox)
{
}

void Dialogue::Update(float deltaTime)
{
	m_ageInSeconds += deltaTime;
}

void Dialogue::Render() const
{
	g_theRenderer->SetOrtho(*g_screenBounds);

	// Draw the background
	AABB2 exterior = m_textBox;
	exterior.AddPaddingToSides(0.25f, 0.25f);

	g_theRenderer->DrawAABB2(exterior, Rgba::WHITE);
	g_theRenderer->DrawAABB2(m_textBox, Rgba::BLACK);
	

	g_theRenderer->DrawTextInBox2D(m_text, m_textBox, Vector2(0.5f, 0.5f), 0.5f, TEXT_DRAW_WORD_WRAP);

	// Put a prompt on the bottom right to continue
	static float flashAlphaScalar = 0.f;
	static float fadeMagnitude = 0.05f;

	// Create the alpha color
	Rgba textColor = Rgba::WHITE;
	textColor.ScaleAlpha(flashAlphaScalar);
	g_theRenderer->DrawTextInBox2D("Press SPACE or A-Button to continue", m_textBox, Vector2::ONES, 0.2f, TEXT_DRAW_WORD_WRAP, nullptr, textColor);

	// Update the fade alpha
	flashAlphaScalar += fadeMagnitude;
	if (flashAlphaScalar >= 1.f || flashAlphaScalar <= 0.f)
	{
		flashAlphaScalar = ClampFloatZeroToOne(flashAlphaScalar);
		fadeMagnitude *= -1.f;
	} 
}

