#pragma once

class Mesh;
class Material;
class Renderable;

class MapChunk
{
public:
	//-----Public Methods-----

	MapChunk(Mesh* mesh, Material* material);
	~MapChunk();


private:
	//-----Private Data-----

	Renderable* m_renderable;

};
