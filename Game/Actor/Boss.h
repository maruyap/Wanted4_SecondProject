#pragma once

#include <Actor/Actor.h>
#include <vector>
#include <Util/timer.h>

class GameLevel;
using namespace Wanted;

class Boss : public Actor
{
	RTTI_DECLARATIONS(Boss, Actor)

public:
	enum class MonsterState
	{
		None = 0,
		Patrol = 1,
		Chase = 2
	};

	Boss(const Vector2& newPosition);
	bool IsHit(const Vector2& wavePosition) const;
	void OnHit();
	virtual void Tick(float deltaTime) override;
	void Render();
	void PatrolMove(GameLevel& level);
	void BossMovement();
	void ExplodeAround();
	void ResetAttackTimer();
	std::vector<Vector2> GetOccupiedPositions();
	
	int hp = 5;
private:
	Timer attackTimer;
	Timer moveTimer{ 1.0f };
	float waitTime = 0.0f;
	bool isInvincible = false;
	Timer invincibleTimer{ 0.7f };
	Vector2 direction = { 0, 1 };
	int size = 3;
};

