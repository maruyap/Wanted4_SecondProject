#include "Wall.h"

Wall::Wall(const Vector2& position)
	: super("#", position, Color::White)
{
	sortingOrder = 9;
}
