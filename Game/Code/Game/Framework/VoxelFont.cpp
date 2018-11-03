#include "Game/Framework/VoxelFont.hpp"
#include "Engine/Core/Utility/StringUtils.hpp"
#include "Engine/Core/Utility/ErrorWarningAssert.hpp"
#include "Engine/Core/Image.hpp"

std::map<std::string, VoxelFont*> VoxelFont::s_fonts;

const VoxelTexture* VoxelFont::GetImageForGlyph(const char glyph) const
{
	return &m_textures[glyph];
}

IntVector3 VoxelFont::GetTextDimensions(const std::string& text) const
{
	int numChars = (int)text.size();

	IntVector3 total = IntVector3::ZERO;

	for (int charIndex = 0; charIndex < numChars; ++charIndex)
	{
		total += m_textures[text[charIndex]].GetDimensions();
	}

	return total;
}

void VoxelFont::LoadFont(const std::string& filename)
{
	XMLDocument document;
	XMLError error = document.LoadFile(filename.c_str());

	ASSERT_OR_DIE(error == tinyxml2::XML_SUCCESS, Stringf("Error: Couldn't open file %s", filename.c_str()));

	const XMLElement* root = document.RootElement();
	std::string fontName = ParseXmlAttribute(*root, "name", fontName);

	if (root != nullptr)
	{
		VoxelFont* font = new VoxelFont(fontName);

		const XMLElement* glyphElement = root->FirstChildElement();

		while (glyphElement != nullptr)
		{
			char glyph = ParseXmlAttribute(*glyphElement, "glyph", '\0');

			std::string imageFile = ParseXmlAttribute(*glyphElement, "file", imageFile);

			Image image;

			image.LoadFromFile(imageFile.c_str());
			image.FlipVertical();
			IntVector2 imageDimensions = image.GetTexelDimensions();
			IntVector3 glyphDimensions = IntVector3(imageDimensions.x, imageDimensions.y, 1);

			ASSERT_OR_DIE(image.GetNumComponentsPerTexel() == 4, "Error: VoxelFont Glyph doesn't have 4 color components per texel");

			font->m_textures[glyph].CreateFromColorStream((Rgba*)image.GetImageData(), glyphDimensions, false);

			glyphElement = glyphElement->NextSiblingElement();
		}

		s_fonts[fontName] = font;
	}
}

VoxelFont* VoxelFont::GetFont(const std::string& fontName)
{
	bool fontExists = (s_fonts.find(fontName) != s_fonts.end());

	if (fontExists)
	{
		return s_fonts.at(fontName);
	}

	return nullptr;
}

VoxelFont::VoxelFont(const std::string& name)
	: m_name(name)
{
}
