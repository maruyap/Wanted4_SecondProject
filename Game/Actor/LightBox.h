#pragma once
#include "Actor/Actor.h"

using namespace Wanted;

class LightBox : public Actor
{
	RTTI_DECLARATIONS(LightBox, Actor)

public:
	LightBox(const Vector2& newPosition);
	virtual bool IsBlocking() override { return true; } // 몬스터 움직임 체크에 쓸거라 true로.
};

