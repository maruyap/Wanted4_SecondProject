#pragma once
#include "Math/Vector2.h"
// 콘솔 좌표에서 위치를 나타날 클래스.
using namespace Wanted;

class Node
{
public:
	Node(int x, int y, Node* parentNode = nullptr)
		: vector2(x, y), parentNode(parentNode)
	{
	}

	Vector2 operator-(const Node& other) const
	{
		return Vector2(
			vector2.x - other.vector2.x,
			vector2.y - other.vector2.y
		);
	}

	bool operator==(const Node& other) const
	{
		return vector2.x == other.vector2.x
			&& vector2.y == other.vector2.y;
	}

public:
	// 이 노드의 위치.
	Vector2 vector2;

	// 시작 위치에서 이 노드까지의 비용.
	int gCost = 0;

	// 이 노드에서 목표지점까지의 예상 비용(휴리스틱).
	int hCost = 0;

	// 최종 비용(gCost+hCost)
	int fCost = 0;

	// 부모 노드 링크.
	Node* parentNode = nullptr;
};