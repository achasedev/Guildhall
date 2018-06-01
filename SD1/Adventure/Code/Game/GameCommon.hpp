/************************************************************************/
/* Project: Protogame2D
/* File: GameCommon.hpp
/* Author: Andrew Chase
/* Date: September 23rd, 2017
/* Bugs: None
/* Description: Is included in all game .cpp files, to have access to all
				common game elements (renderers, input systems, etc)
/************************************************************************/
#pragma once
#include <map>
#include <string>

class Renderer;
class InputSystem;
class AudioSystem;
class Player;
class Camera;
class Game;
class AABB2;

extern Renderer* g_theRenderer;
extern InputSystem* g_theInputSystem;
extern AudioSystem* g_theAudioSystem;

extern Player* g_thePlayer;
extern Camera* g_theCamera;
extern Game* g_theGame;

extern AABB2* g_screenBounds;

extern constexpr float GLOBAL_FRICTION = 8.f;
extern bool g_inDebugMode;

// Sounds
extern const std::string ATTRACT_MUSIC;
extern const std::string GAMEOVER_MUSIC;
extern const std::string PLAYING_MUSIC;
extern const std::string VICTORY_MUSIC;