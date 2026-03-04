#include "Drone.h"
#include "Player.h"
#include "Core/Input.h"
#include "Engine/Engine.h"
#include "Interface/IcanPlayerMove.h"
#include "Level/Level.h"
#include "Actor/Bubble.h"
#include "Level/GameLevel.h"
#include "Game/Game.h"

Drone::Drone(const Wanted::Vector2& position)
	: super("D", position, Color::Green)
{
	sortingOrder = 30;
}

void Drone::Tick(float deltaTime)
{
	super::Tick(deltaTime);

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

