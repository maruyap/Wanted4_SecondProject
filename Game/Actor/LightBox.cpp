#include "LightBox.h"

LightBox::LightBox(const Vector2& newPosition)
	: super("L", newPosition, Color::White)
{
	sortingOrder = 4;
}