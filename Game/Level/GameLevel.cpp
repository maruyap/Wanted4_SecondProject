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
		std::cerr << "Failed to open map file.\n";
		__debugbreak();
	}

	// 맵 읽기.
	fseek(file, 0, SEEK_END);
	size_t fileSize = ftell(file);
	rewind(file);

	char* data = new char[fileSize + 1];
	size_t readSize = fread(data, sizeof(char), fileSize, file);
	data[readSize] = '\0'; // 안전을 위해 널 문자 추가.

	int index = 0;
	Wanted::Vector2 position;

	//맵 크기 초기화.
	mapWidth = 0;
	mapHeight = 0;

	while (true)
	{
		if (index >= fileSize)
		{
			// 파일 끝에 도달했을 때 마지막 줄의 높이를 반영.
			if (position.x > 0)
			{
				if (position.x > mapWidth) mapWidth = (int)position.x;
				mapHeight = (int)position.y + 1;
			}
			break;
		}

		char mapCharacter = data[index];
		++index;

		// 윈도우 스타일 개행(\r\n)의 \r 문자 무시 로직.
		if (mapCharacter == '\r') continue;

		if (mapCharacter == '\n')
		{
			// 개행 시 현재까지의 x값 중 최대치를 width로 갱신
			if (position.x > mapWidth) mapWidth = (int)position.x;

			++position.y;
			position.x = 0;
			continue;
		}

		// 한문자씩 처리.
		switch (mapCharacter)
		{
		case '#':
			AddNewActor(new Wall(position));
			break;
		case 'o':
			AddNewActor(new Ground(position));
			break;
		case 'P':
			AddNewActor(player = new Player(position));
			playerStartPosition = position;
			AddNewActor(new Ground(position));
			break;
		case 'L':
			AddNewActor(new LightBox(position));
			AddNewActor(new Ground(position));
			break;
		case 'H':
			AddNewActor(new HeavyBox(position));
			AddNewActor(new Ground(position));
			break;
		case 'M':
			AddNewActor(new Monster(position));
			AddNewActor(new Ground(position));
			break;
		case '*':
			AddNewActor(new Boss(position));
			AddNewActor(new Ground(position));
			break;
		case 'F':
			AddNewActor(new Item(position, 1));
			AddNewActor(new Ground(position));
			break;
		case 'B':
			AddNewActor(new Item(position, 2));
			AddNewActor(new Ground(position));
			break;
		case 'K':
			AddNewActor(new Item(position, 3));
			AddNewActor(new Ground(position));
			break;
		case 'D':
			AddNewActor(new Drone(position));
			AddNewActor(new Ground(position));
			break;
		default:
			// 정의되지 않은 문자는 x 좌표 증가에서 제외하거나 건너뜀.
			continue;
		}

		++position.x;

	}

	// 모든 맵 파싱이 끝난 후 확정된 크기로 맵 배열 초기화.
	InitCanMoveMapAndDangerMap();

	delete[] data;
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

	if (Input::Get().GetKeyDown(VK_F1))
	{
		if (astar)
		{
			astar->isDebuged = !astar->isDebuged; // bool 뒤집기 토글 로직
		}
	}

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
	if (pos.x >= 0 && pos.x < 19 && pos.y >= 0 && pos.y < 17) 
	{
		dangerMap[pos.y][pos.x] += value;
	}
}

// 현재 칸이 위험한지 확인하는 함수 (에이스타가 호출)
int GameLevel::GetDangerValue(Vector2 pos)
{
	return dangerMap[pos.y][pos.x];
}

void GameLevel::InitCanMoveMapAndDangerMap()
{


	// 2. 2차원 벡터를 0으로 초기화하며 할당
	// std::vector<T>(개수, 초기값) 문법
	// 외부 벡터는 행(Height)의 개수만큼, 내부 벡터는 열(Width)의 개수만큼 생성.
	canMoveMap = std::vector<std::vector<int>>(
		mapHeight,
		std::vector<int>(mapWidth, 0)
	);

	dangerMap = std::vector<std::vector<int>>(
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
