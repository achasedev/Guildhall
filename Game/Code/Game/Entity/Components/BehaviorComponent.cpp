/************************************************************************/
/* File: BehaviorComponent.cpp
/* Author: Andrew Chase
/* Date: October 15th 2018
/* Description: Implementation of the BehaviorComponent base class
/************************************************************************/
#include "Game/Framework/Game.hpp"
#include "Game/Framework/World.hpp"
#include "Game/Entity/Components/BehaviorComponent.hpp"
#include "Engine/Core/Utility/HeatMap.hpp"


//-----------------------------------------------------------------------------------------------
// Constructor
//
BehaviorComponent::BehaviorComponent()
{
}


//-----------------------------------------------------------------------------------------------
// Destructor
//
BehaviorComponent::~BehaviorComponent()
{
}


//-----------------------------------------------------------------------------------------------
// Initialize
//
void BehaviorComponent::Initialize(AnimatedEntity* owningEntity)
{
	m_owningEntity = owningEntity;
}


//-----------------------------------------------------------------------------------------------
// Update
//
void BehaviorComponent::Update()
{
}
