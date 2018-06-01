/************************************************************************/
/* File: GameCommon.cpp
/* Author: Andrew Chase
/* Date: November 29th, 2017
/* Bugs: None
/* Description: Header file to be included in all game .cpp files
/************************************************************************/
#include "Game/GameCommon.hpp"

Renderer* g_theRenderer = nullptr;
InputSystem* g_theInputSystem = nullptr;
AudioSystem* g_theAudioSystem = nullptr;
Player* g_thePlayer = nullptr;
Camera* g_theCamera = nullptr;
Game* g_theGame = nullptr;
AABB2* g_screenBounds = nullptr;

bool g_inDebugMode = false;

const std::string ATTRACT_MUSIC		= "Data/Audio/Attract.mp3";
const std::string PLAYING_MUSIC		= "Data/Audio/Playing.mp3";
const std::string GAMEOVER_MUSIC	= "Data/Audio/GameOver.mp3";
const std::string VICTORY_MUSIC		= "Data/Audio/Victory.wav";