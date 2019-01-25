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

// Game Constants
constexpr float CAMERA_TRANSLATION_SPEED = 5.0f;
constexpr float CAMERA_ROTATION_SPEED = 45.f;