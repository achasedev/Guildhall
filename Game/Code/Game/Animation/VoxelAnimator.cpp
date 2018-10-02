#include "Game/Framework/Game.hpp"
#include "Game/Animation/AnimationSet.hpp"
#include "Game/Animation/VoxelAnimator.hpp"
#include "Game/Animation/VoxelAnimation.hpp"
#include "Engine/Core/Time/Stopwatch.hpp"

VoxelAnimator::VoxelAnimator(const AnimationSet* animSet, VoxelSprite* defaultSprite)
	: m_animationSet(animSet)
	, m_defaultSprite(defaultSprite)
{
	m_stopwatch = new Stopwatch(Game::GetGameClock());
}

void VoxelAnimator::Play(const std::string& animationAlias, ePlayMode modeOverride /*= PLAYMODE_DEFAULT*/)
{
	std::string nameToUse = animationAlias;

	if (m_animationSet != nullptr)
	{
		std::string translation;
		bool foundTranslation = m_animationSet->TranslateAlias(animationAlias, translation);

		if (foundTranslation)
		{
			nameToUse = translation;
		}
	}

	m_currentAnimation = VoxelAnimation::GetAnimationClip(nameToUse);
	m_stopwatch->Reset();
	m_playmode = modeOverride;
}

VoxelSprite* VoxelAnimator::GetCurrentSprite() const
{
	if (m_currentAnimation != nullptr)
	{
		return m_currentAnimation->Evaluate(m_stopwatch->GetElapsedTime(), m_playmode);
	}
}

bool VoxelAnimator::IsCurrentAnimationFinished() const
{
	float currAnimDuration = m_currentAnimation->GetTotalDuration();
	return (m_stopwatch->GetElapsedTime() >= currAnimDuration);
}

