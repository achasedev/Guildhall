#pragma once
#include "Engine/Rendering/Core/Vertex.hpp"
#include "Engine/Core/Threading/Threading.hpp"
#include "Engine/DataStructures/ThreadSafeQueue.hpp"
#include "Engine/Math/IntVector3.hpp"

#define NUM_REBUILD_THREADS 8

struct ChunkData_t
{
	unsigned int* indices;
	Vertex3D_PCU* vertices;
	unsigned int chunkIndex;
	IntVector3 chunkDimensions;

};

class ChunkRebuilder
{

public:
	//-----Public Methods-----

	static void Initialize();
	static void Shutdown();

	static ChunkRebuilder* GetInstance();

	void PushRebuild(unsigned int index);
	void StartRebuilding(const Rgba* colorData, const IntVector3& worldVoxelDimensions);

	bool GetNextPendingChunk(ChunkData_t& chunkData);
	
	bool IsQuitting() const;

private:
	//-----Private Methods-----

	ChunkRebuilder();
	~ChunkRebuilder();
	ChunkRebuilder(const ChunkRebuilder& copy) = delete;


private:
	//-----Private Data-----

	ThreadHandle_t m_threads[NUM_REBUILD_THREADS];
	ThreadSafeQueue<ChunkData_t> m_pendingChunks;
	ThreadSafeQueue<ChunkData_t> m_finishedChunks;

	bool m_isQuitting = false;

	static ChunkRebuilder* s_instance;

};
