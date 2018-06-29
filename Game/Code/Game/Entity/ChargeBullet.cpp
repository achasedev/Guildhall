#include "Game/Framework/Game.hpp"
#include "Game/Environment/Map.hpp"
#include "Game/Entity/ChargeBullet.hpp"

ChargeBullet::ChargeBullet(const Vector3& position, const Quaternion& orientation, unsigned int team)
	: Bullet(position, orientation, team)
{
	RenderableDraw_t draw = m_renderable->GetDraw(0);
	draw.drawMatrix = Matrix44::MakeScale(Vector3(2.f)) * draw.drawMatrix;

	m_renderable->ClearDraws();
	m_renderable->AddDraw(draw);

	m_damageAmount = 10;
	m_speed *= 0.5f;
	m_physicsRadius = 3.f;
}

ChargeBullet::~ChargeBullet()
{
}

void ChargeBullet::OnCollisionWithEntity(GameEntity* other)
{
	if (other->GetTeamIndex() != m_team)
	{
		std::vector<GameEntity*> entitiesHit = Game::GetMap()->GetEntitiesInArea(transform.position, m_aoeRadius);

		for (int index = 0; index < (int) entitiesHit.size(); ++index)
		{
			if (entitiesHit[index]->GetTeamIndex() != m_team)
			{
				entitiesHit[index]->TakeDamage(m_damageAmount);
			}
		}

		SetMarkedForDelete(true);
	}
}

