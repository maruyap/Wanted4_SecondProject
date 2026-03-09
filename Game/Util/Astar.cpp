#include "AStar.h"

#include <algorithm>
#include <cmath>
#include <Windows.h>
#include <iostream>
#include <set>
#include <Render/Renderer.h>
#include <Level/GameLevel.h>
using namespace Wanted;
AStar::AStar()
{
}

AStar::~AStar()
{
	ClearLists();
}

void AStar::ClearLists()
{
	// 마스터 리스트에 담긴 것만 다 지우기.
	for (Node* node : allNodes) {
		if (node) {
			delete node;
		}
	}
	allNodes.clear();      // 해제된 주소를 가리키는 주소값 비우기.
	openList.clear();      
	closedList.clear();   

	startNode = nullptr;
	goalNode = nullptr;
}
/*
std::vector<Vector2> AStar::FindPath(
	Vector2 startPos, Vector2 goalPos,
	std::vector<std::vector<int>>& grid,
	std::vector<std::vector<int>>& dangerMap)
{
	// 이전 탐색 데이터 및 메모리 완전 정리.
	ClearLists();

	// 시작/목표 노드 생성 MakeNewNode 사용
	this->startNode = MakeNewNode((int)startPos.x, (int)startPos.y);
	this->goalNode = MakeNewNode((int)goalPos.x, (int)goalPos.y);

	if (!this->startNode || !this->goalNode || grid.empty() || grid[0].empty())
	{
		return { };
	}

	// 시작 노드 설정.
	openList.emplace_back(this->startNode);

	std::vector<Direction> directions =
	{
		{ -1, 0, 1 },  { 1, 0, 1 },{ 0, -1, 1 }, { 0, 1, 1 }
	};

	while (!openList.empty())
	{
		// fCost가 가장 낮은 노드 검색.
		Node* currentNode = openList[0];
		for (Node* const node : openList)
		{
			if (node->fCost < currentNode->fCost)
			{
				currentNode = node;
			}
		}

		// 목적지 도착 확인.
		if (IsDestination(currentNode))
		{
			return ConstructPath(currentNode);
		}

		// 열린 리스트에서 현재 노드 제거.
		for (auto it = openList.begin(); it != openList.end(); ++it)
		{
			if ((*it) == currentNode)
			{
				openList.erase(it);
				break;
			}
		}

		// 방문 노드(닫힌 리스트)에 추가.
		closedList.emplace_back(currentNode);

		// 이웃 노드 탐색.
		for (const Direction& direction : directions)
		{
			int newX = currentNode->vector2.x + direction.x;
			int newY = currentNode->vector2.y + direction.y;

			// 유효성 및 장애물 검사.
			if (!IsInRange(newX, newY, grid) || grid[newY][newX] == 1)
			{
				continue;
			}

			int newGCost = currentNode->gCost + direction.cost;

			// 이미 방문했는지 확인.
			if (HasVisited(newX, newY, newGCost))
			{
				continue;
			}

			// 임시 노드 생성을 지양 필요할 때만 MakeNewNode를 호출
			// 먼저 열린 리스트에 이미 해당 위치의 노드가 있는지 검색.
			Node* openListNode = nullptr;
			for (Node* const node : openList)
			{
				if (node->vector2.x == newX && node->vector2.y == newY)
				{
					openListNode = node;
					break;
				}
			}

			if (openListNode)
			{
				// 이미 열린 리스트에 있다면 비용 비교 후 업데이트만 진행
				if (newGCost < openListNode->gCost)
				{
					openListNode->parentNode = currentNode;
					openListNode->gCost = newGCost;
					openListNode->fCost = openListNode->gCost + openListNode->hCost;
				}
				continue;
			}

			// 여기에 없어야만 새로운 노드를 생성.
			Node* neighborNode = MakeNewNode(newX, newY);
			neighborNode->parentNode = currentNode;
			neighborNode->gCost = newGCost;
			neighborNode->hCost = CalculateHeuristic(neighborNode, this->goalNode);
			neighborNode->fCost = neighborNode->gCost + neighborNode->hCost;

			if (grid[newY][newX] == 0)
			{
				grid[newY][newX] = 5; // 탐색 시각화.
			}

			openList.emplace_back(neighborNode);
		}
	}

	return { };
}
*/

std::vector<Vector2> AStar::FindPath(
	Vector2 startPos, Vector2 goalPos,
	std::vector<std::vector<int>>& grid,
	std::vector<std::vector<int>>& dangerMap)
{
	if (grid[goalPos.y][goalPos.x] == 1)
	{
		return {};
	}
	// 이전 탐색 데이터 및 메모리 완전 정리.
	ClearLists();

	// 시작/목표 노드 생성 MakeNewNode 사용
	this->startNode = MakeNewNode((int)startPos.x, (int)startPos.y);
	this->goalNode = MakeNewNode((int)goalPos.x, (int)goalPos.y);

	if (!this->startNode || !this->goalNode || grid.empty() || grid[0].empty())
	{
		return { };
	}

	// 시작 노드 설정.
	openList.emplace_back(this->startNode);

	// 이동 비용 단위를 10으로 설정.
	std::vector<Direction> directions =
	{
		{ -1, 0, 10 },  { 1, 0, 10 },{ 0, -1, 10 }, { 0, 1, 10 }
	};

	while (!openList.empty())
	{
		// fCost가 가장 낮은 노드 검색.
		Node* currentNode = openList[0];
		for (Node* const node : openList)
		{
			if (node->fCost < currentNode->fCost)
			{
				currentNode = node;
			}
			// fCost가 같다면 목표에 더 가까운(hCost가 낮은) 노드를 선택
			else if (node->fCost == currentNode->fCost)
			{
				if (node->hCost < currentNode->hCost)
				{
					currentNode = node;
				}
			}
		}

		if (isDebuged)
		{
			GameLevel::gameLevelInstance->Draw();
			

			// 현재까지 탐색된 모든 열린 리스트와 닫힌 리스트 시각화
		
			for (Node* node : allNodes)
			{
				// 이미 탐색 중인 노드는 '+' 기호로 표시
				Renderer::Get().Submit("+", node->vector2, Color::Green, 100);
			}
			// 현재 검사 중인 노드는 특별한 색으로 강조
			Renderer::Get().Submit("@", currentNode->vector2, Color::Red, 101);
			
			// 더블 버퍼링 강제 출력 (이게 호출되어야 화면에 나타남)
			Renderer::Get().Draw();


			// 시각화를 위해 잠깐 멈추기
			DWORD delay = static_cast<DWORD>(0.1f * 1000);
			Sleep(delay);
		}

		// 목적지 도착 확인.
		if (IsDestination(currentNode))
		{
			std::vector<Vector2> path = ConstructPath(currentNode);

			if (this->isDebuged)
			{
				// 최종 경로를 하나씩 그리는 루프
				for (const auto& pos : path)
				{
					// 배경(액터들)을 새로 그려서 이전 탐색 흔적(+) 지우기.
					if (GameLevel::gameLevelInstance) {
						GameLevel::gameLevelInstance->Draw();
					}

					// 그 위에 최종 경로만 올리기(우선순위를 높게 설정)
					// (이미 지나온 경로들도 함께 보여주려면 path의 현재 인덱스까지 루프를 돌며 Submit해야함)
					for (const auto& p : path) {
						if (p == pos) break; // 현재 그리는 지점까지만 표시
						Renderer::Get().Submit("*", p, Color::White, 102);
					}
					Renderer::Get().Submit("*", pos, Color::White, 102);

					// 화면에 출력
					Renderer::Get().Draw();

					// 연출 속도 조절
					Sleep(80);
				}

				this->isDebuged = false; // 연출 종료
			}
			return path;
		}

		// 열린 리스트에서 현재 노드 제거.
		for (auto it = openList.begin(); it != openList.end(); ++it)
		{
			if ((*it) == currentNode)
			{
				openList.erase(it);
				break;
			}
		}

		// 방문 노드(닫힌 리스트)에 추가.
		closedList.emplace_back(currentNode);

		// 이웃 노드 탐색.
		for (const Direction& direction : directions)
		{
			int newX = currentNode->vector2.x + direction.x;
			int newY = currentNode->vector2.y + direction.y;

			// 유효성 및 장애물 검사.
			if (!IsInRange(newX, newY, grid) || grid[newY][newX] == 1)
			{
				continue;
			}

			// 위험 지역(dangerMap == 1)일 경우 가중치(70)를 더함.
			// 이동 비용 단위가 10이므로, 70을 더하면 약 7칸을 돌아가는 것과 같은 비용이 됨.
			int weight = (dangerMap[newY][newX] == 1) ? 70 : 0;
			int newGCost = currentNode->gCost + direction.cost + weight;

			// 이미 방문했는지 확인.
			if (HasVisited(newX, newY, newGCost))
			{
				continue;
			}

			// 임시 노드 생성을 지양 필요할 때만 MakeNewNode를 호출
			// 먼저 열린 리스트에 이미 해당 위치의 노드가 있는지 검색.
			Node* openListNode = nullptr;
			for (Node* const node : openList)
			{
				if (node->vector2.x == newX && node->vector2.y == newY)
				{
					openListNode = node;
					break;
				}
			}

			if (openListNode)
			{
				// 이미 열린 리스트에 있다면 비용 비교 후 업데이트만 진행.
				if (newGCost < openListNode->gCost)
				{
					openListNode->parentNode = currentNode;
					openListNode->gCost = newGCost;
					openListNode->fCost = openListNode->gCost + openListNode->hCost;
				}
				continue;
			}

			// 여기에 없어야만 새로운 노드를 생성.
			Node* neighborNode = MakeNewNode(newX, newY);
			neighborNode->parentNode = currentNode;
			neighborNode->gCost = newGCost;
			neighborNode->hCost = CalculateHeuristic(neighborNode, this->goalNode);
			neighborNode->fCost = neighborNode->gCost + neighborNode->hCost;

			if (grid[newY][newX] == 0)
			{
				grid[newY][newX] = 5; // 탐색 시각화.
			}

			openList.emplace_back(neighborNode);
		}
	}

	return { };
}
void AStar::DisplayGridWithPath(
	std::vector<std::vector<int>>& grid,
	const std::vector<Node*>& path)
{
	static COORD position = { 0,0 };
	static HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleCursorPosition(handle, position);

	static int white = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
	static int green = FOREGROUND_GREEN;
	static int red = FOREGROUND_RED;

	// 구분을 위해 설정했던 데이터 초기화.
	for (int y = 0; y < static_cast<int>(grid.size()); ++y)
	{
		for (int x = 0; x < static_cast<int>(grid[0].size()); ++x)
		{
			int& value = grid[y][x];
			if (value == 5)
			{
				value = 0;
			}
		}
	}

	// 경로를 제외한 맵 출력.
	for (int y = 0; y < static_cast<int>(grid.size()); ++y)
	{
		for (int x = 0; x < static_cast<int>(grid[0].size()); ++x)
		{
			int& value = grid[y][x];

			// 시작 위치 = 2.
			if (value == 2)
			{
				SetConsoleTextAttribute(handle, red);
				std::cout << "S ";
				continue;
			}

			// 목표 위치 = 3
			if (value == 3)
			{
				SetConsoleTextAttribute(handle, red);
				std::cout << "G ";
				continue;
			}

			// 장애물 = 1
			if (value == 1)
			{
				SetConsoleTextAttribute(handle, white);
				std::cout << "1 ";
				continue;
			}

			// 빈 공간.
			if (value == 0)
			{
				SetConsoleTextAttribute(handle, white);
				std::cout << "0 ";
			}
		}

		// 개행(다음 줄로 이동).
		std::cout << "\n";
	}

	// 경로 출력.
	for (Node* const node : path)
	{
		// 경로는 '*' 표시.
		position.X = static_cast<short>(node->vector2.x * 2);
		position.Y = static_cast<short>(node->vector2.y);

		SetConsoleCursorPosition(handle, position);
		SetConsoleTextAttribute(handle, green);

		std::cout << "* ";
		// 잠시 대기 (단위: 밀리초).
		DWORD delay = static_cast<DWORD>(0.05f * 1000);
		Sleep(delay);
	}

	// 위치 초기화.
	position.X = 0;
	position.Y = 0;
}

std::vector<Vector2> AStar::ConstructPath(Node* goalNode)
{
	std::vector<Vector2> path;
    Node* currentNode = goalNode;
    while (currentNode) {
        path.push_back(currentNode->vector2); // 좌표값 복사
        currentNode = currentNode->parentNode;
    }
    std::reverse(path.begin(), path.end());
    return path;
}

int AStar::CalculateHeuristic(Node* currentNode, Node* goalNode)
{
	// 추정 비용 계산 함수.
	// 고민해볼 계산 방식.
	// 현재노드에서 목표 노드까지의 비용 계산.
	// 단순 거리를 휴리스틱 비용으로 계산.
	// 현재 노드와 목표 노드의 x, y 좌표 차이의 절대값을 합산.
	int dx = std::abs(currentNode->vector2.x - goalNode->vector2.x);
	int dy = std::abs(currentNode->vector2.y - goalNode->vector2.y);

	// 이동 비용 단위가 10이므로 거리 합 * 10.
	return (dx + dy) * 10;
}

bool AStar::IsInRange(
	int x, int y, const std::vector<std::vector<int>>& grid)
{
	// 예외처리.
	if (grid.empty() || grid[0].empty())
	{
		return false;
	}

	// x, y 범위가 벗어났는지 확인.
	if (x < 0 || x >= static_cast<int>(grid[0].size())
		|| y < 0 || y >= static_cast<int>(grid.size()))
	{
		return false;
	}

	// 벗어나지 않았으면 true 반환.
	return true;
}

bool AStar::HasVisited(int x, int y, int gCost)
{
	// 열린 리스트에 이미 같은 위치가 있고,
	// 기존 위치가 비용이 더 낮으면 방문했다고 판단 (중복 방지).
	for (Node* const node : openList)
	{
		if (node->vector2.x == x && node->vector2.y == y
			&& gCost >= node->gCost)
		{
			return true;
		}
	}

	// 닫힌 리스트에 이미 갖은 위치가 있고, 
	// 비용이 더 낮으면 방문했다고 판단.
	for (Node* const node : closedList)
	{
		if (node->vector2.x == x && node->vector2.y == y
			/*&& gCost >= node->gCost*/)
		{
			return true;
		}
	}

	return false;
}

bool AStar::IsDestination(const Node* const node)
{
	// 두 노드가 같은지 비교.
	return *node == *goalNode;
}

void AStar::DisplayGrid(std::vector<std::vector<int>>& grid)
{
	// 좌표 변수.
	static COORD position = { 0, 0 };
	static HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleCursorPosition(handle, position);

	// 색상 값.
	static int white = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
	static int green = FOREGROUND_GREEN;
	static int red = FOREGROUND_RED;

	// 맵 출력.
	for (int y = 0; y < static_cast<int>(grid.size()); ++y)
	{
		for (int x = 0; x < static_cast<int>(grid[0].size()); ++x)
		{
			// 시작 위치 = 2.
			if (grid[y][x] == 2)
			{
				SetConsoleTextAttribute(handle, red);
				std::cout << "S ";
				continue;
			}

			// 목표 위치 = 3.
			if (grid[y][x] == 3)
			{
				SetConsoleTextAttribute(handle, red);
				std::cout << "G ";
				continue;
			}

			// 장애물 = 1.
			if (grid[y][x] == 1)
			{
				SetConsoleTextAttribute(handle, white);
				std::cout << "1 ";
				continue;
			}

			// 경로 = 5.
			if (grid[y][x] == 5)
			{
				SetConsoleTextAttribute(handle, green);
				std::cout << "+ ";
				continue;
			}

			// 빈 공간 = 0.
			if (grid[y][x] == 0)
			{
				SetConsoleTextAttribute(handle, white);
				std::cout << "0 ";
			}
		}

		// 개행 (다음 줄로 이동).
		std::cout << "\n";
	}

	// 위치 초기화.
	position.X = 0;
	position.Y = 0;
}
Node* AStar::MakeNewNode(int x, int y) {
	Node* newNode = new Node(x, y);

	// 부모 노드나 이동비용은 0이나 nullptr로 초기화.
	newNode->parentNode = nullptr;
	newNode->gCost = 0;
	newNode->hCost = 0;
	newNode->fCost = 0;

	// 생성된 모든 노드를 AllNode에 담기.
	allNodes.push_back(newNode);

	return newNode;
}