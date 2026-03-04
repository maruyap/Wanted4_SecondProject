#pragma once

#include "Actor/Actor.h"

using namespace Wanted;

class Wall : public Actor
{
	RTTI_DECLARATIONS(Wall, Actor)

public:
	Wall(const Vector2& position);
	virtual bool IsBlocking() { return true; }
};

