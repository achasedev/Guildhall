/************************************************************************/
/* Project: Incursion
/* File: GameCommon.hpp
/* Author: Andrew Chase
/* Date: September 23rd, 2017
/* Bugs: None
/* Description: Is included in all game .cpp files, to have access to all
				common game elements (renderers, input systems, etc)
/************************************************************************/
#pragma once
#include <string>

class Renderer;
class InputSystem;
class AudioSystem;
class Game;
class Camera;
class SpriteSheet;
class PlayerTank;

extern Renderer* g_theRenderer;
extern InputSystem* g_theInputSystem;
extern AudioSystem* g_theAudioSystem;
extern Game* g_theGame;
extern Camera* g_theCamera;
extern SpriteSheet* g_tileSpriteSheet;
extern SpriteSheet* g_explosionSpriteSheet;
extern SpriteSheet* g_pickUpSpriteSheet;
extern PlayerTank* g_thePlayer;

// Textures
extern const std::string TERRAIN_TEXTURE_PATH;
extern const std::string PICKUP_TEXTURE_PATH;
extern const std::string PLAYER_BASE_TEXTURE_PATH;
extern const std::string PLAYER_TURRET_TEXTURE_PATH;
extern const std::string ENEMY_TANK_LIGHT_TEXTURE_PATH;
extern const std::string ENEMY_TANK_MEDIUM_TEXTURE_PATH;
extern const std::string ENEMY_TANK_HEAVY_TEXTURE_PATH;
extern const std::string ENEMY_TANK_TD_TEXTURE_PATH;
extern const std::string FRIENDLY_TANK_LIGHT_TEXTURE_PATH;
extern const std::string FRIENDLY_TANK_MEDIUM_TEXTURE_PATH;
extern const std::string FRIENDLY_TANK_HEAVY_TEXTURE_PATH;
extern const std::string FRIENDLY_TANK_TD_TEXTURE_PATH;
extern const std::string TURRET_BASE_TEXTURE_PATH;
extern const std::string TURRET_TOP_TEXTURE_PATH;
extern const std::string BULLET_TEXTURE_PATH;
extern const std::string EXPLOSION_TEXTURE_PATH;

extern const std::string PAUSE_SCREEN_TEXTURE_PATH;
extern const std::string GAME_OVER_SCREEN_TEXTURE_PATH;
extern const std::string TITLE_SCREEN_TEXTURE_PATH;
extern const std::string TITLE_SCREEN_FLASH_TEXTURE_PATH;
extern const std::string VICTORY_SCREEN_TEXTURE_PATH;
extern const std::string VICTORY_SCREEN_FLASH_TEXTURE_PATH;
extern const std::string VICTORY_SCREEN_FADE_TEXTURE_PATH;
extern const std::string LOADING_SCREEN_TEXTURE_PATH;


// Sounds
extern const std::string LEVEL_MUSIC_PATH;
extern const std::string MAIN_MENU_MUSIC_PATH;
extern const std::string VICTORY_MUSIC_PATH;

extern const std::string PLAYER_TANK_ENGINE_SOUND_PATH;
extern const std::string SHOT_FIRED_SOUND_PATH;
extern const std::string PLAYER_DAMAGE_TAKEN_SOUND_PATH;
extern const std::string ENEMY_DAMAGE_TAKEN_SOUND_PATH;

extern const std::string PLAYER_PAUSE_SOUND_PATH;
extern const std::string PLAYER_UNPAUSE_SOUND_PATH;
extern const std::string PLAYER_RESPAWN_SOUND_PATH;
extern const std::string MAP_CHANGE_SOUND_PATH;

extern const std::string EXPLOSION1_SOUND_PATH;
extern const std::string EXPLOSION2_SOUND_PATH;
extern const std::string EXPLOSION3_SOUND_PATH;
extern const std::string EXPLOSION4_SOUND_PATH;
extern const std::string EXPLOSION5_SOUND_PATH;

extern const std::string INVINCIBILITY_SOUND_PATH;
extern const std::string HEALTH_SOUND_PATH;
extern const std::string REINFORCEMENTS_SOUND_PATH;
extern const std::string SPEEDBOOST_SOUND_PATH;

extern const float CLIENT_ASPECT; 