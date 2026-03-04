#include "Bubble.h"
#include "Level/GameLevel.h"
#include "Actor/Boss.h"
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
