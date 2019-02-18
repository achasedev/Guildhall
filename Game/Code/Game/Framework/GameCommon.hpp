/************************************************************************/
/* File: GameCommon.hpp
/* Author: Andrew Chase
/* Date: March 24th, 2018
/* Description: Is included in all game .cpp files, to have access to all
				common game elements (renderers, input systems, etc)
/************************************************************************/
#pragma once
#include "Engine/Core/Utility/ErrorWarningAssert.hpp"
#include "Engine/Core/Utility/StringUtils.hpp"

// Camera
constexpr float CAMERA_TRANSLATION_SPEED = 5.0f;
constexpr float CAMERA_ROTATION_SPEED = 45.f;

// World
constexpr float CHUNK_ACTIVATION_RANGE = 32.f;
constexpr float CHUNK_DEACTIVATION_RANGE = CHUNK_ACTIVATION_RANGE + 16.f; // A chunk's worth