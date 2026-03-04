#include "Monster.h"
#include "Util/Util.h"
#include <algorithm>
#include <queue>
#include <iostream>

Monster::Monster(const Vector2& Position)
	: super("M", Position, Color::Red)
{
	sortingOrder = 11;
}

void Monster::Tick(float deltaTime)
{
    moveTimer.Tick(deltaTime);

    if (moveTimer.IsTimeOut())
    {
        GameLevel* gameLevel = static_cast<GameLevel*>(GetOwner());
        if (!gameLevel) return; // nullptr 체크를 가장 위로.

        Vector2 myPos = GetPosition();
        Vector2 playerPos = gameLevel->GetPlayerPosition();
        int distance = myPos.DistanceTo(playerPos);

        // 상태 결정 및 관리.
        if (distance >= 7) 
        {
            if (state != MonsterState::Patrol)
            { // 상태가 바뀔 때만 초기화.
                state = MonsterState::Patrol;
                path.clear();
            }
        }
        else
        {
            state = MonsterState::Chase;
        }

        // 상태별 로직 실행.
        if (state == MonsterState::Patrol)
        {
            PatrolMove(*gameLevel);
            SetPosition(myPos + direction);
        }
        else if (state == MonsterState::Chase)
        {
            if (targetPosition != playerPos || path.empty())
            {
                FindPath(playerPos);
                targetPosition = playerPos;

                if (path.empty())
                {
                    PatrolMove(*gameLevel);
                    SetPosition(myPos + direction);
                }
            }
            Movement();
        }
        moveTimer.Reset();
    }
}
void Monster::Movement()
{
    // 이동 가능 여부만 판단.
    if (path.empty() || pathIndex >= (int)path.size()) return;

    Vector2 nextPosition = path[pathIndex];
    GameLevel* gameLevel = static_cast<GameLevel*>(GetOwner());

    if (gameLevel && gameLevel->CanMonsterOrBubbleMove(nextPosition)) 
    {
        SetPosition(nextPosition);
        pathIndex++;
    }
    else 
    {
        // 길이 막혔을 때만 재탐색.
        FindPath(targetPosition);
    }

}

void Monster::FindPath(Vector2 dest)
{
    path.clear();
    pathIndex = 0;
    Vector2 start = GetPosition();
    Vector2 parent[20][20];
    bool visited[20][20] = { false };

    std::fill(&parent[0][0], &parent[0][0] + sizeof(parent) / sizeof(Vector2), Vector2(-1, -1));

    std::queue<Vector2> q;
    q.push(start);
    visited[(int)start.y][(int)start.x] = true;

    GameLevel* level = static_cast<GameLevel*>(GetOwner());
    bool found = false;

    while (!q.empty()) 
    {
        Vector2 curr = q.front();
        q.pop();

        if (curr.x == dest.x && curr.y == dest.y) 
        {
            found = true;
            break;
        }

        static const Vector2 dirs[] = { {0,1}, {0,-1}, {1,0}, {-1,0} }; // static으로 한 번만 생성.
        for (const auto& d : dirs) 
        {
            Vector2 next = { curr.x + d.x, curr.y + d.y };

            if (level->CanMonsterOrBubbleMove(next) && !visited[(int)next.y][(int)next.x])
            {
                visited[(int)next.y][(int)next.x] = true;
                parent[(int)next.y][(int)next.x] = curr;
                q.push(next);
            }
        }
    }

    if (found) 
    {
        Vector2 curr = dest;
        while (curr.x != -1)
        {
            path.push_back(curr);
            curr = parent[(int)curr.y][(int)curr.x];
        }
        std::reverse(path.begin(), path.end());
        if (!path.empty()) path.erase(path.begin());
    }
}

void Monster::PatrolMove(GameLevel& level)
{
    // 현재 방향으로 계속 갈 수 있는지 먼저 체크.
    Vector2 nextPos = GetPosition() + direction;

    // 앞이 뚫려있다면 아무것도 안 하고 그대로 유지.
    if (level.CanMonsterOrBubbleMove(nextPos))
    {
        return;
    }

    // 앞이 막혔을 때만 새로운 방향 탐색.
    static const Vector2 dirs[] = { {0,1}, {0,-1}, {1,0}, {-1,0} };

    // 4방향을 랜덤한 순서로 검사 (시작점만 랜덤으로 정함).
    int startIdx = std::rand() % 4;

    for (int i = 0; i < 4; i++)
    {
        int targetIdx = (startIdx + i) % 4;
        Vector2 checkPos = GetPosition() + dirs[targetIdx];

        // 갈 수 있는 방향을 찾으면 즉시 확정하고 종료.
        if (level.CanMonsterOrBubbleMove(checkPos))
        {
            direction = dirs[targetIdx];
            return;
        }
    }

    // 모든 방향이 막혔다면 멈춤.
    direction = { 0, 0 };
}