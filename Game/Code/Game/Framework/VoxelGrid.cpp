/************************************************************************/
/* File: VoxelGrid.cpp
/* Author: Andrew Chase
/* Date: September 15th, 2018
/* Description: Implementation of the VoxelGrid class
/************************************************************************/
#include "Game/Entity/Entity.hpp"
#include "Game/Entity/Player.hpp"
#include "Game/Entity/Weapon.hpp"
#include "Game/Framework/Game.hpp"
#include "Game/Framework/World.hpp"
#include "Game/Framework/VoxelGrid.hpp"
#include "Game/Framework/VoxelFont.hpp"
#include "Game/Framework/GameCamera.hpp"
#include "Game/Framework/VoxelTerrain.hpp"
#include "Engine/Assets/AssetDB.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/IntVector3.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Utility/HeatMap.hpp"
#include "Engine/Rendering/Core/Renderer.hpp"
#include "Engine/Core/Time/ProfileLogScoped.hpp"
#include "Engine/Rendering/Shaders/ComputeShader.hpp"
#include "Engine/Rendering/Resources/VoxelTexture.hpp"

// Constants for any size grid
#define VERTICES_PER_VOXEL 24
#define INDICES_PER_VOXEL 36

#define COLOR_BINDING (8)
#define COUNT_BINDING (9)
#define VERTEX_BINDING (10)
#define INDEX_BINDING (11)

bool AreCoordsOnEdge(const IntVector3& coords, const IntVector3& dimensions)
{
	bool xOnEdge = coords.x == 0 || coords.x == dimensions.x - 1;
	bool yOnEdge = coords.y == 0 || coords.y == dimensions.y - 1;
	bool zOnEdge = coords.z == 0 || coords.z == dimensions.z - 1;

	if (xOnEdge && yOnEdge && !zOnEdge) { return true; }
	if (xOnEdge && !yOnEdge && zOnEdge) { return true; }
	if (!xOnEdge && yOnEdge && zOnEdge) { return true; }

	return false;
}


//-----------------------------------------------------------------------------------------------
// Destructor
//
VoxelGrid::~VoxelGrid()
{
	if (m_gridColors != nullptr)
	{
		free(m_gridColors);
		m_gridColors = nullptr;
	}
}


//-----------------------------------------------------------------------------------------------
// Initializes the grid buffers and dimensions
//
void VoxelGrid::Initialize(const IntVector3& voxelDimensions)
{
	m_dimensions = voxelDimensions;

	int numVoxels = m_dimensions.x * m_dimensions.y * m_dimensions.z;

	m_gridColors = (Rgba*)malloc(numVoxels * sizeof(Rgba));
	memset(m_gridColors, 0, numVoxels * sizeof(Rgba));

	for (int i = 0; i < numVoxels; ++i)
	{
		IntVector3 coords = GetCoordsForIndex(i);

		if (AreCoordsOnEdge(coords, m_dimensions))
		{
			m_gridColors[i] = Rgba::GetRandomColor();
		}
	}

	// Initialize mesh building steps
	m_computeShader = new ComputeShader();
	m_computeShader->Initialize("Data/ComputeShaders/VoxelMeshRebuild.cs");

	InitializeBuffers();
}


//-----------------------------------------------------------------------------------------------
// Constructs the mesh for the grid and draws the mesh to screen
//
void VoxelGrid::BuildMeshAndDraw()
{
	PROFILE_LOG_SCOPE_FUNCTION();

	// Rebuild the mesh
	RebuildMesh();

	// Draw the mesh
	DrawGrid();
}


//-----------------------------------------------------------------------------------------------
// Clears the grid to be empty, used at the start of each frame
//
void VoxelGrid::Clear()
{
	PROFILE_LOG_SCOPE_FUNCTION();
	memset(m_gridColors, 0, GetVoxelCount() * sizeof(Rgba));
}


//-----------------------------------------------------------------------------------------------
// Draws the 3D texture to the grid
//
void VoxelGrid::DrawEntity(const Entity* entity, const IntVector3& offset, const Rgba& whiteReplacement /*= Rgba::WHITE*/)
{
	PROFILE_LOG_SCOPE_FUNCTION();

	const VoxelTexture* texture = entity->GetTextureForRender();
	IntVector3 position = entity->GetCoordinatePosition() + offset;

	Draw3DTexture(texture, position, whiteReplacement);

	// Hack to render the player's weapon
	if (entity->IsPlayer())
	{
		const Player* player = dynamic_cast<const Player*>(entity);

		Weapon* weapon = player->GetCurrentWeapon();
		
		if (weapon->GetEntityDefinition()->GetName() != "Pistol")
		{
			const VoxelTexture* weaponTexture = weapon->GetTextureForUIRender();

			if (weaponTexture != nullptr)
			{
				IntVector3 weaponPosition = position + IntVector3(0, 12, 0);
				Draw3DTexture(weaponTexture, weaponPosition);
			}
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Draws the terrain to the grid with the given heightmap
//
void VoxelGrid::DrawTerrain(VoxelTerrain* terrain, const IntVector3& offset)
{
	PROFILE_LOG_SCOPE_FUNCTION();

	for (int heightMapZ = 0; heightMapZ < m_dimensions.z; ++heightMapZ)
	{
		for (int heightMapX = 0; heightMapX < m_dimensions.x; ++heightMapX)
		{
			int height = terrain->GetHeightAtCoords(IntVector2(heightMapX, heightMapZ));

			height = ClampInt(height + offset.y, 0, m_dimensions.y);

			if (height == 0)
			{
				continue;
			}

			int westDiff = AbsoluteValue(terrain->GetHeightAtCoords(IntVector2(heightMapX - 1, heightMapZ)) + offset.y - height);
			int eastDiff = AbsoluteValue(terrain->GetHeightAtCoords(IntVector2(heightMapX + 1, heightMapZ)) + offset.y - height);
			int southDiff = AbsoluteValue(terrain->GetHeightAtCoords(IntVector2(heightMapX, heightMapZ - 1)) + offset.y - height);

			bool hasSharpDiff = (westDiff > 1 || eastDiff > 1 || southDiff > 1);

			IntVector3 gridCoords = IntVector3(heightMapX, height - 1, heightMapZ) + IntVector3(offset.x, 0, offset.z);

			bool isOnGridEdge = (gridCoords.x == 0 || gridCoords.z == 0 || gridCoords.x == m_dimensions.x - 1);

			if (hasSharpDiff || isOnGridEdge)
			{
				int maxFill = ClampInt(MaxInt(MaxInt(westDiff, eastDiff), southDiff), 0, height);

				for (int y = height - 1; y >= height - maxFill; --y)
				{
					Rgba color = terrain->GetColorAtCoords(IntVector3(heightMapX, y, heightMapZ));

					int gridIndex = GetIndexForCoords(IntVector3(gridCoords.x, y, gridCoords.z));

					if (gridIndex >= 0)
					{
						m_gridColors[gridIndex] = color;
					}
				}
			}
			else
			{
				Rgba color = terrain->GetColorAtCoords(IntVector3(heightMapX, height - 1, heightMapZ));

				int gridIndex = GetIndexForCoords(gridCoords);

				if (gridIndex >= 0)
				{
					m_gridColors[gridIndex] = color;
				}
			}
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Draws the 3D texture to the grid
//
void VoxelGrid::Draw3DTexture(const VoxelTexture* texture, const IntVector3& startCoord, const Rgba& whiteReplacement /*= Rgba::WHITE*/)
{
	IntVector3 dimensions = texture->GetDimensions();

	for (int xOff = 0; xOff < dimensions.x; ++xOff)
	{
		for (int yOff = 0; yOff < dimensions.y; ++yOff)
		{
			for (int zOff = 0; zOff < dimensions.z; ++zOff)
			{
				IntVector3 localCoords = IntVector3(xOff, yOff, zOff);

				IntVector3 currCoords = startCoord + localCoords;

				int index = GetIndexForCoords(currCoords);

				if (index != -1)
				{
					Rgba colorToRender = texture->GetColorAtCoords(localCoords);

					if (colorToRender.a != 0)
					{
						// Allow white to be replaced, for player shirt colors
						if (colorToRender == Rgba::WHITE)
						{
							colorToRender = whiteReplacement;
						}

						m_gridColors[index] = colorToRender;
					}
				}
			}
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Draws the collision representation of the entity to the grid
//
void VoxelGrid::DebugDrawEntityCollision(const Entity* entity, const IntVector3& offset)
{
	PROFILE_LOG_SCOPE_FUNCTION();

	const VoxelTexture* texture = entity->GetTextureForRender();
	Vector3 position = entity->GetPosition();
	IntVector3 dimensions = entity->GetDimensions();

	// Coordinate the object occupies (object bottom center)
	IntVector3 coordinatePosition = entity->GetCoordinatePosition() + offset;

	for (int xOff = 0; xOff < dimensions.x; ++xOff)
	{
		for (int yOff = 0; yOff < dimensions.y; ++yOff)
		{
			for (int zOff = 0; zOff < dimensions.z; ++zOff)
			{
				IntVector3 localCoords = IntVector3(xOff, yOff, zOff);
				IntVector3 globalCoords = coordinatePosition + localCoords;

				int globalIndex = GetIndexForCoords(globalCoords);
				if (globalIndex != -1)
				{
					if (texture->DoLocalCoordsHaveCollision(localCoords))
					{
						m_gridColors[globalIndex] = Rgba::RED;
					}
				}
			}
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Draws the given text to the grid
//
void VoxelGrid::DrawVoxelText(const std::string& text, const IntVector3& referenceStart, const VoxelFontDraw_t& options)
{
	IntVector3 textDimensions = options.font->GetTextDimensions(text);
	textDimensions.x *= options.scale.x;
	textDimensions.y *= options.scale.y;
	textDimensions.z *= options.scale.z;

	// Add in the border
 	textDimensions.x += 2 * options.borderThickness;
	textDimensions.y += 2 * options.borderThickness;

	IntVector3 forward = IntVector3(CrossProduct(Vector3(options.right), Vector3(options.up)));
	IntVector3 startWorldCoord = referenceStart;
	startWorldCoord -= options.right * (int)((float)textDimensions.x * options.alignment.x);
	startWorldCoord -= options.up * (int)((float)textDimensions.y * options.alignment.y);
	startWorldCoord -= forward * (int)((float)textDimensions.z * options.alignment.z);

	//.IntVector3 worldSpan = options.right * textDimensions.x + options.up * textDimensions.y + forward * textDimensions.z;

	IntVector3 glyphDimensions = options.font->GetGlyphDimensions();

	for (int zOff = 0; zOff < textDimensions.z; ++zOff)
	{
		for (int yOff = 0; yOff < textDimensions.y; ++yOff)
		{
			for (int xOff = 0; xOff < textDimensions.x; ++xOff)
			{
				int charIndex = ((xOff - options.borderThickness) / options.scale.x) / glyphDimensions.x;
				int xOffset = ((xOff - options.borderThickness) / options.scale.x) % glyphDimensions.x;
				int yOffset = (yOff - options.borderThickness) / options.scale.y;

				IntVector3 worldOffset = options.right * xOff + options.up * yOff + forward * zOff;
				int index = GetIndexForCoords(startWorldCoord + worldOffset);

				if (index == -1)
				{
					continue;
				}

				// Border check
				if ((xOff < options.borderThickness || xOff > textDimensions.x - options.borderThickness - 1) || (yOff < options.borderThickness || yOff > textDimensions.y - options.borderThickness - 1))
				{
					m_gridColors[index] = options.fillColor;
					continue;
				}

				// Get the color, compensating for the border
				Rgba baseColor = options.font->GetColorForGlyphPixel(text[charIndex], IntVector2(xOffset, yOffset));

				if (baseColor.a > 0)
				{
					m_gridColors[index] = options.textColor;
				}
				else if (options.mode == VOXEL_FONT_FILL_FULL)
				{
					m_gridColors[index] = options.fillColor;
				}
			}
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Draws a wire box starting at the given coords and going until dimensions
// Used for drawing borders around text
//
void VoxelGrid::DrawWireBox(const IntVector3& startCoords, const IntVector3& dimensions, const Rgba& color, bool shadeX /*= false*/, bool shadeY /*= false*/, bool shadeZ /*= false*/)
{
	IntVector3 endCoords = startCoords + dimensions;
	for (int y = startCoords.y; y < endCoords.y; ++y)
	{
		for (int z = startCoords.z; z < endCoords.z; ++z)
		{
			for (int x = startCoords.x; x < endCoords.x; ++x)
			{
				bool xOnEdge = (x == startCoords.x || x == endCoords.x - 1);
				bool yOnEdge = (y == startCoords.y || y == endCoords.y - 1);
				bool zOnEdge = (z == startCoords.z || z == endCoords.z - 1);

				bool twoOnEdge = ((xOnEdge && yOnEdge) || (xOnEdge && zOnEdge) || (yOnEdge && zOnEdge));

				bool passXCheck = xOnEdge && shadeX;
				bool passYCheck = yOnEdge && shadeY;
				bool passZCheck = zOnEdge && shadeZ;

				bool passesAShadeCheck = (passXCheck || passYCheck || passZCheck);

				if (twoOnEdge || passesAShadeCheck)
				{
					int index = GetIndexForCoords(IntVector3(x, y, z));

					if (index != -1)
					{
						m_gridColors[index] = color;
					}
				}
			}
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Draws a solid box to the grid of the given color
//
void VoxelGrid::DrawSolidBox(const IntVector3& startCoords, const IntVector3& dimensions, const Rgba& color, bool overwrite /*= true*/)
{
	IntVector3 endCoords = startCoords + dimensions;
	for (int y = startCoords.y; y < endCoords.y; ++y)
	{
		for (int z = startCoords.z; z < endCoords.z; ++z)
		{
			for (int x = startCoords.x; x < endCoords.x; ++x)
			{
				int index = GetIndexForCoords(IntVector3(x, y, z));

				if (index != -1)
				{
					bool shouldWrite = (overwrite || m_gridColors[index].a == 0);

					if (shouldWrite)
					{
						m_gridColors[index] = color;
					}
				}
			}
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Returns the number of voxels in this grid (solid and non-solid)
//
int VoxelGrid::GetVoxelCount() const
{
	return m_dimensions.x * m_dimensions.y * m_dimensions.z;
}


//-----------------------------------------------------------------------------------------------
// Returns the linear index for the voxel given by coords
//
int VoxelGrid::GetIndexForCoords(const IntVector3& coords) const
{
	// Check if it's in bounds first
	if (coords.x >= m_dimensions.x || coords.x < 0 || coords.y >= m_dimensions.y || coords.y < 0 || coords.z >= m_dimensions.z || coords.z < 0)
	{
		return -1;
	}

	return coords.y * (m_dimensions.x * m_dimensions.z) + coords.z * m_dimensions.x + coords.x;
}


//-----------------------------------------------------------------------------------------------
// Returns the coordinates in the grid for the voxel given by index
//
IntVector3 VoxelGrid::GetCoordsForIndex(unsigned int index) const
{
	int y = index / (m_dimensions.x * m_dimensions.z);
	int leftover = index % (m_dimensions.x * m_dimensions.z);

	int z = leftover / m_dimensions.x;
	int x = leftover % m_dimensions.x;

	// Check if in bounds
	if (x >= m_dimensions.x || x < 0 || y >= m_dimensions.y || y < 0 || z >= m_dimensions.z || z < 0)
	{
		return IntVector3(-1, -1, -1);
	}

	return IntVector3(x, y, z);
}


//-----------------------------------------------------------------------------------------------
// Binds and initializes the buffer data for the buffers used for GPU operations
//
void VoxelGrid::InitializeBuffers()
{
	int voxelCount = GetVoxelCount();

	// Color Buffer
	m_colorBuffer.Bind(COLOR_BINDING);
	m_colorBuffer.CopyToGPU(voxelCount * sizeof(Rgba), nullptr);

	// Count Buffer
	m_countBuffer.Bind(COUNT_BINDING);

	int val = 0;
	m_countBuffer.CopyToGPU(sizeof(unsigned int), &val); // Initialize it to 0 so compute can start incrementing

	// Worst/largest case is we have a checkerboard mesh, in which only half the voxels are meshes
	// So we cut the amount in half here
	unsigned int vertexCount = (voxelCount / 4) * VERTICES_PER_VOXEL;
	unsigned int indexCount = (voxelCount / 4) * INDICES_PER_VOXEL;

	// Setup the mesh so the compute shader can directly write to its buffers
	m_mesh.InitializeBuffersForCompute<VertexVoxel>((unsigned int)VERTEX_BINDING, vertexCount, (unsigned int)INDEX_BINDING, indexCount);
}


//-----------------------------------------------------------------------------------------------
// Updates the GPU-side buffers in preparation for the next build
//
void VoxelGrid::UpdateBuffers()
{
	PROFILE_LOG_SCOPE_FUNCTION();

	// Send down the color data
	m_colorBuffer.CopyToGPU(GetVoxelCount() * sizeof(Rgba), m_gridColors);

	// Clear the face count
	unsigned int val = 0;
	m_countBuffer.CopyToGPU(sizeof(unsigned int), &val);
}


//-----------------------------------------------------------------------------------------------
// Initializes the grid buffers and dimensions
//
void VoxelGrid::RebuildMesh()
{
	PROFILE_LOG_SCOPE_FUNCTION();

	// Update the GPU-side buffers
	UpdateBuffers();

	// Execute the build step
	m_computeShader->Execute(m_dimensions.x / 8, m_dimensions.y / 8, m_dimensions.z / 8);

	// Get the vertex and index count from the buffer
	unsigned int* offset = (unsigned int*) m_countBuffer.MapBufferData();
	unsigned int faceOffset = offset[0];
	m_countBuffer.UnmapBufferData();

	// Get the counts
	unsigned int vertexCount = faceOffset * 4;
	unsigned int indexCount = faceOffset * 6;

	// Update the mesh's CPU side data
	m_mesh.UpdateCounts(vertexCount, indexCount);
	m_mesh.SetDrawInstruction(PRIMITIVE_TRIANGLES, true, 0, indexCount);
}


//-----------------------------------------------------------------------------------------------
// Draws the grid mesh to the screen
//
void VoxelGrid::DrawGrid()
{
	PROFILE_LOG_SCOPE_FUNCTION();

	// Draw
	Renderer* renderer = Renderer::GetInstance();
	renderer->SetCurrentCamera(Game::GetGameCamera());

	Renderable rend;
	rend.AddInstanceMatrix(Matrix44::IDENTITY);

	RenderableDraw_t draw;
	draw.mesh = &m_mesh;
	draw.sharedMaterial = AssetDB::CreateOrGetSharedMaterial("Default_Opaque");

	rend.AddDraw(draw);

	renderer->DrawRenderable(&rend);
}
