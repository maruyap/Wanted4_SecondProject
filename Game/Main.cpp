#include <iostream>
#include "Engine/Engine.h"
#include "Level/GameLevel.h"
#include "Game/Game.h"

#define _CRTDBG_MAP_ALLOC
#include <cstdlib>
#include <crtdbg.h>

using namespace Wanted;

int main()
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	Game game;
	
	game.Run();
}