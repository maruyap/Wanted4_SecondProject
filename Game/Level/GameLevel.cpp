#include "GameLevel.h"
#include "Actor/Player.h"
#include "Actor/Ground.h"
#include "Actor/Wall.h"
#include "Actor/HeavyBox.h"
#include "Actor/LightBox.h"
#include "Actor/Item.h"
#include "Actor/Monster.h"
#include "Actor/Bubble.h"
#include "Core/Input.h"
#include "Game/Game.h"
#include "Actor/Actor.h"
#include "Actor/WaveEffect.h"
#include "Render/Renderer.h"
#include "Actor/Boss.h"
#include "Actor/Drone.h"
#include <iostream>

GameLevel* GameLevel::gameLevelInstance = nullptr;

GameLevel::GameLevel()
{
	gameLevelInstance = this;
	Util::SetRandomSeed();

	astar = new AStar();

	LoadMap("Octo1.txt");
	// Octo1.txt or Patrit14.txt or Test1.txt.
}

GameLevel::~GameLevel()
{
	if (astar) {
		// [핵심] 레벨이 꺼지기 전에 마지막으로 남은 노드들 청소
		astar->ClearLists();
		delete astar;
		astar = nullptr;
	}
}

// H == HeavyBox(밀수없으나 파괴는 가능한 박스).
// L == LightBox(밀수도 있고 파괴도 가능한 박스).
// # == Wall(밀수없고 파괴도 불가능한 벽).
// o == Ground(지나다닐 수 있는 일반 땅).
// 가로 19 세로 17.
// 원래 크아가 가로15 세로13 인데 가장자리 이중벽으로 막아서 19 17.
// 밑에 맵파일 읽어들여서 액터 세팅해주는 함수.

void GameLevel::LoadMap(const char* filename)
{
	InitCanMoveMap();

	// 파일 로드.
	// 최종 파일 경로 만들기. ("../Assets/filename")
	char path[2048] = {};
	sprintf_s(path, 2048, "../Assets/%s", filename);

	// 파일 열기.
	FILE* file = nullptr;
	fopen_s(&file, path, "rt");

	// 예외 처리.
	if (!file)
	{
		// 표준 오류 콘솔 활용.
		std::cerr << "Failed to open map file.\n";

		// 디버그 모드에서 중단점으로 중단해주는 기능.
		__debugbreak();
	}

	// 맵 읽기.
	// 맵 크기 파악: File Position 포인터를 파일의 끝으로 이동.
	fseek(file, 0, SEEK_END);

	// 이 위치 읽기.
	size_t fileSize = ftell(file);

	// File Position 처음으로 되돌리기.
	rewind(file);

	// 파일에서 데이터를 읽어올 버퍼 생성.
	char* data = new char[fileSize + 1];

	// 데이터 읽기.
	size_t readSize = fread(data, sizeof(char), fileSize, file);

	// 읽어온 문자열을 분석(파싱-Parsing)해서 출력.
	// 인덱스를 사용해 한문자씩 읽기.
	int index = 0;

	// 객체를 생성할 위치 값.
	Wanted::Vector2 position;

	while (true)
	{
		// 종료 조건.
		if (index >= fileSize)
		{
			break;
		}

		// 캐릭터 읽기.
		char mapCharacter = data[index];
		++index;

		// 개행 문자 처리.
		if (mapCharacter == '\n')
		{
			//std::cout << "\n";
			// y좌표는 하나 늘리고, x 좌표 초기화.
			++position.y;
			position.x = 0;
			continue;
		}

	
		// 한문자씩 처리.
		switch (mapCharacter)
		{
		case '#':
			//std::cout << "#" Wall(밀 수도 파괴할 수도 없는 벽)
			AddNewActor(new Wall(position));
			break;

		case 'o':
			//std::cout << " " Ground(움직일 수 있는 땅)
			AddNewActor(new Ground(position));
			break;

		case 'P':
			//std::cout << "P" Player(움직이고 물풍선을 놓는 플레이어)
			// 플레이어도 이동 가능함.
			// 플레이어 밑에 땅이 있어야 함.
			AddNewActor(player = new Player(position));
			playerStartPosition = position;
			AddNewActor(new Ground(position));
			break;

		case 'L':
			//std::cout << "=" LightBox(밀 수도 있고 파괴도 가능한 박스)
			// 박스는 이동 가능함.
			// 박스가 옮겨졌을 때 그 밑에 땅이 있어야 함.
			AddNewActor(new LightBox(position));
			AddNewActor(new Ground(position));
			break;

		case 'H':
			//std::cout << "X" HeavyBox(밀 수 없으나 파괴는 가능한 박스)
			AddNewActor(new HeavyBox(position));
			AddNewActor(new Ground(position));
			break;

		case 'M':
			// 플레이어가 일정 거리에 들어오면 추적하고 아닐경우 순찰하는 몬스터.
			AddNewActor(new Monster(position));
			AddNewActor(new Ground(position));
			break;

		case '*':
			// 충돌범위 9칸을 가지고 체력이 많은 보스 몬스터.
			AddNewActor(new Boss(position));
			AddNewActor(new Ground(position));
			break;

		case 'F':
			// 물풍선 폭발 범위를 상하좌우 한칸씩 늘려주는 아이템.
			AddNewActor(new Item(position, 1));
			AddNewActor(new Ground(position));
			break;

		case 'B':
			// BubbleLimit을 증가시켜 주는 아이템.
			AddNewActor(new Item(position, 2));
			AddNewActor(new Ground(position));
			break;

		case 'K':
			// KickShoes 아이템 플레이어가 먹고 나면 물풍선을 차서 이동 시킴.
			AddNewActor(new Item(position, 3));
			AddNewActor(new Ground(position));
			break;

		case 'D':
			AddNewActor(new Drone(position));
			AddNewActor(new Ground(position));
			break;
		}

		// 맵파일로 추가되는 액터 외에 박스 파괴시 드랍되는 아이템이나 몬스터 액터도 있음
		// Monster(M)    Balloon(B) Flask(F)   Skate(S) KickShoes(K)
		// x 좌표 증가 처리.
		++position.x;
	}

	// 사용한 버퍼 해제.
	delete[] data;

	// 파일이 정상적으로 열렸으면 닫기.
	fclose(file);
}


void GameLevel::CollisionPlayerAndOther()
{
	Vector2 playerPosition = player->GetPosition();
	for (Actor* actor : actors)
	{
		if (actor->IsTypeOf<Boss>())
		{
			Boss* boss = static_cast<Boss*>(actor);
			// 플레이어의 현재 위치가 보스의 3x3 영역 안이라면 죽음.
			if (boss->IsHit(playerPosition))
			{
				player->Destroy();
			}
		}

		if (actor->IsTypeOf<Item>())
		{
			if (actor->GetPosition() == playerPosition)
			{
				Item* item = static_cast<Item*>(actor);

				switch (static_cast<int>(item->itemType))
				{
				case 1: // "F" (Flask) - 물줄기 길이 증가.
				{
					if (player->bubbleScope < 4)
						player->bubbleScope++;
				}
					break;

				case 2: // "B" (Bubble) - 설치 가능 개수 증가.
				{
					if (player->bubbleLimit < 8)
						player->bubbleLimit++;
				}
					break;

				case 3: // "K" (KickShoes) - 물풍선 차기 가능.
					player->canKick = true;
					break;
				}
				item->Destroy();
			}
		}
		
		if (actor->IsTypeOf<Monster>())
		{
			if (actor->GetPosition() == playerPosition)
			{
				player->Destroy();
			}
		}
	}
	
	
}
Vector2 GameLevel::GetPlayerPosition()
{
	for (Actor* actor : actors)
	{
		if (actor->IsTypeOf<Player>())
		{
			return actor->GetPosition();
		}
	}
	return Vector2(0, 0);
}


void GameLevel::CollisionWaveAndOther(const Vector2& position)
{
	// 중심점에 이펙트 생성.
	AddNewActor(new WaveEffect(position));

	// 상, 하, 좌, 우 방향 벡터.
	Vector2 dirs[4] = { Vector2(0, -1), Vector2(0, 1), Vector2(-1, 0), Vector2(1, 0) };

	// 각 방향으로 플레이어의 사거리만큼 뻗어나감.
	for (int d = 0; d < 4; ++d)
	{
		for (int step = 1; step <= player->bubbleScope; ++step)
		{
			Vector2 currentPos = position + (dirs[d] * step);
			bool isBlocked = false; // 이번 칸에서 줄기가 막히는지 여부.

			// 루프를 돌며 현재 좌표의 액터들을 검사.
			for (Actor* actor : actors)
			{
				if (actor->DestroyRequested()) continue;

				// 3*3보스 충돌 체크 .
				if (actor->IsTypeOf<Boss>())
				{
					Boss* boss = static_cast<Boss*>(actor);
					if (boss->IsHit(currentPos))
					{
						boss->OnHit();
						//보스는 피격은 되지만 물줄기를 막지는 않음.
					}
					continue;
				}

				// 1*1 객체 처리.
				if (actor->GetPosition() == currentPos)
				{

					if (actor->IsTypeOf<Bubble>())
					{
						static_cast<Bubble*>(actor)->isTimeOut = true;
					}

					else if (actor->IsTypeOf<LightBox>() || actor->IsTypeOf<HeavyBox>() ||
						actor->IsTypeOf<Wall>() || actor->IsTypeOf<Monster>() ||
						actor->IsTypeOf<Player>() || actor->IsTypeOf<Item>())
					{

						if (actor->IsTypeOf<LightBox>() || actor->IsTypeOf<HeavyBox>())
						{
							if (Util::Random(0, 1) == 1)
							{
								SpawnItem(actor->GetPosition(), Util::Random(1, 3));
							}
							actor->Destroy();
						}
						else if (actor->IsTypeOf<Monster>() || actor->IsTypeOf<Player>())
						{
							actor->Destroy();
						}

						// 벽(Wall)이나 박스류는 줄기를막음.
						if (actor->IsTypeOf<Wall>() || actor->IsTypeOf<LightBox>() || actor->IsTypeOf<HeavyBox>())
						{
							isBlocked = true;
						}
					}
				}
			}

			// --- 이펙트 생성 및 중단 처리 ---
			// 만약 벽에 막혔다면, 그 칸에 이펙트를 그릴지 말지 결정.
			// 보통 박스가 터지는 칸까지는 물줄기가 그려지는 게 자연스러움.
			AddNewActor(new WaveEffect(currentPos));

			if (isBlocked) break; // 이번 방향은 여기서 종료.
		}
	}
}

bool GameLevel::CanMove(const Vector2& playerPosition, const Vector2& nextPosition)
{
	Actor* targetObstacle = nullptr;    

	for (Actor* actor : actors)
	{
		if (actor->GetPosition() == nextPosition)
		{
			if (actor->IsTypeOf<Wall>() || actor->IsTypeOf<HeavyBox>()) return false;
			if (actor->IsTypeOf<LightBox>() || actor->IsTypeOf<Bubble>())
			{
				targetObstacle = actor;
			}
		}
	}
	if (targetObstacle)
	{
		if (targetObstacle->IsTypeOf<Bubble>())
		{
			if (!player->canKick) return false;

			Bubble* b = static_cast<Bubble*>(targetObstacle);

			if (!b->isSliding)
			{
				b->moveDir = nextPosition - playerPosition;
				b->isSliding = true;
				b->moveTimer.Reset();
				b->timer.Reset();
			}
			return false;      // 버블을 찬 경우엔 제자리에서 차게되니 false 반환.
		}
		Vector2 direction = nextPosition - playerPosition;
		Vector2 newPosition = targetObstacle->GetPosition() + direction;

		for (Actor* actor : actors)
		{
			if (actor->GetPosition() == newPosition)   // 내가 가려는 위치가 밀 수 있는 박스일 경우 [나][LightBox][newposition]
			{                                          // 이때 newposition이 장애물일경우 false  ->결국 저런상태론 캐릭을 못움직이니 false 반환.
				if (actor->IsTypeOf<Wall>() || actor->IsTypeOf<HeavyBox>() ||
					actor->IsTypeOf<LightBox>() || actor->IsTypeOf<Bubble>())
				{
					return false;
				}
			}
		}
		targetObstacle->SetPosition(newPosition);    // 밀 수 있는경우 Light박스도 이동시키고 캐릭도 이동가능하니 true 반환.
		return true;
	}
	return true;
}


void GameLevel::Bomb(void)
{
	for (Actor* actor : actors)
	{
		if (actor->IsTypeOf<Bubble>())
		{
			Bubble* b = static_cast<Bubble*>(actor);

			if (b->isTimeOut == true)
			{
				// 실제 폭발 충돌 처리 함수 호출.
				CollisionWaveAndOther(b->GetPosition());
				b->Destroy();
			}
		}
	}
}

void GameLevel::Tick(float deltatime)
{
	Level::Tick(deltatime);

	if (Input::Get().GetKeyDown(VK_ESCAPE))
	{
		Game::Get().ToggleMenu();
	}

	if (Input::Get().GetKeyDown('Q'))
	{
		Game::Get().QuitEngine();
	}

	if (Input::Get().GetKeyDown('R'))
	{
		bool PlayerIsAlive = false;
		for (Actor* actor : actors)
		{
			if (actor->IsTypeOf<Player>())
			{
				PlayerIsAlive = true;
			}	
		}

		if (PlayerIsAlive == false && playerLifeCount > 0)
		{
			Player* newPlayer = new Player(playerStartPosition);
			this->player = newPlayer;
			--playerLifeCount;
			AddNewActor(newPlayer);
		}
	}

	if (Input::Get().GetKeyDown('K'))
	{
		EnemyAllKill();
	}

	if (Input::Get().GetKeyDown('N'))
	{
		if (!MonsterCounter())
		{
			ClearActors();
			NextStage();
		}
	}

	if (player == nullptr || player->DestroyRequested())
		return;

	Bomb();

	if (player->wantSpawnBubble == true)
		SpawnBubble();

	CollisionPlayerAndOther();

	DrawUI();
}




void GameLevel::SpawnItem(const Vector2& position, int itemType)
{
	AddNewActor(new Item(position, itemType));
}

void GameLevel::SpawnBubble(void)
{
	player->wantSpawnBubble = false;

	int currentCount = 0;
	for (Actor* actor : actors)
	{
		if (actor->IsTypeOf<Bubble>()) 
		{
			currentCount++;
		}
	}

	if (currentCount < player->bubbleLimit) 
	{
		AddNewActor(new Bubble(player->GetPosition()));
	}
	
}



bool GameLevel::CanMonsterOrBubbleMove(const Vector2& nextPosition)
{
	for (Actor* actor : actors)
	{
		if (actor->GetPosition() == nextPosition)
		{
			if (actor->IsBlocking() == true)
				return false;
		}
	}
	return true;
}

void GameLevel::DrawUI()
{
	static char lifeBuf[64];
	static char bombBuf[64];
	static char scopeBuf[64];
	static char canKickBuf[64];

	Renderer& renderer = Renderer::Get();

	sprintf_s(lifeBuf, "LIFE  : %d", playerLifeCount);
	renderer.Submit(lifeBuf, Vector2(24, 2), Color::Red, 100);

	if (player)
	{
		sprintf_s(bombBuf, "MAX_BOMB  : %d / 8", player->bubbleLimit);
		renderer.Submit(bombBuf, Vector2(20, 4), Color::White, 100);

		sprintf_s(scopeBuf, "MAX_SCOPE : %d / 4", player->bubbleScope);
		renderer.Submit(scopeBuf, Vector2(20, 5), Color::White, 100);

		const char* kickStatus = player->canKick ? "ON" : "OFF";

		sprintf_s(canKickBuf, "CAN_KICK  : %s", kickStatus);

		Color kickColor = player->canKick ? Color::Green : Color::White;
		renderer.Submit(canKickBuf, Vector2(20, 6), kickColor, 100);
	}
}

bool GameLevel::CanBossMove(const Vector2 &centerPosition)
{
	for (int y = -1; y <= 1; ++y)
		for (int x = -1; x <= 1; ++x)
			if (!CanMonsterOrBubbleMove({ centerPosition.x + x, centerPosition.y + y }))
				return false;
	return true;
}

void GameLevel::OnActorDestroyed(Actor* actor)
{
	// Player 액터가 삭제될 때 Level에서 들고있는 player 포인터를 nullptr로 바꿔줘야 안전함.
	if (actor == player)
	{
		player = nullptr;
	}
}

void GameLevel::ClearActors()
{
	for (Actor* actor : actors)
	{
		actor->Destroy();
	}
}

int GameLevel::MonsterCounter()
{
	int aliveMonster = 0;
	for (Actor* actor : actors)
	{
		if (actor->IsTypeOf<Monster>() || actor->IsTypeOf<Boss>())
		{
			++aliveMonster;
		}
	}
	return aliveMonster;
}

void GameLevel::NextStage()
{
	LoadMap("Test1.txt");
}

void GameLevel::EnemyAllKill()
{
	for (Actor* actor : actors)
	{
		if (actor->IsTypeOf<Monster>() || actor->IsTypeOf<Boss>())
			actor->Destroy();
	}
}

void GameLevel::AddDanger(Vector2 pos, int value) 
{
	if (pos.x >= 0 && pos.x < 20 && pos.y >= 0 && pos.y < 20) 
	{
		dangerMap[pos.y][pos.x] += value;
	}
}

// 현재 칸이 위험한지 확인하는 함수 (에이스타가 호출)
int GameLevel::GetDangerValue(Vector2 pos)
{
	return dangerMap[pos.y][pos.x];
}

void GameLevel::InitCanMoveMap()
{
	// 1. 가로, 세로 크기 설정.
	const int mapWidth = 20;
	const int mapHeight = 20;

	// 2. 2차원 벡터를 0으로 초기화하며 할당
	// std::vector<T>(개수, 초기값) 문법
	// 외부 벡터는 행(Height)의 개수만큼, 내부 벡터는 열(Width)의 개수만큼 생성.
	canMoveMap = std::vector<std::vector<int>>(
		mapHeight,
		std::vector<int>(mapWidth, 0)
	);

}

void GameLevel::UpdateCanMoveMap()
{
	for (int y = 0; y < 17; ++y) // MAP_HEIGHT는 맵의 세로 크기
	{
		for (int x = 0; x < 19; ++x) // MAP_WIDTH는 맵의 가로 크기
		{
			canMoveMap[y][x] = 0;
		}
	}
	for (auto& actor : actors)
	{
		if (actor->IsTypeOf<LightBox>() || actor->IsTypeOf<HeavyBox>() ||
			actor->IsTypeOf<Bubble>() || actor->IsTypeOf<Wall>())
		{
			Vector2 actorPos = actor->GetPosition();
			canMoveMap[actorPos.y][actorPos.x] = 1;
		}
	}
}
