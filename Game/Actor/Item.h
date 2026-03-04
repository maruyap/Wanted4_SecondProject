#pragma once

#include <Actor/Actor.h>

using namespace Wanted;

class Item : public Actor
{
	RTTI_DECLARATIONS(Item, Actor)

public:
	enum class ItemType
	{
		None = 0,
		flask = 1,
		Balloon = 2,
		KickShoes = 3
	};
	
	Item(const Vector2& position, int itemType);
	
public:
	ItemType itemType = ItemType::None;
};

