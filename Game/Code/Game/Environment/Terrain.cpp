#include "Game/Environment/Terrain.hpp"
#include "Engine/Core/Image.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/IntVector3.hpp"
#include "Engine/Rendering/Core/Renderable.hpp"

#include "Engine/Assets/AssetDB.hpp"
#include "Engine/Rendering/Materials/MaterialInstance.hpp"

Terrain::Terrain()
{

}

Terrain::~Terrain()
{
}

//-----------------------------------------------------------------------------------------------
// Loads the terrain in from a height map image file (will be updated later to load from XML)
//
void Terrain::LoadFromFile(const char* filename)
{
	// Load the image in from disk
	Image* heightMap = new Image();
	heightMap->LoadFromFile(filename);

	// Set the block dimensions
	int defaultHeight = 10;
	IntVector2 imageDimensions = heightMap->GetTexelDimensions();
	m_blockDimensions = IntVector3(imageDimensions.x, defaultHeight, imageDimensions.y);

	// Set the world extents
	// Blocks for the terrain are (1,1,1) in world dimensions, so maxs are just float version
	// of the block count
	m_worldBounds.mins = Vector3::ZERO;
	m_worldBounds.maxs = Vector3(m_blockDimensions);


	// Set the blocks
	m_renderable = new Renderable();
	m_renderable->AddInstanceMatrix(Matrix44::IDENTITY);
	m_blocks.resize(imageDimensions.x * imageDimensions.y * defaultHeight);
	int numBlocksPerLayer = imageDimensions.x * imageDimensions.y;

	for (int x = 0; x < imageDimensions.x; ++x)
	{
		for (int z = 0; z < imageDimensions.y; ++z)
		{
			float grayscale = heightMap->GetTexelGrayScale(imageDimensions.x * z + x);

			int currHeight = (int) RangeMapFloat(grayscale, 0.f, 1.0f, 0.f, 10.f);
			for (int y = 0; y < currHeight; ++y)
			{
				int index = numBlocksPerLayer * y + imageDimensions.x * z + x;

				// Set up the draw for this cube

				RenderableDraw_t draw;

				Vector3 currBlockBottomFrontLeft = Vector3((float)x, (float)y, (float)z);
				Vector3 drawPosition = currBlockBottomFrontLeft + Vector3(0.5f);

				draw.drawMatrix = Matrix44::MakeModelMatrix(drawPosition, Vector3::ZERO, Vector3::ONES);
				draw.sharedMaterial = AssetDB::GetSharedMaterial("Default_Opaque");
				draw.materialInstance = new MaterialInstance(draw.sharedMaterial);
				draw.materialInstance->SetDiffuse(AssetDB::CreateOrGetTexture("Data/Images/Debug/Debug.png"));
				draw.mesh = AssetDB::GetMesh("Cube");

				m_renderable->AddDraw(draw);

				m_blocks[index].SetWorldBounds(currBlockBottomFrontLeft, currBlockBottomFrontLeft + Vector3::ONES);
			}

			// Add in the blank draws, or update Renderable to allow insertion/deletion of draws at indices
		}
	}
}

Renderable* Terrain::GetRenderable() const
{
	return m_renderable;
}
