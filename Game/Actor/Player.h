#pragma once

#include "Actor/Actor.h"

using namespace Wanted;

class Player : public Actor
{
	RTTI_DECLARATIONS(Player, Actor)

public:
	Player();
	Player(const Vector2& position);
	~Player();

private:
	virtual void Tick(float deltaTime) override;
	virtual void Draw() override;
	
public:
	bool wantSpawnBubble = false;
	bool canKick = false;
	int bubbleLimit = 2;
	int bubbleScope = 4; // 물풍선 위험지역 시각화를 더 뚜렷하게 보기위해
	//int bubbleScope = 1;
};

