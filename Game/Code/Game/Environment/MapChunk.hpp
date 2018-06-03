/************************************************************************/
/* File: MapChunk.hpp
/* Author: Andrew Chase
/* Date: June 3rd, 2018
/* Description: Class to represent a single renderable chunk of the world map
/************************************************************************/
#pragma once

class Mesh;
class Material;
class Matrix44;
class Renderable;

class MapChunk
{
public:
	//-----Public Methods-----

	MapChunk(const Matrix44& model, Mesh* mesh, Material* material);
	~MapChunk();


private:
	//-----Private Data-----

	Renderable* m_renderable;

};
