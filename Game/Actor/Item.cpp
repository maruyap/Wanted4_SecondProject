#include "Item.h"

Item::Item(const Vector2& position, int itemType)
	: super(
		(itemType == 1) ? "F" : (itemType == 2) ?
		"B" : "K", // 1: 물줄기, 2: 버블, 그외: 신발
		position, Color::Green),
	itemType((ItemType)itemType)
{
	sortingOrder = 1;
}

