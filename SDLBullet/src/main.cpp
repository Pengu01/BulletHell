#include "Game.h"

int main(int argc, char* args[])
{
	Game game;
	if(game.Start())game.GameLoop();
	return 0;
}