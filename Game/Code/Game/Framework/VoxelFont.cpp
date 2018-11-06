#include "Engine/Assets/AssetDB.hpp"
#include "Game/Framework/VoxelFont.hpp"
#include "Engine/Core/Utility/StringUtils.hpp"
#include "Engine/Core/Utility/ErrorWarningAssert.hpp"
#include "Engine/Core/Image.hpp"
#include "Engine/Math/MathUtils.hpp"


Rgba VoxelFont::GetColorForGlyphPixel(const char glyph, const IntVector2& offset) const
{
	IntVector2 totalDimensions = m_image->GetTexelDimensions();
	IntVector2 spriteLayout = IntVector2(16, 16);
	int glyphIndex = (int)glyph;

	// Get the glyph bottom left
	IntVector2 spriteCoord;
	spriteCoord.x = (glyphIndex % spriteLayout.x);
	spriteCoord.y = (glyphIndex / spriteLayout.x);
	spriteCoord.y = spriteLayout.y - spriteCoord.y - 1;

	int spriteStepX = (totalDimensions.x / spriteLayout.x);
	int spriteStepY = (totalDimensions.y / spriteLayout.y);

	IntVector2 spriteBottomLeft = IntVector2(spriteCoord.x * spriteStepX, spriteCoord.y * spriteStepY);
	IntVector2 finalCoord = spriteBottomLeft + offset;

	return m_image->GetTexelColor(finalCoord.x, finalCoord.y);
}

IntVector3 VoxelFont::GetGlyphDimensions() const
{
	IntVector2 spriteLayout = IntVector2(16, 16);
	IntVector2 imageDimensions = m_image->GetTexelDimensions();

	return IntVector3(imageDimensions.x / spriteLayout.x, imageDimensions.y / spriteLayout.y, 1);
}

IntVector3 VoxelFont::GetTextDimensions(const std::string& text) const
{
	IntVector2 glyphDimensions = m_image->GetTexelDimensions() / 16;

	int numChars = (int)text.size();
	
	return IntVector3(numChars * glyphDimensions.x, glyphDimensions.y, 1);
}

VoxelFont::VoxelFont(const std::string& name, const std::string& imageFile)
	: m_glyphLayout(IntVector2(16, 16))
{
	m_image = AssetDB::CreateOrGetImage(imageFile);
	//m_image->FlipVertical();
}
