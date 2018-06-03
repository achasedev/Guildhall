#include "Game/Environment/Map.hpp"
#include "Game/Environment/MapChunk.hpp"
#include "Game/Framework/GameCommon.hpp"

#include "Engine/Core/Image.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Assets/AssetDB.hpp"
#include "Engine/Rendering/Core/Renderable.hpp"
#include "Engine/Rendering/Meshes/MeshBuilder.hpp"
#include "Engine/Rendering/Materials/Material.hpp"

#include "Engine/Rendering/DebugRendering/DebugRenderSystem.hpp"

Map::~Map()
{
	// Delete all chunks
	for (int index = 0; index < (int) m_mapChunks.size(); ++index)
	{
		delete m_mapChunks[index];
	}

	m_mapChunks.clear();
}

void Map::Intialize(const AABB2& worldBounds, float minHeight, float maxHeight, const IntVector2& chunkLayout, const std::string& filepath)
{
	m_worldBounds = worldBounds;
	m_chunkLayout = chunkLayout;
	m_heightRange = FloatRange(minHeight, maxHeight);

	Image* image = AssetDB::CreateOrGetImage(filepath);
	m_image = image;

	// Each texel of the image is a vertex, so ensure the image is at least 2x2
	IntVector2 imageDimensions = image->GetDimensions();
	GUARANTEE_OR_DIE(imageDimensions.x >= 2 && imageDimensions.y >= 2, Stringf("Error: Map::Initialize received bad map image \"%s\"", filepath.c_str()));

	// Assert that the image dimensions can be evenly chunked
	GUARANTEE_OR_DIE(((imageDimensions.x - 1) % chunkLayout.x == 0) && ((imageDimensions.y - 1) % chunkLayout.y == 0), Stringf("Error: Map::Initialize couldn't match the chunk layout to the image \"%s\"", filepath.c_str()));

	m_dimensions = IntVector2(imageDimensions.x - 1, imageDimensions.y - 1);

	// Set up iteration values
	Vector2 worldDimensions = worldBounds.GetDimensions();

	// Construct all the positions and uvs
	std::vector<Vector3> positions;
	std::vector<Vector2> uvs;
	float xStride = worldDimensions.x / (float) (imageDimensions.x - 1);
	float zStride = worldDimensions.y / (float) (imageDimensions.y - 1);

	for (int texelYIndex = 0; texelYIndex < imageDimensions.y; ++texelYIndex)
	{
		for (int texelXIndex = 0; texelXIndex < imageDimensions.x; ++texelXIndex)
		{
			// Determine the position
			float x = worldBounds.mins.x + texelXIndex * xStride;
			float z = worldBounds.maxs.y - texelYIndex * zStride;
			float y = image->GetTexelGrayScale(texelXIndex, texelYIndex);
			y = RangeMapFloat(y, 0.f, 255.f, minHeight, maxHeight);

			Vector3 position = Vector3(x, y, z);
			positions.push_back(position);

			float u = ((float) texelXIndex / ((float) imageDimensions.x - 1));
			float v = 1.0f - ((float) texelYIndex / ((float) imageDimensions.y - 1));

			Vector2 uv = Vector2(u, v);
			uvs.push_back(uv);
		}
	}

	// Build chunks
	IntVector2 chunkDimensions = IntVector2((imageDimensions.x - 1) / chunkLayout.x, (imageDimensions.y - 1) / chunkLayout.y);
	Material* mapMaterial = AssetDB::CreateOrGetSharedMaterial("Map");
	mapMaterial->SetProperty("SPECULAR_AMOUNT", 0.f);
	mapMaterial->SetProperty("SPECULAR_POWER", 1.f);

	for (int chunkYIndex = 0; chunkYIndex < chunkLayout.y; ++chunkYIndex)
	{
		// Across chunks - x
		for (int chunkXIndex = 0; chunkXIndex < chunkLayout.x; ++chunkXIndex)
		{
			int texelXStart = chunkXIndex * chunkDimensions.x;
			int texelYStart = chunkYIndex * chunkDimensions.y;

			MeshBuilder mb;
			std::vector<Vector3> chunkPositions;
			mb.BeginBuilding(PRIMITIVE_TRIANGLES, false);
			for (int texelYIndex = texelYStart; texelYIndex <= texelYStart + chunkDimensions.y; ++texelYIndex)
			{
				for (int texelXIndex = texelXStart; texelXIndex <= (texelXStart + chunkDimensions.x); ++texelXIndex)
				{
					int vertexIndex = texelYIndex * imageDimensions.x + texelXIndex;
					chunkPositions.push_back(positions[vertexIndex]);	
				}
			}

			// Get the average location of all the positions
			Vector3 averagePosition = Vector3::ZERO;
			int positionCount = (int) chunkPositions.size();
			for (int index = 0; index < positionCount; ++index)
			{
				averagePosition += chunkPositions[index];
			}
			averagePosition /= (float) positionCount;

			DebugRenderSystem::DrawUVSphere(averagePosition, 1000.f);

			Matrix44 model = Matrix44::MakeModelMatrix(averagePosition, Vector3::ZERO, Vector3::ONES);
			Matrix44 toLocal = Matrix44::MakeModelMatrix(-1.f * averagePosition, Vector3::ZERO, Vector3::ONES);

			mb.BeginBuilding(PRIMITIVE_TRIANGLES, false);
			for (int texelYIndex = texelYStart; texelYIndex < texelYStart + chunkDimensions.y; ++texelYIndex)
			{
				for (int texelXIndex = texelXStart; texelXIndex < (texelXStart + chunkDimensions.x); ++texelXIndex)
				{
					int tl = texelYIndex * imageDimensions.x + texelXIndex;
					int tr = tl + 1;
					int bl = tl + imageDimensions.x;
					int br = bl + 1;
	
					// Transform them to local space
					Vector3 tlPosition = toLocal.TransformPoint(positions[tl]).xyz();
					Vector3 trPosition = toLocal.TransformPoint(positions[tr]).xyz();
					Vector3 blPosition = toLocal.TransformPoint(positions[bl]).xyz();
					Vector3 brPosition = toLocal.TransformPoint(positions[br]).xyz();

					if ((chunkXIndex + chunkYIndex) % 2 == 0)
					{
						mb.SetColor(Rgba::RED);
					}

					mb.SetUVs(uvs[tl]);
					mb.PushVertex(tlPosition);
					
					mb.SetUVs(uvs[bl]);
					mb.PushVertex(blPosition);
					
					mb.SetUVs(uvs[br]);
					mb.PushVertex(brPosition);
					
					mb.SetUVs(uvs[tl]);
					mb.PushVertex(tlPosition);
					
					mb.SetUVs(uvs[br]);
					mb.PushVertex(brPosition);
					
					mb.SetUVs(uvs[tr]);
					mb.PushVertex(trPosition);
				}
			}

			mb.FinishBuilding();
			mb.GenerateSmoothNormals();

			Mesh* chunkMesh = mb.CreateMesh();

			MapChunk* chunk = new MapChunk(model, chunkMesh, mapMaterial);
			m_mapChunks.push_back(chunk);
		}
	}
}


Image* Map::GetImage() const
{
	return m_image;
}

