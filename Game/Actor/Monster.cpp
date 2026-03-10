#include "Monster.h"
#include "Util/Util.h"
#include <algorithm>
#include <queue>
#include <iostream>
#include <Actor/Player.h>
Monster::Monster(const Vector2& Position)
	: super("M", Position, Color::Red)
{
	sortingOrder = 11;
    
}

Monster::~Monster()
{
    
}

// Tick만 수정하면 됨.(BFS랑 에이스타)

// 기존BFS추적과 에이스타추적의 시각화 비교목적을 위한 버전.(BFS추적)
void Monster::Tick(float deltaTime)
{
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
        FindPath(playerPos);
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

        if (gameLevel->player != nullptr)
        { 
            chasePlayerMoveCount++;
        }
    }
   
}

//밑에꺼 에이스타 시각화 완료된 버전
//void Monster::Tick(float deltaTime)
//{
//    if (DestroyRequested()) return;
//
//    GameLevel* gameLevel = static_cast<GameLevel*>(GetOwner());
//    if (!gameLevel || !gameLevel->GetAStar()) return;
//
//    AStar* astar = gameLevel->GetAStar();
//
//    // 플레이어 생존 확인.
//    bool canChase = (gameLevel->player && !gameLevel->player->DestroyRequested());
//
//    // 디버그 시각화 (추격 가능할 때만).
//    if (astar->isVisualizing && canChase)
//    {
//        UpdatePath(gameLevel->player->GetPosition());
//        astar->isVisualizing = false;
//        moveTimer.Reset();
//        return;
//    }
//
//    moveTimer.Tick(deltaTime);
//    if (!moveTimer.IsTimeOut()) return;
//
//    // 목적지 결정 (한칸 이동마다).
//    Vector2 currentPos = GetPosition();
//    Vector2 nextPos = currentPos; // 일단 현재 위치로 초기화.
//
//    if (canChase)
//    {
//        state = MonsterState::Chase;
//        UpdatePath(gameLevel->player->GetPosition()); // 경로 갱신.
//
//        // UpdatePath에서 첫 칸을 지웠으므로 path[0]이 다음 목적지.
//        if (!path.empty())
//        {
//            nextPos = path[0];
//        }
//        else
//        {
//            canChase = false; // 갈 수 있는 경로가 없다면 순찰로 넘김.
//        }
//    }
//
//    if (!canChase)
//    {
//        state = MonsterState::Patrol;
//        path.clear();
//        PatrolMove(*gameLevel); // 방향 결정.
//        nextPos = currentPos + direction;
//    }
//
//    // 계산된 nextPos가 유효한 범위 안이고, 벽(1)이 아닐 때만 이동.
//    if (nextPos.y >= 0 && nextPos.y < (int)gameLevel->canMoveMap.size() &&
//        nextPos.x >= 0 && nextPos.x < (int)gameLevel->canMoveMap[0].size())
//    {
//        if (gameLevel->canMoveMap[(int)nextPos.y][(int)nextPos.x] != 1)
//        {
//            SetPosition(nextPos);
//        }
//        else
//        {
//            // 만약 계산된 위치가 벽이라면, 이동하지 않고 경로 비우기.
//            path.clear();
//        }
//    }
//
//    moveTimer.Reset();
//    if (gameLevel->player != nullptr)
//    {
//        chasePlayerMoveCount++;
//    }
//}

// 비교를 위한 순찰 제거 에이스타 추적버전Tick
//void Monster::Tick(float deltaTime)
//{
//    if (DestroyRequested()) return;
//
//    moveTimer.Tick(deltaTime);
//
//    // 타이머가 도달했을 경우에만 경로계산 및 움직임.
//    if (moveTimer.IsTimeOut())
//    {
//        GameLevel* gameLevel = static_cast<GameLevel*>(GetOwner());
//        if (!gameLevel) return;
//
//        state = MonsterState::Chase;
//        Vector2 playerPos = gameLevel->GetPlayerPosition();
//        targetPosition = playerPos;
//
//        // 길찾기 수행 (0.5초마다 현재 상황에 맞는 최적의 경로 갱신)
//        UpdatePath(targetPosition);
//
//        // 2. 이동 실행 (0.5초에 딱 한 번만 한 칸 움직임)
//        if (!path.empty() && pathIndex < (int)path.size())
//        {
//            Movement();
//        }
//        else
//        {
//            // 경로가 없으면 순찰 로직 실행.
//            PatrolMove(*gameLevel);
//            SetPosition(GetPosition() + direction);
//        }
//
//        moveTimer.Reset();
//    }
//}

void Monster::BeginPlay()
{
    GameLevel* gameLevel = static_cast<GameLevel*>(GetOwner());
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

// 밑에꺼 에이스타 추적버전
//void Monster::Movement()
//{
//    // 경로 끝에 도달했으면 종료.
//    if (path.empty() || pathIndex >= (int)path.size()) return;
//
//    Vector2 nextPosition = path[pathIndex];
//    GameLevel* gameLevel = static_cast<GameLevel*>(GetOwner());
//
//    // 다음 칸이 여전히 이동 가능한 상태인지 최종 체크.
//    if (gameLevel && gameLevel->CanMonsterOrBubbleMove(nextPosition))
//    {
//        SetPosition(nextPosition);
//        pathIndex++; // 성공적으로 이동했으므로 다음 목표 지점으로 인덱스 증가.
//    }
//    else
//    {
//        // 길이 막혔다면 다음 타이머를 기다리지 않고 즉시 재탐색 유도.
//        UpdatePath(targetPosition);
//    }
//}

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
    // 필요한 객체 가쳐오기.
    GameLevel* gameLevel = static_cast<GameLevel*>(GetOwner());
    if (!gameLevel) return;

    // GameLevel에 하나만 존재하는 공유 에이스타 객체 가져오기.
    AStar* sharedAstar = gameLevel->GetAStar();
    if (!sharedAstar) return;

    // 2. 맵의 장애물 정보를 최신화.(0으로 초기화 후 못가는 곳 1로 채우는 함수 호출)
    gameLevel->UpdateCanMoveMap();

    // 에이스타 실행
    std::vector<Vector2> resultPath = sharedAstar->FindPath(GetPosition(), dest, gameLevel->canMoveMap, gameLevel->dangerMap);

    // 4. 결과를 몬스터 자신의 경로 변수에 담기.
    this->path = resultPath;

    // 
    // 에이스타 결과의 첫 번째 칸은 항상 몬스터의 현재 위치
    // Movement()가 다음 칸부터 자연스럽게 이동하게 하기 위해 첫 칸을 제거.
    if (!this->path.empty())
    {
        this->path.erase(this->path.begin());
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