#pragma once
#include "Actor/Actor.h"
#include <vector>
#include "Util/Timer.h"
#include "Level/GameLevel.h"
#include "Util/Astar.h"

using namespace Wanted;

class Monster : public Actor
{
	RTTI_DECLARATIONS(Monster, Actor)

public:
	enum class MonsterState
	{
		None = 0,
		Patrol = 1,
		Chase = 2
	};

	Monster(const Vector2& newPosition);
	~Monster();

	virtual void Tick(float deltaTime) override;
	virtual void BeginPlay() override;
	void Movement();   // 저장된 경로가 현시점에 이동가능한지 체크 후 움직이기.
	void FindPath(Vector2 dest); // bfs 실행해서 path에 경로 저장.

	void UpdatePath(Vector2 dest); // AStar용 경로 저장.
	void PatrolMove(GameLevel& level);
	std::vector<Vector2> path; // 찾은 경로 좌표들.
private:
	Timer moveTimer{ 0.5f };
	Vector2 direction = { 1, 0 };

	AStar* astar = nullptr; // 게임 레벨의 Astar를 받아올 Astar 포인터.
	
	int pathIndex = 0;       // 현재 따라가는 경로 위치.
	Vector2 targetPosition{ -1, -1 }; // 현재 추격중인 타겟 위치.
	MonsterState state = MonsterState::Patrol;
};