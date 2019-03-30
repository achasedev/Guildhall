/************************************************************************/
/* File: GameAudioSystem.cpp
/* Author: Andrew Chase
/* Date: March 30th 2019
/* Description: Implementation of the custom game audio system
/************************************************************************/
#include "Game/Framework/GameAudioSystem.hpp"
#include "Game/Framework/Game.hpp"
#include "Engine/Math/MathUtils.hpp"


//-----------------------------------------------------------------------------------------------
// Update
//
void GameAudioSystem::BeginFrame()
{
	AudioSystem::BeginFrame();

	UpdateMusicCrossfade();
}


//-----------------------------------------------------------------------------------------------
// Updates the crossfade members to properly fade between two tracks
// Does nothing if there's only one track currently playing
//
void GameAudioSystem::UpdateMusicCrossfade()
{
	// Check volume
	if (m_trackTendingToTarget != MISSING_SOUND_ID && m_tendingTargetCurrentVolume != m_targetMusicVolume)
	{
		float delta = Game::GetDeltaTime();

		if (m_tendingTargetCurrentVolume < m_targetMusicVolume)
		{
			m_tendingTargetCurrentVolume = ClampFloat(m_tendingTargetCurrentVolume + delta, 0.f, m_targetMusicVolume);
		}
		else
		{
			m_tendingTargetCurrentVolume = ClampFloat(m_tendingTargetCurrentVolume - delta, m_targetMusicVolume, 1.0f);
		}

		SetSoundPlaybackVolume(m_trackTendingToTarget, m_tendingTargetCurrentVolume);
	}

	if (m_trackTendingToZero != MISSING_SOUND_ID)
	{
		float delta = Game::GetDeltaTime();

		m_tendingZeroCurrentVolume = ClampFloatZeroToOne(m_tendingZeroCurrentVolume - delta);

		if (m_tendingZeroCurrentVolume == 0.f)
		{
			StopSound(m_trackTendingToZero);
			m_trackTendingToZero = MISSING_SOUND_ID;
		}
		else
		{
			SetSoundPlaybackVolume(m_trackTendingToZero, m_tendingZeroCurrentVolume);
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Plays the given background music
//
void GameAudioSystem::PlayBGM(const std::string& filename, bool fadeIn /*= true*/, bool loop /*= true*/)
{
	SoundID nextSong = CreateOrGetSound(filename);

	if (fadeIn)
	{
		if (m_trackTendingToZero != MISSING_SOUND_ID)
		{
			StopSound(m_trackTendingToZero);
		}

		m_trackTendingToZero = m_trackTendingToTarget;
		m_tendingZeroCurrentVolume = m_tendingTargetCurrentVolume;

		m_tendingTargetCurrentVolume = 0.f;
		m_trackTendingToTarget = PlaySound(nextSong, loop, m_tendingTargetCurrentVolume);
	}
	else
	{
		if (m_trackTendingToZero != MISSING_SOUND_ID)
		{
			StopSound(m_trackTendingToZero);
			m_trackTendingToZero = MISSING_SOUND_ID;
			m_tendingZeroCurrentVolume = 0.f;
		}

		if (m_trackTendingToTarget != MISSING_SOUND_ID)
		{
			StopSound(m_trackTendingToTarget);
		}

		m_tendingTargetCurrentVolume = m_targetMusicVolume;
		m_trackTendingToTarget = PlaySound(nextSong, loop, m_tendingTargetCurrentVolume);
	}
}


//-----------------------------------------------------------------------------------------------
// Sets the volume that the background music should be at, and will tranition towards it
// If transitionTo is false, the bgm is immediately set to that volume
//
void GameAudioSystem::SetBGMVolume(float newVolume, bool transitionTo /*= true*/)
{
	m_targetMusicVolume = newVolume;

	if (!transitionTo)
	{
		m_tendingTargetCurrentVolume = newVolume;
		SetSoundPlaybackVolume(m_trackTendingToTarget, m_targetMusicVolume);
	}
}


//-----------------------------------------------------------------------------------------------
// Plays the system sound given by the systemSoundName
//
void GameAudioSystem::PlaySystemSound(const std::string& systemSoundName)
{
	bool soundExists = m_systemSounds.find(systemSoundName) != m_systemSounds.end();

	if (soundExists)
	{
		PlaySound(m_systemSounds[systemSoundName]);
	}
}


//-----------------------------------------------------------------------------------------------
// Plays the sound, taking into consideration the other sounds currently playing on the channel
// groups
//
void GameAudioSystem::PlayGameSound(eGameSoundType soundType, SoundID sound, float volume /*= 1.0f*/)
{
	PlaySound(sound, false, volume);
}


//-----------------------------------------------------------------------------------------------
// Adds the given system sound by name to the registry for playback
//
void GameAudioSystem::AddSystemSound(const std::string& soundName, SoundID soundID)
{
	m_systemSounds[soundName] = soundID;
}


//-----------------------------------------------------------------------------------------------
// Returns true if the main background track is finished playing, for main menu intro loop
//
bool GameAudioSystem::IsBackgroundMusicFinished() const
{
	return IsSoundFinished(m_trackTendingToTarget);
}
