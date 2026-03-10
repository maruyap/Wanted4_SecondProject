#pragma once

#include "Actor/Actor.h"
#include "Math/Vector2.h"

using namespace Wanted;

class Drone : public Actor
{
	RTTI_DECLARATIONS(Drone, Actor)

public:
	Drone(const Wanted::Vector2& position);
	virtual void Tick(float deltaTime) override;
	void TestBubbleTrigger(float deltaTime);
};

