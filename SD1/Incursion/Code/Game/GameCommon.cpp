/************************************************************************/
/* Project: Incursion
/* File: GameCommon.cpp
/* Author: Andrew Chase
/* Date: September 23rd, 2017
/* Bugs: None
/* Description: Header file to be included in all game .cpp files
/************************************************************************/
#include "Game/GameCommon.hpp"

Renderer* g_theRenderer = nullptr;
InputSystem* g_theInputSystem = nullptr;
AudioSystem* g_theAudioSystem = nullptr;

Game* g_theGame = nullptr;
Camera* g_theCamera = nullptr;
SpriteSheet* g_tileSpriteSheet = nullptr;
SpriteSheet* g_explosionSpriteSheet = nullptr;
SpriteSheet* g_pickUpSpriteSheet = nullptr;
PlayerTank* g_thePlayer = nullptr;

//-----Texture paths-----
const std::string TERRAIN_TEXTURE_PATH				= "Data/Images/Terrain_8x8.png";
const std::string PICKUP_TEXTURE_PATH				= "Data/Images/Extras_4x4.png";
const std::string PLAYER_BASE_TEXTURE_PATH			= "Data/Images/PlayerTankBase.png";
const std::string PLAYER_TURRET_TEXTURE_PATH		= "Data/Images/PlayerTankTop.png";
const std::string ENEMY_TANK_LIGHT_TEXTURE_PATH		= "Data/Images/EnemyLightTank.png";
const std::string ENEMY_TANK_MEDIUM_TEXTURE_PATH	= "Data/Images/EnemyMediumTank.png";
const std::string ENEMY_TANK_HEAVY_TEXTURE_PATH		= "Data/Images/EnemyHeavyTank.png";
const std::string ENEMY_TANK_TD_TEXTURE_PATH		= "Data/Images/EnemyTankDestroyer.png";
const std::string FRIENDLY_TANK_LIGHT_TEXTURE_PATH	= "Data/Images/FriendlyLightTank.png";
const std::string FRIENDLY_TANK_MEDIUM_TEXTURE_PATH	= "Data/Images/FriendlyMediumTank.png";
const std::string FRIENDLY_TANK_HEAVY_TEXTURE_PATH	= "Data/Images/FriendlyHeavyTank.png";
const std::string FRIENDLY_TANK_TD_TEXTURE_PATH		= "Data/Images/FriendlyTankDestroyer.png";
const std::string TURRET_BASE_TEXTURE_PATH			= "Data/Images/EnemyTurretBase.png";
const std::string TURRET_TOP_TEXTURE_PATH			= "Data/Images/EnemyTurretTop.png";
const std::string BULLET_TEXTURE_PATH				= "Data/Images/Bullet.png";
const std::string EXPLOSION_TEXTURE_PATH			= "Data/Images/Explosion_5x5.png";

// Menu textures
const std::string PAUSE_SCREEN_TEXTURE_PATH			= "Data/Images/PauseScreen.png";
const std::string GAME_OVER_SCREEN_TEXTURE_PATH		= "Data/Images/GameOverScreen.png";
const std::string TITLE_SCREEN_TEXTURE_PATH			= "Data/Images/TitleScreen.png";
const std::string TITLE_SCREEN_FLASH_TEXTURE_PATH	= "Data/Images/TitleScreenFlash.png";
const std::string VICTORY_SCREEN_TEXTURE_PATH		= "Data/Images/VictoryScreen.png";
const std::string VICTORY_SCREEN_FLASH_TEXTURE_PATH = "Data/Images/VictoryScreenFlash.png";
const std::string VICTORY_SCREEN_FADE_TEXTURE_PATH  = "Data/Images/VictoryScreenFadeIn.png";
const std::string LOADING_SCREEN_TEXTURE_PATH		= "Data/Images/LoadingScreen.png";

//-----Sound paths-----
const std::string LEVEL_MUSIC_PATH					= "Data/Audio/LevelMusic.wav";
const std::string MAIN_MENU_MUSIC_PATH				= "Data/Audio/MainMenuMusic.mp3";
const std::string VICTORY_MUSIC_PATH				= "Data/Audio/VictoryMusic.wav";

const std::string PLAYER_TANK_ENGINE_SOUND_PATH		= "Data/Audio/TankIdle.wav";
const std::string SHOT_FIRED_SOUND_PATH				= "Data/Audio/BulletFired.wav";
const std::string PLAYER_DAMAGE_TAKEN_SOUND_PATH	= "Data/Audio/PlayerDamageTaken.wav";
const std::string ENEMY_DAMAGE_TAKEN_SOUND_PATH		= "Data/Audio/EnemyDamageTaken.wav";

const std::string PLAYER_PAUSE_SOUND_PATH			= "Data/Audio/PlayerPause.wav";
const std::string PLAYER_UNPAUSE_SOUND_PATH			= "Data/Audio/PlayerUnpause.wav";
const std::string PLAYER_RESPAWN_SOUND_PATH			= "Data/Audio/PlayerRespawn.wav";
const std::string MAP_CHANGE_SOUND_PATH				= "Data/Audio/MapChange.wav";

const std::string EXPLOSION1_SOUND_PATH				= "Data/Audio/Explosion1.wav";
const std::string EXPLOSION2_SOUND_PATH				= "Data/Audio/Explosion2.wav";
const std::string EXPLOSION3_SOUND_PATH				= "Data/Audio/Explosion3.wav";
const std::string EXPLOSION4_SOUND_PATH				= "Data/Audio/Explosion4.wav";
const std::string EXPLOSION5_SOUND_PATH				= "Data/Audio/Explosion5.wav";

const std::string INVINCIBILITY_SOUND_PATH			= "Data/Audio/InvincibilityPickUp.wav";
const std::string HEALTH_SOUND_PATH					= "Data/Audio/HealthPickUp.wav";
const std::string REINFORCEMENTS_SOUND_PATH			= "Data/Audio/ReinforcementsPickUp.wav";
const std::string SPEEDBOOST_SOUND_PATH				= "Data/Audio/SpeedBoostPickUp.wav";

const float CLIENT_ASPECT = (16.f / 9.f);  // We are requesting a 16:9 aspect window area