#include "Boss.h"
#include "Render/Renderer.h"
#include "Level/GameLevel.h"
#include "Actor/Wall.h"
#include "Actor/LightBox.h"
#include "Actor/HeavyBox.h"
#include "Actor/Bubble.h"
#include "Actor/WaveEffect.h"
#include "Actor/Player.h"
#include "Util/Util.h"

Boss::Boss(const Vector2& Position)
	: super("@", Position, Color::Red)

{
    ResetAttackTimer();
    sortingOrder = 13;
}

bool Boss::IsHit(const Vector2& wavePos) const 
{
    Vector2 center = GetPosition();
    // 중심점에서 x, y 거리가 각각 1 이하이면 보스의 3x3 영역 내부임.
    return (abs(center.x - wavePos.x) <= 1 && abs(center.y - wavePos.y) <= 1);
}

void Boss::OnHit()
{
    if (!isInvincible)
    {
        --hp;
        if (hp <= 0)
            this->Destroy();

        isInvincible = true;
    }
}

void Boss::Tick(float deltaTime)
{
    super::Tick(deltaTime);

    // 대기 시간 처리.
    if (waitTime > 0.0f)
    {
        waitTime -= deltaTime; // 남은 대기 시간 차감.
        Render();              // 멈춰있는 동안에도 그리기는 해야 함.
        return;                // 여기서 리턴하니까 아래 이동/공격 로직이 멈춤.
    }

    if (isInvincible) 
    {       // 한 물풍선으로 보스 다중 타격을 방지하기위한 무적 타이머.
        invincibleTimer.Tick(deltaTime);

        if (invincibleTimer.IsTimeOut())
        {
            isInvincible = false;
            invincibleTimer.Reset();
        }
    }

    // 공격 타이머 먼저 체크 (선딜레이 트리거).
    attackTimer.Tick(deltaTime);
    if (attackTimer.IsTimeOut())
    {
        // 선딜레이 발생 1.0초 멈추게 설정.
        waitTime = 1.0f;

        // 실제 폭발 실행.
        ExplodeAround();

        // 후딜레이 발생 0.7초를 더 추가 (총 1.0초 멈췄다 터지고, 또 0.7초 멈춤).
  
        waitTime += 0.7f;

        ResetAttackTimer(); // 다음 랜덤 시간 설정 및 Reset().
        return;
    }

    // 이동 타이머 처리 (평소 상태).
    moveTimer.Tick(deltaTime);
    if (moveTimer.IsTimeOut())
    {
        BossMovement();
        moveTimer.Reset();
    }

    Render();
}


std::vector<Vector2> Boss::GetOccupiedPositions()
{
    std::vector<Vector2> positions;
    Vector2 center = GetPosition();

    // 중심을 기준으로 -1 ~ +1 범위 (총 3x3).
    for (int y = -1; y <= 1; ++y)
    {
        for (int x = -1; x <= 1; ++x)
        {
            positions.push_back({ center.x + x, center.y + y });
        }
    }
    return positions;
}
void Boss::ResetAttackTimer()
{
    float nextTime = Util::RandomRange(3.0f, 6.0f);
    attackTimer.SetTargetTime(nextTime);
    attackTimer.Reset();
}
void Boss::Render() 
{
   
    auto positions = GetOccupiedPositions();
    for (const auto& pos : positions)
    {
        // 보스의 몸체를 구성하는 문자들을 출력.
        Renderer::Get().Submit("@", pos, color, sortingOrder);
    }
}
void Boss::PatrolMove(GameLevel& level)
{
    // 현재 방향으로 3x3 덩어리가 갈 수 있는지 체크.
    Vector2 nextPos = GetPosition() + direction;

    // 앞이 3x3 공간만큼 충분히 뚫려있다면 그대로 유지.
    if (level.CanBossMove(nextPos)) // 보스 전용 체크 함수 (3x3 검사).
    {
        return;
    }

    // 앞이 막혔을 때만 새로운 방향 탐색.
    static const Vector2 dirs[] = { {0,1}, {0,-1}, {1,0}, {-1,0} };
    int startIdx = std::rand() % 4;

    for (int i = 0; i < 4; i++)
    {
        int targetIdx = (startIdx + i) % 4;
        Vector2 checkPos = GetPosition() + dirs[targetIdx];

        // 바꿀 방향도 3x3 공간이 나오는지 확인.
        if (level.CanBossMove(checkPos))
        {
            direction = dirs[targetIdx];
            return;
        }
    }

    // 모든 방향(3x3 공간 확보 불가능)이 막혔다면 멈춤.
    direction = { 0, 0 };
}
void Boss::BossMovement()
{
    GameLevel* level = static_cast<GameLevel*>(GetOwner());
    if (!level) return;

    // 방향 결정.
    PatrolMove(*level);

    // 이동 실행.
    Vector2 nextPosition = GetPosition() + direction;
    if (direction != Vector2(0, 0) && level->CanBossMove(nextPosition))
    {
        SetPosition(nextPosition);
    }
}

void Boss::ExplodeAround()
{
    const std::vector<Actor*>& allActors = this->GetOwner()->GetActors();
    Vector2 center = GetPosition(); // 오직 보스의 현재 위치만 사용.

    // 중심에서 3칸씩.
    int rangeX = 3;
    int rangeY = 3;

    // 이펙트 생성.
    for (int y = -rangeY; y <= rangeY; ++y)
    {
        for (int x = -rangeX; x <= rangeX; ++x)
        {
            Vector2 effectPosition = { center.x + x, center.y + y };
            static_cast<GameLevel*>(GetOwner())->AddNewActor(new WaveEffect(effectPosition));
        }
    }

    // 실제 타격 판정.
    for (Actor* actor : allActors)
    {
        if (!actor || actor == this) continue;
        Vector2 actorPos = actor->GetPosition();

        // 오직 center와의 거리만 체크.
        if (abs(center.x - actorPos.x) <= rangeX && abs(center.y - actorPos.y) <= rangeY)
        {
            if (actor->IsTypeOf<Bubble>())
            {
                Bubble* bubble = static_cast<Bubble*>(actor);
                bubble->isTimeOut = true; // 범위 내 물풍선만 즉시 폭발.
            }
            else if (actor->IsTypeOf<HeavyBox>() ||
                actor->IsTypeOf<LightBox>())
            {
                int chance = Util::Random(0, 1);
                if (chance == 1)
                {
                    GameLevel* gameLevel = static_cast<GameLevel*>(this->GetOwner());
                    int itemType = Util::Random(1, 3);
                    gameLevel->SpawnItem(actor->GetPosition(), itemType);
                }
                actor->Destroy(); // 범위 내 박스만 제거.
            }
            else if (actor->IsTypeOf<Player>())
                actor->Destroy();
        }
    }
}