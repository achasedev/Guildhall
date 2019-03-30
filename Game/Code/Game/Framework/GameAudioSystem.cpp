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
// Constructor
//
GameAudioSystem::GameAudioSystem()
{
	for (int i = 0; i < NUM_ENEMY_CHANNELS; ++i)
	{
		m_enemyChannels[i] = nullptr;
	}
}


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
void GameAudioSystem::PlayGameSound(eGameSoundType soundType, SoundID soundID, float volume /*= 1.0f*/)
{
	int numChannels = 0;
	FMOD::Channel** channelsToUse = nullptr;

	switch (soundType)
	{
	case SOUND_TYPE_ENEMY:
		channelsToUse = m_enemyChannels;
		numChannels = NUM_ENEMY_CHANNELS;
		break;
	case SOUND_TYPE_WEAPON:
		channelsToUse = m_weaponChannels;
		numChannels = NUM_WEAPON_CHANNELS;

		break;
	case SOUND_TYPE_OTHER:
		channelsToUse = m_otherChannels;
		numChannels = NUM_OTHER_CHANNELS;
		break;
	default:
		break;
	}

	FMOD::Sound* sound = m_registeredSounds[soundID];

	// Find an empty channel
	int indexToPlayOn = -1;
	int countBeingPlayed = 0;

	for (int channelIndex = 0; channelIndex < numChannels; ++channelIndex)
	{
		bool channelExists = channelsToUse[channelIndex] != nullptr;
		FMOD::Sound* soundPlayedOnChannel = nullptr;

		if (channelExists)
		{
			channelsToUse[channelIndex]->getCurrentSound(&soundPlayedOnChannel);

			if (soundPlayedOnChannel != nullptr && soundPlayedOnChannel == sound)
			{
				countBeingPlayed++;
			}
		}

		// Channel hasn't been created or there is no song on the channel
		// Also don't play it if there's already MAX number of this sound being played
		if (!channelExists || soundPlayedOnChannel == nullptr)
		{
			indexToPlayOn = channelIndex;

			// Don't break here, as we need to iterate over all channels to get the count
			// We'll end up getting the last idle channel slot for our sound if we find one
		}
	}

	// If we found a free channel, play the sound if we're under the max play count
	if (indexToPlayOn != -1 && countBeingPlayed < MAX_DUPLICATE_CONCURRENT_SOUNDS)
	{
		m_fmodSystem->playSound(sound, nullptr, false, &channelsToUse[indexToPlayOn]);
		channelsToUse[indexToPlayOn]->setVolume(volume);
		channelsToUse[indexToPlayOn]->setLoopCount(0);
	}
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
