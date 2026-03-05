#include "Monster.h"
#include "Util/Util.h"
#include <algorithm>
#include <queue>
#include <iostream>

Monster::Monster(const Vector2& Position)
	: super("M", Position, Color::Red)
{
	sortingOrder = 11;
    astar = new AStar();
}

Monster::~Monster()
{
   /* if (astar)
    {
        delete astar;
        astar = nullptr;
    }*/
}

// 기존BFS추적과 에이스타추적의 시각화 비교목적을 위한 버전.
//void Monster::Tick(float deltaTime)
//{
//    moveTimer.Tick(deltaTime);
//
//    if (moveTimer.IsTimeOut())
//    {
//        GameLevel* gameLevel = static_cast<GameLevel*>(GetOwner());
//        if (!gameLevel) return;
//
//        Vector2 myPos = GetPosition();
//        Vector2 playerPos = gameLevel->GetPlayerPosition();
//
//        // 거리와 상관없이 무조건 추적 상태로 설정.
//        state = MonsterState::Chase;
//
//        // 항상 플레이어의 위치를 향해 길찾기 수행.
//        // 매 타임아웃마다 경로를 새로 갱신하여 물풍선 변화에 즉각 반응하게 함.
//        FindPath(playerPos);
//        targetPosition = playerPos;
//
//        if (!path.empty())
//        {
//            Movement(); // 경로가 있으면 추적 이동.
//        }
//        else
//        {
//            // 경로가 없을 때(길이 막혔을 때)만 제자리 대기 혹은 순찰.
//            PatrolMove(*gameLevel);
//            SetPosition(myPos + direction);
//        }
//
//        moveTimer.Reset();
//    }
//}

// 비교를 위한 순찰 제거 에이스타 추적버전Tick
void Monster::Tick(float deltaTime)
{
    if (DestroyRequested()) return;

    moveTimer.Tick(deltaTime);

    if (moveTimer.IsTimeOut())
    {
        GameLevel* gameLevel = static_cast<GameLevel*>(GetOwner());
        if (!gameLevel) return;

        Vector2 myPos = GetPosition();
        Vector2 playerPos = gameLevel->GetPlayerPosition();

        // 거리와 상관없이 무조건 추적 상태로 설정.
        state = MonsterState::Chase;

        // 항상 플레이어의 위치를 향해 길찾기 수행.
        // 매 타임아웃마다 경로를 새로 갱신하여 물풍선 변화에 즉각 반응하게 함.
        UpdatePath(playerPos);
        targetPosition = playerPos;

        if (!path.empty())
        {
            Movement(); // 경로가 있으면 추적 이동.
        }
        else
        {
            // 경로가 없을 때(길이 막혔을 때)만 제자리 대기 혹은 순찰.
            PatrolMove(*gameLevel);
            SetPosition(myPos + direction);
        }

        moveTimer.Reset();
    }
}



//밑에가 기존 거리기준 추적 순회 변경로직이 들어간 버전
//void Monster::Tick(float deltaTime)
//{
//    moveTimer.Tick(deltaTime);
//
//    if (moveTimer.IsTimeOut())
//    {
//        GameLevel* gameLevel = static_cast<GameLevel*>(GetOwner());
//        if (!gameLevel) return; // nullptr 체크를 가장 위로.
//
//        Vector2 myPos = GetPosition();
//        Vector2 playerPos = gameLevel->GetPlayerPosition();
//        int distance = myPos.DistanceTo(playerPos);
//
//        // 상태 결정 및 관리.
//        if (distance >= 7) 
//        {
//            if (state != MonsterState::Patrol)
//            { // 상태가 바뀔 때만 초기화.
//                state = MonsterState::Patrol;
//                path.clear();
//            }
//        }
//        else
//        {
//            state = MonsterState::Chase;
//        }
//
//        // 상태별 로직 실행.
//        if (state == MonsterState::Patrol)
//        {
//            PatrolMove(*gameLevel);
//            SetPosition(myPos + direction);
//        }
//        else if (state == MonsterState::Chase)
//        {
//            if (targetPosition != playerPos || path.empty())
//            {
//                FindPath(playerPos);
//                targetPosition = playerPos;
//
//                if (path.empty())
//                {
//                    PatrolMove(*gameLevel);
//                    SetPosition(myPos + direction);
//                }
//            }
//            Movement();
//        }
//        moveTimer.Reset();
//    }
//}

// 밑에꺼 순찰모드 뺀 BFS추적버전
//void Monster::Movement()
//{
//    // 이동 가능 여부만 판단.
//    if (path.empty() || pathIndex >= (int)path.size()) return;
//
//    Vector2 nextPosition = path[pathIndex];
//    GameLevel* gameLevel = static_cast<GameLevel*>(GetOwner());
//
//    if (gameLevel && gameLevel->CanMonsterOrBubbleMove(nextPosition)) 
//    {
//        SetPosition(nextPosition);
//        pathIndex++;
//    }
//    else 
//    {
//        // 길이 막혔을 때만 재탐색.
//        FindPath(targetPosition);
//    }
//
//}


// 밑에꺼 에이스타 추적버전
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
        UpdatePath(targetPosition);
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


void Monster::UpdatePath(Vector2 dest)
{
    path.clear();

    GameLevel* gameLevel = static_cast<GameLevel*>(GetOwner());
    if (gameLevel == nullptr || DestroyRequested()) return;

    // 원본 맵이 비어있으면 초기화.
    if (gameLevel->canMoveMap.empty()) gameLevel->InitCanMoveMap();

    // 원본을 직접 쓰지 말고 복사본 사용.
    std::vector<std::vector<int>> localMap = gameLevel->canMoveMap;

    // 현재 프레임의 동적인 장애물(다른 액터들) 정보를 복사본에만 기록.
    const std::vector<Actor*>& actors = gameLevel->GetActors();
    for (Actor* actor : actors)
    {
        // 자기 자신은 장애물에서 제외해야 길을 찾기 시작할 수 있음.
        if (actor == this) continue;

        if (actor->IsBlocking())
        {
            Vector2 actorPos = actor->GetPosition();
            // 인덱스 범위 체크 (안전장치).
            if (actorPos.y >= 0 && actorPos.y < localMap.size() &&
                actorPos.x >= 0 && actorPos.x < localMap[0].size())
            {
                localMap[actorPos.y][actorPos.x] = 1;
            }
        }
    }

    Node* startNode = new Node(position.x, position.y);
    Node* goalNode = new Node(dest.x, dest.y);

    // 4. 복사본을 전달해서 탐색.
    std::vector<Node*> resultPath = astar->FindPath(startNode, goalNode, localMap);

    path.clear();
    for (Node* node : resultPath)
    {
        path.emplace_back(node->vector2);
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