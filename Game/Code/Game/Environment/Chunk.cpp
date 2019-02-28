/************************************************************************/
/* File: Chunk.cpp
/* Author: Andrew Chase
/* Date: February 9th 2019
/* Description: Implementation of the Chunk class
/************************************************************************/
#include "Game/Environment/Chunk.hpp"
#include "Game/Framework/GameCommon.hpp"
#include "Game/Environment/BlockType.hpp"
#include "Game/Environment/BlockLocator.hpp"
#include "Engine/Core/File.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Assets/AssetDB.hpp"
#include "Engine/Math/IntVector3.hpp"
#include "Engine/Rendering/Core/Renderer.hpp"
#include "Engine/Core/Utility/SmoothNoise.hpp"
#include "Engine/Core/Utility/StringUtils.hpp"
#include "Engine/Rendering/Meshes/MeshBuilder.hpp"
#include "Engine/Core/DeveloperConsole/DevConsole.hpp"


struct ChunkFileHeader_t
{
	uint8_t fourDigitCharacterCode[4] = { 'S', 'M', 'C', 'D' };

	uint8_t version = Chunk::CHUNK_VERSION;
	uint8_t chunkBitX = Chunk::CHUNK_BITS_X;
	uint8_t chunkBitY = Chunk::CHUNK_BITS_Y;
	uint8_t chunkBitZ = Chunk::CHUNK_BITS_Z;

	uint8_t unusedByteOne = 0;
	uint8_t unusedByteTwo = 0;
	uint8_t unusedByteThree = 0;
	uint8_t format = 'R'; // R stands for RLE-Compression
};


//-----------------------------------------------------------------------------------------------
// Constructor
//
Chunk::Chunk(const IntVector2& chunkCoords)
	: m_chunkCoords(chunkCoords)
{
	// Set the world bounds
	m_worldBounds.mins = Vector3(chunkCoords.x * CHUNK_DIMENSIONS_X, chunkCoords.y * CHUNK_DIMENSIONS_Y, 0);
	m_worldBounds.maxs = m_worldBounds.mins + Vector3(CHUNK_DIMENSIONS_X, CHUNK_DIMENSIONS_Y, CHUNK_DIMENSIONS_Z);
}


//-----------------------------------------------------------------------------------------------
// Destructor
//
Chunk::~Chunk()
{
	if (m_mesh != nullptr)
	{
		delete m_mesh;
		m_mesh = nullptr;
	}
}


//---C FUNCTION----------------------------------------------------------------------------------
// Checks the contents of the file to see if it is a valid chunk file, and throws a recoverable
// error if any issues are found
//
bool VerifyChunkDataFromFile(File* file)
{
	file->LoadFileToMemory();
	const char* data = file->GetData();

	ChunkFileHeader_t header;
	memcpy(&header, data, sizeof(header));

	// Verify the header
	std::string fourCharacterCode = std::string(data, 4);

	// 4cc
	if (fourCharacterCode != "SMCD")
	{
		ERROR_RECOVERABLE(Stringf("Error: File %s doesn't have four character code SMCD", file->GetFilePathOpened().c_str()).c_str());
		return false;
	}

	// Version
	if (header.version != Chunk::CHUNK_VERSION)
	{
		ERROR_RECOVERABLE(Stringf("Error: Chunk File %s is version %i, game is version %i", file->GetFilePathOpened().c_str(), header.version, Chunk::CHUNK_VERSION).c_str());
		return false;
	}
	
	// Chunk dimensions
	if (header.chunkBitX != Chunk::CHUNK_BITS_X)
	{
		ERROR_RECOVERABLE(Stringf("Error: Chunk File %s has %i bits for X, game has %i bits for X", file->GetFilePathOpened().c_str(), header.chunkBitX, Chunk::CHUNK_BITS_X).c_str());
		return false;
	}

	if (header.chunkBitY != Chunk::CHUNK_BITS_Y)
	{
		ERROR_RECOVERABLE(Stringf("Error: Chunk File %s has %i bits for Y, game has %i bits for Y", file->GetFilePathOpened().c_str(), header.chunkBitY, Chunk::CHUNK_BITS_Y).c_str());
		return false;
	}

	if (header.chunkBitZ != Chunk::CHUNK_BITS_Z)
	{
		ERROR_RECOVERABLE(Stringf("Error: Chunk File %s has %i bits for Z, game has %i bits for Z", file->GetFilePathOpened().c_str(), header.chunkBitZ, Chunk::CHUNK_BITS_Z).c_str());
		return false;
	}

	// Compression Format
	// #TODO: Update to support other formats
	if (header.format != 'R')
	{
		ERROR_RECOVERABLE(Stringf("Error: Chunk File %s has format %c specified, only format 'R' is supported", file->GetFilePathOpened().c_str(), header.format));
		return false;
	}

	// Check run count
	size_t fileSize = file->GetSize();
	int totalBlocks = 0;
	for (size_t byteIndex = sizeof(ChunkFileHeader_t); byteIndex < fileSize; byteIndex += 2)
	{
		uint8_t runCount = (uint8_t)data[byteIndex + 1];
		totalBlocks += runCount;
	}

	int amountShouldTotal = Chunk::CHUNK_DIMENSIONS_X * Chunk::CHUNK_DIMENSIONS_Y * Chunk::CHUNK_DIMENSIONS_Z;
	if (totalBlocks != amountShouldTotal)
	{
		ERROR_RECOVERABLE(Stringf("Error: Chunk File %s should specify %i blocks but only specifies %i", file->GetFilePathOpened().c_str(), amountShouldTotal, totalBlocks));
		return false;
	}

	return true;
}


//-----------------------------------------------------------------------------------------------
// Loads the file into memory and parses the data to initialize this chunk
//
bool Chunk::InitializeFromFile(const std::string& filepath)
{
	File file;
	bool fileFound = file.Open(filepath.c_str(), "rb");

	if (!fileFound)
	{
		return false;
	}

	bool dataGood = VerifyChunkDataFromFile(&file);

	if (!dataGood)
	{
		return false;
	}

	// Load in the blocks
	uint8_t* data = (uint8_t*)file.GetData();
	size_t fileSize = file.GetSize();

	// Start after the header
	int blocksLoadedSoFar = 0;
	for (size_t byteIndex = sizeof(ChunkFileHeader_t); byteIndex < fileSize; byteIndex += 2)
	{
		uint8_t runType = data[byteIndex];
		uint8_t runCount = data[byteIndex + 1];

		for (int blockIndex = blocksLoadedSoFar; blockIndex < blocksLoadedSoFar + (int) runCount; ++blockIndex)
		{
			m_blocks[blockIndex].SetType(runType);
		}

		blocksLoadedSoFar += runCount;
	}
	
	return true;
}


//-----------------------------------------------------------------------------------------------
// Populates the chunk with blocks using Perlin Noise
//
void Chunk::GenerateWithPerlinNoise(int baseElevation, int maxDeviationFromBaseElevation, int seaLevel)
{
	const BlockType* grassType = BlockType::GetTypeByName("Grass");
	const BlockType* dirtType = BlockType::GetTypeByName("Dirt");
	const BlockType* stoneType = BlockType::GetTypeByName("Stone");
	const BlockType* waterType = BlockType::GetTypeByName("Water");
	const BlockType* missingType = BlockType::GetTypeByIndex(BlockType::MISSING_TYPE_INDEX);

	for (int yIndex = 0; yIndex < CHUNK_DIMENSIONS_Y; ++yIndex)
	{
		for (int xIndex = 0; xIndex < CHUNK_DIMENSIONS_X; ++xIndex)
		{
			// Use XY center positions for the noise, in world coordinates
			Vector2 chunkOffsetFromWorldOrigin = Vector2(m_chunkCoords.x * CHUNK_DIMENSIONS_X, m_chunkCoords.y * CHUNK_DIMENSIONS_Y);
			Vector2 blockXYCenter = chunkOffsetFromWorldOrigin + Vector2(xIndex, yIndex) + Vector2(0.5f);

			// Get the height of the chunk at these coordinates
			float noise = Compute2dPerlinNoise(blockXYCenter.x, blockXYCenter.y, 50.f);
			int elevationFromNoise = RoundToNearestInt(noise * maxDeviationFromBaseElevation) + baseElevation;

			int maxHeightForThisColumn = MaxInt(elevationFromNoise, seaLevel);

			for (int zIndex = 0; zIndex < maxHeightForThisColumn; ++zIndex)
			{
				const BlockType* typeToUse = nullptr;
				if (elevationFromNoise >= seaLevel)
				{
					if (zIndex == elevationFromNoise - 1)
					{
						typeToUse = grassType;
					}
					else if (zIndex > elevationFromNoise - 4) // Dirt for 3 blocks below the grass height
					{
						typeToUse = dirtType;
					}
					else
					{
						typeToUse = stoneType;
					}
				}
				else
				{
					if (zIndex >= elevationFromNoise)
					{
						typeToUse = waterType;
					}
					else if (zIndex > seaLevel - 4)
					{
						typeToUse = dirtType;
					}
					else
					{
						typeToUse = stoneType;
					}
				}


				IntVector3 blockCoords = IntVector3(xIndex, yIndex, zIndex);
				int blockIndex = GetBlockIndexFromBlockCoords(blockCoords);

				m_blocks[blockIndex].SetType(typeToUse->m_typeIndex);
			}
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Builds the mesh for the chunk using the block data it has already set
//
void Chunk::BuildMesh()
{
	m_meshBuilder.BeginBuilding(PRIMITIVE_TRIANGLES, true);

	for (int blockIndex = 0; blockIndex < BLOCKS_PER_CHUNK; ++blockIndex)
	{
		Block& block = m_blocks[blockIndex];
		uint8_t blockTypeIndex = block.GetType();

		if (blockTypeIndex == 0)
		{
			continue;
		}

		const BlockType* typeToUse = BlockType::GetTypeByIndex(blockTypeIndex);
		PushVerticesForBlock(blockIndex, typeToUse);
	}

	m_meshBuilder.FinishBuilding();

	if (m_mesh == nullptr)
	{
		m_mesh = m_meshBuilder.CreateMesh();
	}
	else
	{
		m_meshBuilder.UpdateMesh(*m_mesh);
	}

	m_isMeshDirty = false;
	m_meshBuilder.Clear();
}


//-----------------------------------------------------------------------------------------------
// Update
//
void Chunk::Update()
{
}


//-----------------------------------------------------------------------------------------------
// Render
//
void Chunk::Render() const
{
	if (m_mesh != nullptr)
	{
		Renderer* renderer = Renderer::GetInstance();
		Material* material = AssetDB::CreateOrGetSharedMaterial("Data/Materials/Overworld_Opaque.material");

		renderer->DrawMeshWithMaterial(m_mesh, material);
	}
}


//-----------------------------------------------------------------------------------------------
// Returns the XY center position of the chunk
//
Vector2 Chunk::GetWorldXYCenter() const
{
	float x = 0.5f * (m_worldBounds.mins.x + m_worldBounds.maxs.x);
	float y = 0.5f * (m_worldBounds.mins.y + m_worldBounds.maxs.y);

	return Vector2(x, y);
}


//-----------------------------------------------------------------------------------------------
// Returns the XY bounds of the chunk
//
AABB2 Chunk::GetWorldXYBounds() const
{
	AABB2 xyBounds;
	xyBounds.mins = m_worldBounds.mins.xy();
	xyBounds.maxs = m_worldBounds.maxs.xy();

	return xyBounds;
}


//-----------------------------------------------------------------------------------------------
// Returns the "0,0,0" position in this chunk's coords in world coordinates
//
Vector3 Chunk::GetOriginWorldPosition() const
{
	float x = CHUNK_DIMENSIONS_X * (float) m_chunkCoords.x;
	float y = CHUNK_DIMENSIONS_Y * (float) m_chunkCoords.y;
	float z = 0.f;

	return Vector3(x, y, z);
}


//-----------------------------------------------------------------------------------------------
// Returns a block locator that points to the block contains the given world position
// Returns an empty locator if this chunk doesn't contain it
//
BlockLocator Chunk::GetBlockLocatorThatContainsPosition(const Vector3& worldPosition)
{
	// Ensure it's within this chunk
	if (!m_worldBounds.ContainsPoint(worldPosition))
	{
		return BlockLocator(nullptr, 0);
	}
	else
	{
		Vector3 localOffsetFromChunkOrigin = worldPosition - m_worldBounds.mins;

		int blockXCoord = Floor(localOffsetFromChunkOrigin.x);
		int blockYCoord = Floor(localOffsetFromChunkOrigin.y);
		int blockZCoord = Floor(localOffsetFromChunkOrigin.z);
		IntVector3 blockCoord = IntVector3(blockXCoord, blockYCoord, blockZCoord);
		int blockIndex = GetBlockIndexFromBlockCoords(blockCoord);

		return BlockLocator(this, blockIndex);
	}
}


//-----------------------------------------------------------------------------------------------
// Writes the chunk to file
//
void Chunk::WriteToFile() const
{
	// Create the directory if it doesn't exist
	CreateDirectoryA("Saves", NULL);

	std::string fileName = Stringf("Saves/Chunk_%i,%i.chunk", m_chunkCoords.x, m_chunkCoords.y);
	File file;
	bool opened = file.Open(fileName.c_str(), "wb+");

	if (!opened)
	{
		ConsoleErrorf("Couldn't open chunk file %s for write", fileName.c_str());
		return;
	}

	
	// Write the header
	ChunkFileHeader_t header;

	std::vector<uint8_t> buffer;
	buffer.reserve(65536);
	buffer.resize(sizeof(header));

	memcpy(buffer.data(), &header, sizeof(header));

	// TODO: Update to check the format before writing

	// Iterate across the blocks
	uint8_t rollingCount = 0;
	uint8_t rollingType = m_blocks[0].GetType();

	for (int blockIndex = 0; blockIndex < BLOCKS_PER_CHUNK; ++blockIndex)
	{
		uint8_t currType = m_blocks[blockIndex].GetType();

		// If it doesn't match
		if (currType != rollingType)
		{
			buffer.push_back(rollingType);
			buffer.push_back(rollingCount);

			rollingType = currType;
			rollingCount = 1;
		}
		else
		{
			if (rollingCount == 0xFF) // If we hit RLE limit
			{
				buffer.push_back(rollingType);
				buffer.push_back(rollingCount);

				rollingCount = 0;
			}

			rollingCount++;
		}
	}

	// Push the last bit
	buffer.push_back(rollingType);
	buffer.push_back(rollingCount);

	// Write to file
	file.Write(buffer.data(), buffer.size());

	file.Close();

	ConsolePrintf(Rgba::GREEN, "Wrote chunk (%i, %i) to file", m_chunkCoords.x, m_chunkCoords.y);
}


//-----------------------------------------------------------------------------------------------
// Returns a reference to the block at the given index
//
Block& Chunk::GetBlock(int blockIndex)
{
	return m_blocks[blockIndex];
}


//-----------------------------------------------------------------------------------------------
// Returns a reference to the block at the given coordinates
//
Block& Chunk::GetBlock(const IntVector3& blockCoords)
{
	int blockIndex = GetBlockIndexFromBlockCoords(blockCoords);
	return GetBlock(blockIndex);
}


//-----------------------------------------------------------------------------------------------
// Returns the chunk coordinates of this chunk
//
IntVector2 Chunk::GetChunkCoords() const
{
	return m_chunkCoords;
}


//-----------------------------------------------------------------------------------------------
// Returns whether this chunk needs to be written to file when deactivated
//
bool Chunk::ShouldWriteToFile() const
{
	return m_needsToBeSaved;
}

//-----------------------------------------------------------------------------------------------
// Returns the chunk directly to the east of this chunk
//
Chunk* Chunk::GetEastNeighbor() const
{
	return m_eastNeighborChunk;
}


//-----------------------------------------------------------------------------------------------
// Returns the chunk directly to the west of this chunk
//
Chunk* Chunk::GetWestNeighbor() const
{
	return m_westNeighborChunk;
}


//-----------------------------------------------------------------------------------------------
// Returns the chunk directly to the north of this chunk
//
Chunk* Chunk::GetNorthNeighbor() const
{
	return m_northNeighborChunk;
}


//-----------------------------------------------------------------------------------------------
// Returns the chunk directly to the south of this chunk
//
Chunk* Chunk::GetSouthNeighbor() const
{
	return m_southNeighborChunk;
}


//-----------------------------------------------------------------------------------------------
// Returns true if all neighbors to this chunk exist and are in the world (may not have meshes)
//
bool Chunk::HasAllFourNeighbors() const
{
	return (m_eastNeighborChunk != nullptr && m_westNeighborChunk != nullptr 
		&& m_northNeighborChunk != nullptr && m_southNeighborChunk != nullptr);
}


//-----------------------------------------------------------------------------------------------
// Returns whether this chunk's mesh needs to be (re)built
//
bool Chunk::IsMeshDirty() const
{
	return m_isMeshDirty;
}


//-----------------------------------------------------------------------------------------------
// Sets this chunk's east neighbor to the one provided
//
void Chunk::SetEastNeighbor(Chunk* chunkToEast)
{
	m_eastNeighborChunk = chunkToEast;
}


//-----------------------------------------------------------------------------------------------
// Sets this chunk's west neighbor to the one provided
//
void Chunk::SetWestNeighbor(Chunk* chunkToWest)
{
	m_westNeighborChunk = chunkToWest;
}


//-----------------------------------------------------------------------------------------------
// Sets this chunk's north neighbor to the one provided
//
void Chunk::SetNorthNeighbor(Chunk* chunkToNorth)
{
	m_northNeighborChunk = chunkToNorth;
}


//-----------------------------------------------------------------------------------------------
// Sets this chunk's south neighbor to the one provided
//
void Chunk::SetSouthNeighbor(Chunk* chunkToSouth)
{
	m_southNeighborChunk = chunkToSouth;
}


//-----------------------------------------------------------------------------------------------
// Sets whether the mesh of this chunk is dirty or not
//
void Chunk::SetIsMeshDirty(bool isMeshDirty)
{
	m_isMeshDirty = isMeshDirty;
}


//-----------------------------------------------------------------------------------------------
// Sets whether this chunk needs to be written to disk when deactivated
//
void Chunk::SetNeedsToBeSavedToDisk(bool needsToBeSaved)
{
	m_needsToBeSaved = needsToBeSaved;
}


//-----------------------------------------------------------------------------------------------
// Sets the block type of the block at the given index to the one provided
//
void Chunk::SetBlockTypeAtBlockIndex(int blockIndex, uint8_t blockType)
{
	Block& block = m_blocks[blockIndex];
	block.SetType(blockType);

	// Dirty the mesh and adjacent neighbors if the block was on the XY-border of the chunk
	m_isMeshDirty = true;
	m_needsToBeSaved = true;

	IntVector3 blockCoords = GetBlockCoordsFromBlockIndex(blockIndex);

	if (blockCoords.x == 0) // West Neighbor
	{
		m_westNeighborChunk->SetIsMeshDirty(true);
	}
	else if (blockCoords.x == CHUNK_DIMENSIONS_X - 1) // East Neighbor
	{
		m_eastNeighborChunk->SetIsMeshDirty(true);
	}

	if (blockCoords.y == 0) // South Neighbor
	{
		m_southNeighborChunk->SetIsMeshDirty(true);
	}
	else if (blockCoords.y == CHUNK_DIMENSIONS_Y - 1) // North Neighbor
	{
		m_northNeighborChunk->SetIsMeshDirty(true);
	}
}


//-----------------------------------------------------------------------------------------------
// Sets the type of the block at the given block coords to the given type
//
void Chunk::SetBlockTypeAtBlockCoords(const IntVector3& blockCoords, uint8_t blockType)
{
	int blockIndex = GetBlockIndexFromBlockCoords(blockCoords);
	SetBlockTypeAtBlockIndex(blockIndex, blockType);
}


//-----------------------------------------------------------------------------------------------
// Returns the block index of the block given by blockCoords
//
int Chunk::GetBlockIndexFromBlockCoords(const IntVector3& blockCoords)
{
	int index = BLOCKS_PER_Z_LAYER * blockCoords.z + CHUNK_DIMENSIONS_X * blockCoords.y + blockCoords.x;
	return index;
}


//-----------------------------------------------------------------------------------------------
// Returns the block coords for the block given by blockIndex
//
IntVector3 Chunk::GetBlockCoordsFromBlockIndex(int blockIndex)
{
	int xCoord = blockIndex & CHUNK_X_MASK;
	int yCoord = (blockIndex & CHUNK_Y_MASK) >> CHUNK_BITS_Y;
	int zCoord = (blockIndex & CHUNK_Z_MASK) >> CHUNK_BITS_XY;

	return IntVector3(xCoord, yCoord, zCoord);
}


//-----------------------------------------------------------------------------------------------
// Pushes the vertices (and indices) into the meshbuilder for the block at the given coords
// PUSHES THE BLOCK VERTICES IN WORLD SPACE
//
void Chunk::PushVerticesForBlock(const IntVector3& blockCoords, const BlockType* type)
{
	int blockIndex = GetBlockIndexFromBlockCoords(blockCoords);

	int worldXOffset = m_chunkCoords.x * CHUNK_DIMENSIONS_X;
	int worldYOffset = m_chunkCoords.y * CHUNK_DIMENSIONS_Y;

	Vector3 cubeBottomSouthWest = Vector3(worldXOffset + blockCoords.x, worldYOffset + blockCoords.y, blockCoords.z);
	Vector3 cubeTopNorthEast = cubeBottomSouthWest + Vector3::ONES;

	// For hidden surface removal
	BlockLocator currBlockLocator = BlockLocator(this, blockIndex);

	BlockLocator eastBlockLocator = currBlockLocator.ToEast();
	BlockLocator westBlockLocator = currBlockLocator.ToWest();
	BlockLocator northBlockLocator = currBlockLocator.ToNorth();
	BlockLocator southBlockLocator = currBlockLocator.ToSouth();
	BlockLocator aboveBlockLocator = currBlockLocator.ToAbove();
	BlockLocator belowBlockLocator = currBlockLocator.ToBelow();

	bool pushEastFace	= eastBlockLocator.GetBlock().GetType() == BlockType::MISSING_TYPE_INDEX || !eastBlockLocator.GetBlock().IsFullyOpaque();
	bool pushWestFace	= westBlockLocator.GetBlock().GetType() == BlockType::MISSING_TYPE_INDEX || !westBlockLocator.GetBlock().IsFullyOpaque();
	bool pushNorthFace	= northBlockLocator.GetBlock().GetType() == BlockType::MISSING_TYPE_INDEX || !northBlockLocator.GetBlock().IsFullyOpaque();
	bool pushSouthFace	= southBlockLocator.GetBlock().GetType() == BlockType::MISSING_TYPE_INDEX || !southBlockLocator.GetBlock().IsFullyOpaque();
	bool pushTopFace	= aboveBlockLocator.GetBlock().GetType() == BlockType::MISSING_TYPE_INDEX || !aboveBlockLocator.GetBlock().IsFullyOpaque();
	bool pushBottomFace = belowBlockLocator.GetBlock().GetType() == BlockType::MISSING_TYPE_INDEX || !belowBlockLocator.GetBlock().IsFullyOpaque();


	// East Face
	if (pushEastFace)
	{
		m_meshBuilder.Push3DQuad(cubeTopNorthEast, Vector2::ONES, type->m_sideUVs, Rgba::WHITE, Vector3::Y_AXIS, Vector3::Z_AXIS, Vector2(1.f, 1.0f));
	}

	// West Face
	if (pushWestFace)
	{
		m_meshBuilder.Push3DQuad(cubeBottomSouthWest, Vector2::ONES, type->m_sideUVs, Rgba::WHITE, Vector3::MINUS_Y_AXIS, Vector3::Z_AXIS, Vector2(1.0f, 0.f));
	}

	// North Face
	if (pushNorthFace)
	{
		m_meshBuilder.Push3DQuad(cubeTopNorthEast, Vector2::ONES, type->m_sideUVs, Rgba::WHITE, Vector3::MINUS_X_AXIS, Vector3::Z_AXIS, Vector2(0.f, 1.0f));
	}

	// South Face
	if (pushSouthFace)
	{
		m_meshBuilder.Push3DQuad(cubeBottomSouthWest, Vector2::ONES, type->m_sideUVs, Rgba::WHITE, Vector3::X_AXIS, Vector3::Z_AXIS, Vector2::ZERO);
	}

	// Top Face
	if (pushTopFace)
	{
		m_meshBuilder.Push3DQuad(cubeTopNorthEast, Vector2::ONES, type->m_topUVs, Rgba::WHITE, Vector3::MINUS_Y_AXIS, Vector3::X_AXIS, Vector2(0.f, 1.0f));
	}

	// Bottom Face
	if (pushBottomFace)
	{
		m_meshBuilder.Push3DQuad(cubeBottomSouthWest, Vector2::ONES, type->m_bottomUVs, Rgba::WHITE, Vector3::MINUS_Y_AXIS, Vector3::MINUS_X_AXIS, Vector2::ONES);
	}
}


//-----------------------------------------------------------------------------------------------
// Pushes the vertices for the given block into the chunk's mesh
//
void Chunk::PushVerticesForBlock(int blockIndex, const BlockType* type)
{
	IntVector3 blockCoords = GetBlockCoordsFromBlockIndex(blockIndex);
	PushVerticesForBlock(blockCoords, type);
}
