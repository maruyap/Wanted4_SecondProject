#pragma once

#include "Actor/Actor.h"
#include "../Util/Timer.h"

class GameLevel;

using namespace Wanted;

class Bubble : public Actor
{
	RTTI_DECLARATIONS(Bubble, Actor)

public:
	Bubble(const Vector2& position);
	~Bubble();

	virtual void Tick(float deltaTime) override;
	virtual void Draw() override;
	virtual bool IsBlocking() { return true; } // cankick = false인경우.
	
	void VisualizeDangerZone();
public:
	Vector2 moveDir = { 0, 0 };

	Timer moveTimer{ 0.1f }; // 플레이어가 물풍선을 찰 때의 속도.
	Timer timer{ 3.0f };

	bool isSliding = false;
	bool isTimeOut = false;
	
	GameLevel* gameLevelPtr = nullptr;
};

