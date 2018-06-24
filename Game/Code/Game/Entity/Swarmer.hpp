#pragma once
#include "Game/Entity/GameEntity.hpp"
#include "Engine/Math/Vector3.hpp"

class Swarmer : public GameEntity
{
public:
	//-----Public Methods-----

	Swarmer(const Vector3& position, unsigned int team);
	~Swarmer();

	virtual void Update(float deltaTime) override;

	virtual void OnCollisionWithEntity(GameEntity* other) override;

	static void InitializeConsoleCommands();


private:
	//-----Private Methods-----

	void FindAndSetTarget();
		Vector3 FindPursueDirection();
		Vector3 FindSeparationDirection(const std::vector<GameEntity*> localFlockmates);
		Vector3 FindAlignmentDirection(const std::vector<GameEntity*> localFlockmates);
		Vector3 FindCohesionDirection(const std::vector<GameEntity*> localFlockmates);
		Vector3 ApplyTargetWeights(const Vector3& pursueTarget, const Vector3& separationTarget, const Vector3& alignmentTarget, const Vector3& cohesionTarget);

	void RotateTowardsTarget();

	bool IsAlmostFacingTarget();
	void MoveForward();

public:
	//-----Public Data-----

	static float s_separationDistance;
	static float s_flockDistance;

	static float s_pursueWeight;
	static float s_separationWeight;
	static float s_alignmentWeight;
	static float s_cohesionWeight;


private:
	//-----Private Data-----

	Vector3 m_target;

	static const float SWARMER_ROTATION_SPEED;
	static const float SWARMER_TRANSLATION_SPEED;

	static const float MIN_DOT_TO_MOVE_FORWARD;
	static const float MIN_DOT_TO_SHOOT;

};
