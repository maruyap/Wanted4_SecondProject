#pragma once
#include "Actor/Actor.h"
#include "Util/Timer.h"
#include "Math/Vector2.h"

using namespace Wanted;

class WaveEffect : public Actor 
{
    RTTI_DECLARATIONS(WaveEffect, Actor)

public:
    // 생성자에서 이펙트 유지시간 설정.
    WaveEffect(const Vector2& Position)
        : Actor("H", Position, Wanted::Color::Blue), lifeTimer(0.6f)
    {
        sortingOrder = 7;
    }
    
    virtual void Tick(float deltaTime) override 
    {
        Actor::Tick(deltaTime);
        lifeTimer.Tick(deltaTime);
        if (lifeTimer.IsTimeOut()) 
        {
            this->Destroy();
        }
    }
  
private:
    Timer lifeTimer;
};