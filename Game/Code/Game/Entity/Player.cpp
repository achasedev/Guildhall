#include "Game/Entity/Player.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Math/Vector3.hpp"

void Player::ProcessInput()
{
	XboxController& controller = InputSystem::GetInstance()->GetController(m_playerID);

	Vector2 leftStick = controller.GetCorrectedStickPosition(XBOX_STICK_LEFT);

	m_force = Vector3(leftStick.x, 0.f, leftStick.y) * 10.f;

	if (leftStick.GetLengthSquared() > 0.f)
	{
		m_orientation = leftStick.GetOrientationDegrees();
	}
}

void Player::Update()
{
	DynamicEntity::Update();
}

