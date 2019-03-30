/************************************************************************/
/* File: GameAudioSystem.hpp
/* Author: Andrew Chase
/* Date: March 30th 2019
/* Description: Class for the audio system used to play audio with balancing
/************************************************************************/
#pragma once
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Core/Time/Stopwatch.hpp"

enum eGameSoundType
{
	SOUND_TYPE_ENEMY,
	SOUND_TYPE_WEAPON,
};

class GameAudioSystem : public AudioSystem
{
public:
	//-----Public Methods-----

	virtual void BeginFrame() override;

	void PlayBGM(const std::string& filename, bool fadeIn = true, bool loop = true);
	void SetBGMVolume(float newVolume, bool transitionTo = true);
	void PlaySystemSound(const std::string& systemSoundName);
	void PlayGameSound(eGameSoundType soundType, SoundID sound, float volume = 1.0f);

	void AddSystemSound(const std::string& soundName, SoundID soundID);

	bool IsBackgroundMusicFinished() const;


private:
	//-----Private Methods-----

	void UpdateMusicCrossfade();


private:
	//-----Private Data-----

	// Statics
	static constexpr int NUM_ENEMY_CHANNELS = 4;
	static constexpr float MUSIC_CROSSFADE_DURATION = 1.0f;

	// Music
	SoundPlaybackID		m_trackTendingToTarget = MISSING_SOUND_ID;
	SoundPlaybackID		m_trackTendingToZero = MISSING_SOUND_ID;
	Stopwatch			m_musicCrossfadeTimer;
	bool				m_musicCrossfading = false;

	float				m_targetMusicVolume = 1.0f;
	float				m_tendingTargetCurrentVolume = 0.f;
	float				m_tendingZeroCurrentVolume = 0.f;


	std::map<std::string, SoundID> m_systemSounds;

	FMOD::Channel* m_enemyChannels[NUM_ENEMY_CHANNELS];

};
