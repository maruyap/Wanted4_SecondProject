#include "Player.h"
#include "Core/Input.h"
#include "Engine/Engine.h"
#include "Interface/IcanPlayerMove.h"
#include "Level/Level.h"
#include "Actor/Bubble.h"
#include "Level/GameLevel.h"
#include "Game/Game.h"

Player::Player()
	: super("P", Vector2::Zero, Color::Green)
{
	sortingOrder = 20;
}

Player::Player(const Vector2& position)
	: super("P", position, Color::Green)
{
	sortingOrder = 15;
}

Player::~Player()
{
}

void Player::Tick(float deltaTime)
{
	super::Tick(deltaTime); // 다형성으로 player::tick만 실행되니
	                        // 레벨단tick에서 해줄게 있다면 동작하게끔.

		// 인터페이스 확인.
	static ICanPlayerMove* canPlayerMoveInterface = nullptr;

	// 오너십 확인 (null 확인).
	if (!canPlayerMoveInterface && GetOwner())
	{
		// 인터페이스로 형변환.
		canPlayerMoveInterface = dynamic_cast<ICanPlayerMove*>(GetOwner());
	}

	if (Input::Get().GetKeyDown(VK_SPACE))
	{
		this->wantSpawnBubble = true;
	}
	if (Input::Get().GetKeyDown(VK_UP))
	{
		Vector2 newPosition(GetPosition().x, GetPosition().y - 1);
		if (canPlayerMoveInterface->CanMove(GetPosition(), newPosition))
		{
			SetPosition(newPosition);
		}
	}
	if (Input::Get().GetKeyDown(VK_DOWN))
	{
		Vector2 newPosition(GetPosition().x, GetPosition().y + 1);
		if (canPlayerMoveInterface->CanMove(GetPosition(), newPosition))
		{
			SetPosition(newPosition);
		}
	}
	if (Input::Get().GetKeyDown(VK_LEFT))
	{
		Vector2 newPosition(GetPosition().x - 1, GetPosition().y);
		if (canPlayerMoveInterface->CanMove(GetPosition(), newPosition))
		{
			SetPosition(newPosition);
		}
	}
	if (Input::Get().GetKeyDown(VK_RIGHT))
	{
		Vector2 newPosition(GetPosition().x + 1, GetPosition().y);
		if (canPlayerMoveInterface->CanMove(GetPosition(), newPosition))
		{
			SetPosition(newPosition);
		}	
	}
}

void Player::Draw()
{
	Actor::Draw();
}

