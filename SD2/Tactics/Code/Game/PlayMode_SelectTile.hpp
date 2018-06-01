/************************************************************************/
/* File: PlayMode_SelectTile.hpp
/* Author: Andrew Chase
/* Date: March 12th, 2018
/* Description: Mode for handling a tile/block selection by the player
/************************************************************************/
#pragma once
#include <vector>
#include <functional>

#include "Game/PlayMode.hpp"

// Callback function, called when a tile is selected
typedef std::function<void(bool, const IntVector3&)> TileSelectionCallback;

class PlayMode_SelectTile : public PlayMode
{
public:
	//-----Public Methods-----

	// Takes the controller for the actor, a list of tile that can be selected, and the callback
	PlayMode_SelectTile(ActorController* controller, std::vector<IntVector3>& selectableCoords, TileSelectionCallback callback);

	// Virtual Methods
	virtual void ProcessInput() override;
	virtual void Update() override;
	virtual void RenderWorldSpace() const override;
	virtual void RenderScreenSpace() const override;
	virtual void Exit() override;


private:
	//-----Private Methods-----

	// Input
	void ProcessSelectionInput();		// Checks if a tile is selected

	// Update
	bool IsCursorInSelection() const;	// Returns true if the cursor is in the selection

	// Render
	void RenderTiles() const;			// Draws the selection tiles


private:
	//-----Private Data-----

	TileSelectionCallback	m_callback;				// Callback for a selection event
	std::vector<IntVector3> m_selectableCoords;		// List of selectable tiles

};
