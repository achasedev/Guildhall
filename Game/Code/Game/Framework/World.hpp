#pragma once

#include <vector>

class Entity;
class Terrain;
class RenderScene;

class World
{
public:
	//-----Public Methods-----

	World();
	~World();

	void Inititalize();

	void Update();
	void Render() const;


private:
	//-----Private Data-----

	Terrain* m_terrain;						// Collection of blocks that make up the environment
	std::vector<Entity*> m_entities;		// Entities in the scene outside of the environment blocks

	RenderScene* m_renderScene;				// Collection of things to render in the world (3D only)

};
