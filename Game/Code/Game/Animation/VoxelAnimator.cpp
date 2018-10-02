#include "Game/Framework/Game.hpp"
#include "Game/Animation/VoxelAnimationSet.hpp"
#include "Game/Animation/VoxelAnimator.hpp"
#include "Game/Animation/VoxelAnimation.hpp"
#include "Engine/Core/Time/Stopwatch.hpp"

VoxelAnimator::VoxelAnimator(const VoxelAnimationSet* animSet, const VoxelSprite* defaultSprite)
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

	const VoxelAnimation* animation = VoxelAnimation::GetAnimationClip(nameToUse);
	if (animation != m_currentAnimation)
	{
		m_currentAnimation = animation;
		m_stopwatch->Reset();
	}

	m_playmode = modeOverride;
}

const VoxelSprite* VoxelAnimator::GetCurrentSprite() const
{
	if (m_currentAnimation != nullptr)
	{
		return m_currentAnimation->Evaluate(m_stopwatch->GetElapsedTime(), m_playmode);
	}

	return m_defaultSprite;
}

bool VoxelAnimator::IsCurrentAnimationFinished() const
{
	float currAnimDuration = m_currentAnimation->GetTotalDuration();
	return (m_stopwatch->GetElapsedTime() >= currAnimDuration);
}

