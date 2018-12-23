#include "Game.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pScmdline, int iCmdshow)
{
	Game* game;
	bool res;

	game = new Game;
	if (!game) {
		return 0;
	}
	res = game->init();
	if (res) {
		game->run();
	}

	game->shutdown();
	delete game;
	game = 0;

	return 0;
}
