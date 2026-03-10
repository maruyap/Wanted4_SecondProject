#include "Drone.h"
#include "Player.h"
#include "Core/Input.h"
#include "Engine/Engine.h"
#include "Interface/IcanPlayerMove.h"
#include "Level/Level.h"
#include "Actor/Bubble.h"
#include "Level/GameLevel.h"
#include "Game/Game.h"
#include "Actor/Monster.h"

Drone::Drone(const Wanted::Vector2& position)
	: super("D", position, Color::Green)
{
	sortingOrder = 30;
}

void Drone::Tick(float deltaTime)
{
	super::Tick(deltaTime);
	TestBubbleTrigger(deltaTime);
	if (Input::Get().GetKeyDown('E'))
	{
		GameLevel* gameLevel = static_cast<GameLevel*>(GetOwner());

		gameLevel->AddNewActor(new Bubble(this->GetPosition()));
	}
	if (Input::Get().GetKeyDown('W'))
	{
		Vector2 newPosition(GetPosition().x, GetPosition().y - 1);
		SetPosition(newPosition);
	
	}
	if (Input::Get().GetKeyDown('S'))
	{
		Vector2 newPosition(GetPosition().x, GetPosition().y + 1);
		SetPosition(newPosition);
		
	}
	if (Input::Get().GetKeyDown('A'))
	{
		Vector2 newPosition(GetPosition().x - 1, GetPosition().y);
			SetPosition(newPosition);
		
	}
	if (Input::Get().GetKeyDown('D'))
	{
		Vector2 newPosition(GetPosition().x + 1, GetPosition().y);
			SetPosition(newPosition);
	}
}

void Drone::TestBubbleTrigger(float deltaTime)
{
    static float triggerTimer = 0.0f;
    static int TriggerCount = 0;

    GameLevel* gameLevel = static_cast<GameLevel*>(GetOwner());
    if (!gameLevel || TriggerCount >= 2) return;

    // 시간 누적 (예: 7초마다 한 번씩 실행).
    triggerTimer += deltaTime;

    // 7초가 지났을 때 실행.
    if (triggerTimer >= 6.5f)
    {
        int startX = 6;
        int startY[] = { 7, 8, 9, 10, 11 };

        // 물풍선 일괄 생성 및 위험 지역 강제 갱신.
        for (int yPos : startY)
        {
            Vector2 testPos(startX, yPos);
            Bubble* newBubble = new Bubble(testPos);
            gameLevel->AddNewActor(newBubble);

            
        }

        triggerTimer = 0.0f; // 타이머 초기화.
        ++TriggerCount;
    }
}