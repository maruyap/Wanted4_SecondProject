#include "Bubble.h"
#include "Level/GameLevel.h"
#include "Actor/Boss.h"
#include "Actor/Player.h"
#include "Render/Renderer.h"
#include "Actor/Wall.h"
#include "Actor/LightBox.h"
#include "Actor/HeavyBox.h"

Bubble::Bubble(const Vector2& position)
	: super("B", position, Color::Blue)
{
	sortingOrder = 14;
}

Bubble::~Bubble()
{
}

void Bubble::Tick(float deltaTime)
{
	super::Tick(deltaTime);
	timer.Tick(deltaTime);
	if (timer.IsTimeOut() == true)
	{
		isTimeOut = true; // 게임레벨에서 폭발 or 연쇄폭발 처리.
	}
    if (this->isSliding)
    {
        this->moveTimer.Tick(deltaTime);

        if (this->moveTimer.IsTimeOut()) 
        {
            this->moveTimer.Reset();
            Vector2 nextPos = position + moveDir;

            // 레벨에게 물어보기 (싱글톤 혹은 레벨 참조 활용).
            if (GameLevel::gameLevelInstance->CanMonsterOrBubbleMove(nextPos))
            {
                position = nextPos;
            }
            else
            {
                isSliding = false; // 막히면 멈춤.
            }
        }
    
        auto& actors = GameLevel::gameLevelInstance->GetActors();
        for (Actor* actor : actors)
        {
            if (actor->IsTypeOf<Boss>())
            {
                Boss* boss = static_cast<Boss*>(actor);

                // 보스의 3x3 영역 안에 물풍선이 들어왔는지 체크.
                if (boss->IsHit(this->GetPosition()))
                { 
                    this->isTimeOut = true;
                    this->isSliding = false; // 이동 중지.
                    break;
                }
            }
        }
    }
}

void Bubble::VisualizeDangerZone()
{
    // 상, 하, 좌, 우 방향 벡터.
    Vector2 dirs[4] = { Vector2(0, -1), Vector2(0, 1), Vector2(-1, 0), Vector2(1, 0) };

    GameLevel* gameLevel = static_cast<GameLevel*>(GetOwner());
    int bombPower = gameLevel->player->bubbleScope;
    // 플레이어의 현재 사거리 사용
    for (int d = 0; d < 4; ++d)
    {
        for (int step = 1; step <= bombPower; ++step)
        {
            Vector2 currentPos = position + (dirs[d] * step);
            bool isBlocked = false;

            for (Actor* actor : gameLevel->GetActors())
            {
                if (actor->DestroyRequested()) continue;

                if (actor->GetPosition() == currentPos)
                {
                    // 벽(Wall)이나 박스류는 줄기를 막음
                    if (actor->IsTypeOf<Wall>() ||
                        actor->IsTypeOf<LightBox>() ||
                        actor->IsTypeOf<HeavyBox>())
                    {
                        isBlocked = true;
                        break;
                    }
                }
            }

            // --- 시각화 제출 ---
            Renderer::Get().Submit("X", currentPos, Color::Red, 1);

            // 벽이나 박스에 막혔다면 이 방향은 여기서 중단.
            if (isBlocked) break;
        }
    }
}


void Bubble::Draw()
{
    super::Draw();

    VisualizeDangerZone();
}
