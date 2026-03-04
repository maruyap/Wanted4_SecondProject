#pragma once

#include "Actor/Actor.h"

using namespace Wanted;

class HeavyBox : public Actor
{
	RTTI_DECLARATIONS(HeavyBox, Actor)

public:
	HeavyBox(const Vector2& newPosition);
	virtual bool IsBlocking() { return true; }
};

