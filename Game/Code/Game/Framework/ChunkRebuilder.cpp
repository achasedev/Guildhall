// #include "Game/Framework/ChunkRebuilder.hpp"
// 
// ChunkRebuilder* ChunkRebuilder::s_instance = nullptr;
// 
// void ThreadWork_Rebuild(void* params)
// {
// 	ChunkData_t chunkData;
// 	while (ChunkRebuilder::GetInstance()->GetNextPendingChunk(chunkData))
// 	{
// 		// Do the rebuild
// 		ChunkData_t data;
// 
// 		int yOffset = (data.chunkIndex / (m_chunkLayout.x * m_chunkLayout.z)) * m_chunkDimensions.y;
// 
// 		int leftOver = chunkIndex % (m_chunkLayout.x * m_chunkLayout.z);
// 		int zOffset = (leftOver / m_chunkLayout.x) * m_chunkDimensions.z;
// 
// 		int xOffset = (leftOver % m_chunkLayout.x) * m_chunkDimensions.x;
// 
// 		MeshBuilder mb;
// 		mb.BeginBuilding(PRIMITIVE_TRIANGLES, true);
// 
// 		for (int i = 0; i < m_chunkDimensions.x; ++i)
// 		{
// 			for (int j = 0; j < m_chunkDimensions.y; ++j)
// 			{
// 				for (int k = 0; k < m_chunkDimensions.z; ++k)
// 				{
// 					IntVector3 coords = IntVector3(i + xOffset, j + yOffset, k + zOffset);
// 
// 					int index = coords.y * (m_dimensions.x * m_dimensions.z) + coords.z * m_dimensions.x + coords.x;
// 
// 					if (m_currentFrame[index].a != 0.f)
// 					{
// 						// Check if we're enclosed
// 						if (!IsVoxelEnclosed(coords))
// 						{
// 							// Get the position
// 							Vector3 position = GetPositionForIndex(index);
// 							mb.PushCube(position, Vector3::ONES, m_currentFrame[index]);
// 						}
// 					}
// 				}
// 			}
// 		}
// 
// 		mb.FinishBuilding();
// 		mb.UpdateMesh(m_chunks[chunkIndex]);
// 
// 
// 		// Push the data to the output queue
// 
// 		// Push neighbors?
// 	}
// }
// 
// ChunkRebuilder::ChunkRebuilder()
// {
// 
// }
// 
// ChunkRebuilder::~ChunkRebuilder()
// {
// }
// 
// void ChunkRebuilder::Initialize()
// {
// 	s_instance = new ChunkRebuilder();
// }
// 
// void ChunkRebuilder::Shutdown()
// {
// 	s_instance->m_isQuitting = true;
// 
// 	if (s_instance != nullptr)
// 	{
// 		delete s_instance;
// 		s_instance = nullptr;
// 	}
// }
// 
// ChunkRebuilder* ChunkRebuilder::GetInstance()
// {
// 	return s_instance;
// }
// 
// void ChunkRebuilder::PushRebuild(unsigned int index)
// {
// 	m_pendingChunks.Enqueue(index);
// }
// 
// void ChunkRebuilder::StartRebuilding(const Rgba* colorData)
// {
// 	// Spin up threads
// 	for (int i = 0; i < NUM_REBUILD_THREADS; ++i)
// 	{
// 		m_threads[i] = Thread::Create(ThreadWork_Rebuild);
// 	}
// 
// 	// Wait for them to finish
// 	for (int i = 0; i < NUM_REBUILD_THREADS; ++i)
// 	{
// 		m_threads[i]->join();
// 		m_threads[i] = nullptr;
// 	}
// 
// 	// Rebuild Again, to catch neighbors
// }
// 
// bool ChunkRebuilder::GetNextPendingChunk(ChunkData_t& chunkData)
// {
// 	return m_pendingChunks.Dequeue(chunkData);
// }
// 
// bool ChunkRebuilder::IsQuitting() const
// {
// 	return m_isQuitting;
// }
