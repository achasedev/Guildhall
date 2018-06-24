#include "Game/Entity/Player.hpp"
#include "Game/Entity/Swarmer.hpp"
#include "Game/Framework/Game.hpp"
#include "Game/Framework/GameCommon.hpp"
#include "Engine/Assets/AssetDB.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Rendering/Core/RenderScene.hpp"
#include "Engine/Rendering/Materials/Material.hpp"

// Constants
const float Swarmer::MIN_DOT_TO_MOVE_FORWARD = 0.707f; // Cos(45 degrees)
const float Swarmer::MIN_DOT_TO_SHOOT = 0.96f; // Cos(15 degrees)

const float Swarmer::SWARMER_ROTATION_SPEED = 60.f;
const float Swarmer::SWARMER_TRANSLATION_SPEED = 5.f;

Swarmer::Swarmer(const Vector3& position, unsigned int team)
	: GameEntity(ENTITY_SWARMER)
{
	transform.position = position;
	m_shouldStickToTerrain = true;
	m_shouldOrientToTerrain = true;

	m_team = team;

	m_renderable = new Renderable();

	Mesh* mesh = AssetDB::GetMesh("Sphere");
	Material* material = AssetDB::GetSharedMaterial("Phong_Opaque");

	RenderableDraw_t draw;
	draw.mesh = mesh;
	draw.sharedMaterial = material;

	m_renderable->AddDraw(draw);
	draw.drawMatrix = Matrix44::MakeModelMatrix(Vector3(-0.2f, 0.2f, 1.f), Vector3::ZERO, Vector3(0.1f));
	m_renderable->AddDraw(draw);

	draw.drawMatrix = Matrix44::MakeModelMatrix(Vector3(0.2f, 0.2f, 1.f), Vector3::ZERO, Vector3(0.1f));
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

	m_target = Game::GetPlayer()->transform.position;

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