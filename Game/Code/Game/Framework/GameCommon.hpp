/************************************************************************/
/* File: GameCommon.hpp
/* Author: Andrew Chase
/* Date: March 24th, 2018
/* Description: Is included in all game .cpp files, to have access to all
				common game elements (renderers, input systems, etc)
/************************************************************************/
#pragma once
#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/IntVector3.hpp"
#include "Engine/Core/Utility/ErrorWarningAssert.hpp"
#include "Engine/Core/Utility/StringUtils.hpp"

// Utility Functions
IntVector3 FloorPositionToIntegerCoords(const Vector3& position);

// Camera
constexpr float CAMERA_TRANSLATION_SPEED = 5.0f;
constexpr float CAMERA_ROTATION_SPEED = 180.f;

// World
constexpr float DEFAULT_CHUNK_ACTIVATION_RANGE = 200.f;
constexpr float DEFAULT_CHUNK_DEACTIVATION_OFFSET = 16.f; // A chunk's worth