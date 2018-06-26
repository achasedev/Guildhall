#include "Game/Entity/Player.hpp"
#include "Game/Entity/Swarmer.hpp"
#include "Game/Framework/Game.hpp"
#include "Game/Environment/Map.hpp"
#include "Game/Framework/GameCommon.hpp"
#include "Engine/Assets/AssetDB.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Rendering/Core/RenderScene.hpp"
#include "Engine/Rendering/Materials/Material.hpp"
#include "Engine/Core/DeveloperConsole/Command.hpp"

// Constants
const float Swarmer::MIN_DOT_TO_MOVE_FORWARD = 0.f; // Cos(180 degrees)
const float Swarmer::MIN_DOT_TO_SHOOT = 0.96f; // Cos(15 degrees)

const float Swarmer::SWARMER_ROTATION_SPEED = 60.f;
const float Swarmer::SWARMER_TRANSLATION_SPEED = 5.f;

// Statics
float Swarmer::s_pursueWeight = 1.f;
float Swarmer::s_separationWeight = 10.f;
float Swarmer::s_alignmentWeight = 3.f;
float Swarmer::s_cohesionWeight = 2.f;

float Swarmer::s_separationDistance = 5.f;
float Swarmer::s_flockDistance = 20.f;

// Console commands
void Command_PursueWeight(Command& cmd);
void Command_SeparationWeight(Command& cmd);
void Command_AlignmentWeight(Command& cmd);
void Command_CohesionWeight(Command& cmd);
void Command_SeparationDistance(Command& cmd);
void Command_FlockDistance(Command& cmd);


Swarmer::Swarmer(const Vector3& position, unsigned int team)
	: GameEntity(ENTITY_SWARMER)
{
	transform.position = position;
	m_shouldStickToTerrain = true;
	m_shouldOrientToTerrain = true;

	m_team = team;

	m_renderable = new Renderable();

	Mesh* mesh = AssetDB::GetMesh("Sphere");
	Material* material = AssetDB::GetSharedMaterial("Data/Materials/Tank.material");

	RenderableDraw_t draw;
	draw.mesh = mesh;
	draw.sharedMaterial = material;
	draw.drawMatrix = Matrix44::MakeModelMatrix(Vector3(0.f, 0.5f, 0.f), Vector3::ZERO, Vector3::ONES);

	m_renderable->AddDraw(draw);
	draw.drawMatrix = Matrix44::MakeModelMatrix(Vector3(-0.2f, 0.7f, 1.f), Vector3::ZERO, Vector3(0.1f));
	m_renderable->AddDraw(draw);

	draw.drawMatrix = Matrix44::MakeModelMatrix(Vector3(0.2f, 0.7f, 1.f), Vector3::ZERO, Vector3(0.1f));
	m_renderable->AddDraw(draw);

	m_renderable->AddInstanceMatrix(transform.GetWorldMatrix());

	Game::GetRenderScene()->AddRenderable(m_renderable);

	// Set physics radius
	m_physicsRadius = 1.5f;

	SetHealth(1);
}

Swarmer::~Swarmer()
{
	Game::GetRenderScene()->RemoveRenderable(m_renderable);

	delete m_renderable;
	m_renderable = nullptr;
}

void Swarmer::Update(float deltaTime)
{
	GameEntity::Update(deltaTime);

	FindAndSetTarget();

	RotateTowardsTarget();

	if (IsAlmostFacingTarget())
	{
		MoveForward();
	}

	m_renderable->SetInstanceMatrix(0, transform.GetWorldMatrix());
}


void Swarmer::OnCollisionWithEntity(GameEntity* other)
{
	if (other->GetTeamIndex() != m_team)
	{
		TakeDamage(1000000);
	}

	GameEntity::OnCollisionWithEntity(other);
}

void Swarmer::InitializeConsoleCommands()
{
	Command::Register("pursue_weight",			"Sets the pursue weight for swarmers",			Command_PursueWeight);
	Command::Register("separation_weight",		"Sets the separation weight for swarmers",		Command_SeparationWeight);
	Command::Register("alignment_weight",		"Sets the alignment weight for swarmers",		Command_AlignmentWeight);
	Command::Register("cohesion_weight",		"Sets the cohesion weight for swarmers",		Command_CohesionWeight);
	Command::Register("separation_distance",	"Sets the separation distance for swarmers",	Command_SeparationDistance);
	Command::Register("flock_distance",			"Sets the separation distance for swarmers",	Command_FlockDistance);
}

void Swarmer::FindAndSetTarget()
{
	// Get the flockmates
	std::vector<GameEntity*> flockmates = Game::GetMap()->GetLocalSwarmers(transform.position, s_flockDistance);

	Vector3 pursueDirection = FindPursueDirection();
	Vector3 separationDirection = FindSeparationDirection(flockmates);
	Vector3 alignmentDirection = FindAlignmentDirection(flockmates);
	Vector3 cohesionDirection = FindCohesionDirection(flockmates);

	m_target = ApplyTargetWeights(pursueDirection, separationDirection, alignmentDirection, cohesionDirection);
}


Vector3 Swarmer::FindPursueDirection()
{
	Vector3 targetPos = Game::GetPlayer()->transform.position;
	return (targetPos - transform.position).GetNormalized();
}

Vector3 Swarmer::FindSeparationDirection(const std::vector<GameEntity*> localFlockmates)
{
	// See which flockmates are within the min distance
	Vector3 finalDirection = Vector3::ZERO;

	float minSquaredDist = s_separationDistance * s_separationDistance;

	for (int entityIndex = 0; entityIndex < (int) localFlockmates.size(); ++entityIndex)
	{
		GameEntity* currEntity = localFlockmates[entityIndex];

		Vector3 direction = (currEntity->transform.position - transform.position);

		float distanceSquared = direction.GetLengthSquared();

		if (distanceSquared > 0.f && distanceSquared <= minSquaredDist)
		{
			finalDirection -= direction;
		}
	}

	if (finalDirection != Vector3::ZERO)
	{
		finalDirection.NormalizeAndGetLength();
	}

	return finalDirection;
}

Vector3 Swarmer::FindAlignmentDirection(const std::vector<GameEntity*> localFlockmates)
{
	Vector3 overallForward = Vector3::ZERO;

	for (int entityIndex = 0; entityIndex < (int) localFlockmates.size(); ++entityIndex)
	{
		GameEntity* currEntity = localFlockmates[entityIndex];

		overallForward += currEntity->transform.GetWorldForward();
	}

	if (overallForward != Vector3::ZERO)
	{
		overallForward.NormalizeAndGetLength();
	}

	return overallForward;
}

Vector3 Swarmer::FindCohesionDirection(const std::vector<GameEntity*> localFlockmates)
{
	Vector3 averagePosition = Vector3::ZERO;

	for (int entityIndex = 0; entityIndex < (int) localFlockmates.size(); ++entityIndex)
	{
		GameEntity* currEntity = localFlockmates[entityIndex];

		averagePosition += currEntity->transform.position;
	}

	if (averagePosition != Vector3::ZERO)
	{
		averagePosition /= (int) localFlockmates.size();

		return (averagePosition - transform.position).GetNormalized();
	}
	else
	{
		return Vector3::ZERO;
	}
}

Vector3 Swarmer::ApplyTargetWeights(const Vector3& pursueDirection, const Vector3& separationDirection, const Vector3& alignmentDirection, const Vector3& cohesionDirection)
{
	Vector3 finalDirection = s_pursueWeight * pursueDirection + s_separationWeight * separationDirection + s_alignmentWeight * alignmentDirection + s_cohesionWeight * cohesionDirection;

	if (finalDirection == Vector3::ZERO)
	{
		finalDirection = transform.GetWorldForward();
	}

	finalDirection.NormalizeAndGetLength();

	return transform.position + finalDirection;
}

//-----------------------------------------------------------------------------------------------
// Rotates the tank towards the target position
//
void Swarmer::RotateTowardsTarget()
{
	// Get transforms
	Matrix44 toWorld = transform.GetWorldMatrix();
	Matrix44 toLocal = toWorld.GetInverse();

	// Convert to local space, snap to xz-plane
	Vector3 localPosition = toLocal.TransformPoint(m_target).xyz();
	localPosition.y = 0.f;

	// Get start and end angle values
	Vector3 currentRotation = transform.rotation.GetAsEulerAngles();
	Vector2 dirToTarget = (m_target - transform.position).xz();

	float startAngle = currentRotation.y;
	float endAngle = 180.f - (dirToTarget.GetOrientationDegrees() + 90.f);

	// Find new Y rotation
	float deltaTime = Game::GetDeltaTime();
	float newAngle = TurnToward(startAngle, endAngle, SWARMER_ROTATION_SPEED * deltaTime);

	// Rotate
	currentRotation.y = newAngle;
	transform.rotation = Quaternion::FromEuler(currentRotation);
}


//-----------------------------------------------------------------------------------------------
// Returns true if the turret body is within the minimum threshold for facing the target
//
bool Swarmer::IsAlmostFacingTarget()
{
	Vector2 dirToTarget = (m_target - transform.GetWorldPosition()).xz();
	dirToTarget.NormalizeAndGetLength();

	Vector2 forward = transform.GetWorldForward().xz();
	forward.NormalizeAndGetLength();

	float dot = DotProduct(dirToTarget, forward);

	return dot >= MIN_DOT_TO_MOVE_FORWARD;
}


//-----------------------------------------------------------------------------------------------
// Translates the target along its forward vector
//
void Swarmer::MoveForward()
{
	float deltaTime = Game::GetDeltaTime();
	Vector3 worldForward = transform.GetWorldForward();
	worldForward.y = 0;
	worldForward.NormalizeAndGetLength();

	Vector3 forwardTranslation = worldForward * (SWARMER_TRANSLATION_SPEED * deltaTime);

	transform.TranslateWorld(forwardTranslation);
}

// Commands

void Command_PursueWeight(Command& cmd)
{
	float newValue = 1.f;
	cmd.GetParam("v", newValue, &newValue);

	Swarmer::s_pursueWeight = newValue;

	ConsolePrintf(Rgba::GREEN, "Pursue Weight set to %f", newValue);
}

void Command_SeparationWeight(Command& cmd)
{
	float newValue = 1.f;
	cmd.GetParam("v", newValue, &newValue);

	Swarmer::s_separationWeight = newValue;

	ConsolePrintf(Rgba::GREEN, "Separation Weight set to %f", newValue);
}

void Command_AlignmentWeight(Command& cmd)
{
	float newValue = 1.f;
	cmd.GetParam("v", newValue, &newValue);

	Swarmer::s_alignmentWeight = newValue;

	ConsolePrintf(Rgba::GREEN, "Alignment Weight set to %f", newValue);
}

void Command_CohesionWeight(Command& cmd)
{
	float newValue = 1.f;
	cmd.GetParam("v", newValue, &newValue);

	Swarmer::s_cohesionWeight = newValue;

	ConsolePrintf(Rgba::GREEN, "Cohesion Weight set to %f", newValue);
}

void Command_SeparationDistance(Command& cmd)
{
	float newValue = 1.f;
	cmd.GetParam("v", newValue, &newValue);

	Swarmer::s_separationDistance = newValue;

	ConsolePrintf(Rgba::GREEN, "Separation Distance set to %f", newValue);
}

void Command_FlockDistance(Command& cmd)
{
	float newValue = 1.f;
	cmd.GetParam("v", newValue, &newValue);

	Swarmer::s_flockDistance = newValue;

	ConsolePrintf(Rgba::GREEN, "Flock distance set to %f", newValue);
}

