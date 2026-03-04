#pragma once

#include "Level/Level.h"
#include "Interface/IcanPlayerMove.h"
#include "Util/Util.h"

using namespace Wanted;

class Player;

class GameLevel : public Level,
	public ICanPlayerMove
{
	RTTI_DECLARATIONS(GameLevel, Level)

public:
	static GameLevel* gameLevelInstance;
	GameLevel();
	~GameLevel();

	virtual void OnActorDestroyed(Actor* actor) override;

	virtual bool CanMove(
		const Wanted::Vector2& playerPosition,
		const Wanted::Vector2& nextPosition) override; // 플레이어 이동 관련 상호작용 함수.
	virtual void Tick(float deltatime) override;
	void CollisionWaveAndOther(const Vector2& position); // 물풍선 폭발 관련 상호작용 함수.
	void SpawnItem(const Vector2& position, int itemType);
	void SpawnBubble();
	void Bomb(); // 물풍선이 폭발할 시점인지 체크하고 폭발 함수를 호출해주는 함수.
	void CollisionPlayerAndOther(); // 플레이어와 충돌 관련 상호작용 함수.
	bool CanBossMove(const Vector2& centerPosition); // 충돌범위 9칸 보스의 이동 가능 여부 체크 함수.
	Vector2 GetPlayerPosition();

	bool CanMonsterOrBubbleMove(const Vector2& nextPosition); // 물풍선과 몬스터가 해당 위치로 이동이 가능한지 체크하는 함수.
	void ClearActors();
	int MonsterCounter(); 
	void NextStage();
	void EnemyAllKill();

	void DrawUI(); // 플레이어 UI 띄워주는 함수

private:
	void LoadMap(const char* filename);

private:
	Player* player = nullptr;
	int playerLifeCount = 3;
	Vector2 playerStartPosition = { -1, -1 };
};
