#include "HeavyBox.h"
#include "Util/Util.h"
#include "Item.h"

HeavyBox::HeavyBox(const Vector2& newPosition)
	: super("H", newPosition, Color::White)
{
	sortingOrder = 6;
}

