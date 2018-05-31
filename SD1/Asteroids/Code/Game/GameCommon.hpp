/************************************************************************/
/* Project: SD1 C27 A3 - Asteroids Gold
/* File: GameCommon.hpp
/* Author: Andrew Chase
/* Date: September 13rd, 2017
/* Bugs: None
/* Description: Is included in all game .cpp files, to have access to all
				common game elements (renderers, input systems, etc)
/************************************************************************/
#pragma once


class Renderer;
class InputSystem;

extern Renderer* g_theRenderer;
extern InputSystem* g_theInputSystem;
