#pragma once
#include <SDL.h>
#include <SDL_image.h>
#include <string>
#include <vector>
#include <stdio.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>
//enums so it is easier to remember
enum Textures {
	PLAYERTEXTURE,
	ENEMYTEXTURE,
	BACKGROUNDTEXTURE,
	BULLETTEXTURE,
	DAMAGEDSHOOTERTEXTURE
};
//Game window size
static const int SCREEN_WIDTH = 1920;
static const int SCREEN_HEIGHT = 1080;
struct transforms {
	float oX = 0;
	float oY = 0;
	double oAngle = 0;
};
struct sprite {
	SDL_Texture* oTexture = NULL;
	int oWidth = 0, oHeight = 0;
};
struct movement {
	float speedMod = 1.0f;
	void Move(transforms &T);
	float oVelY = 0;
	float oVelX = 0;
};
struct player {
	void HandleEvent(SDL_Event& e, movement &M);
	float pierce = 2;
	float damage = 1;
	float fireRate = 1;
	float bspeed = 5;
	float size = 1;
	int upgrades = 0;
	int xp = 0;
	int level = 0;
};
struct enemy {
	void ToPlayer(transforms pT, transforms &T, movement &M);
	int hp = 0;
	float oDrag = 0;
	Uint32 id = 0;
	int damageFrames = 0;
	int xp = 0;
};
struct background {
	void BackgroundIllusion(SDL_FPoint offset, transforms &T);
	SDL_FPoint bgState;
	int tileSize = 16;
};
struct bullet {
	int bPierce = 0;
	int bDamage = 0;
	Uint64 lifespan = 0;
	std::vector<Uint32> history;
};
struct Text
{
	int oX = 0, oY = 0;
	Uint64 spawnTime = 0;
	std::string text = "";
	Text(float x, float y, Uint64 spawntime, std::string dtext)
	{
		oX = x;
		oY = y;
		spawnTime = spawntime;
		text = dtext;
	}
};
//player struct with eventhandler and modifiable speedvariable
struct Player
{
public:
	sprite S;
	transforms T;
	movement M;
	player P;
	Player(SDL_Texture* texture)
	{
		SDL_QueryTexture(texture, NULL, NULL, &S.oWidth, &S.oHeight);
		S.oTexture = texture;
		M.speedMod = 2.0f;
	}	
private:
};

struct Background
{
public:
	sprite S;
	background Bg;
	transforms T;
	Background(SDL_Texture* texture, float x, float y)
	{
		SDL_QueryTexture(texture, NULL, NULL, &S.oWidth, &S.oHeight);
		S.oTexture = texture;
		Bg.bgState = {x,y };
		Bg.tileSize = 64;
	}
private:
};

struct Enemy
{
public:
	sprite S;
	transforms T;
	movement M;
	enemy E;
	Enemy(SDL_Texture* texture, int x, int y, int life, int size, float drag)
	{
		SDL_QueryTexture(texture, NULL, NULL, &S.oWidth, &S.oHeight);
		S.oTexture = texture;
		T.oX = x;
		T.oY = y;
		E.hp = life;
		E.id = SDL_GetTicks();
		E.xp = life*2;
		S.oWidth *= size;
		S.oHeight *= size;
		E.oDrag = drag;
	}
private:
};

struct Bullet
{
public:
	sprite S;
	transforms T;
	bullet B;
	movement M;
	Bullet(SDL_Texture* texture, int x, int y, double angle, int pierce, int damage, float speed, float velX, float velY, float size)
	{
		SDL_QueryTexture(texture, NULL, NULL, &S.oWidth, &S.oHeight);
		S.oTexture = texture;
		T.oX = x;
		T.oY = y;
		T.oAngle = angle;
		B.bPierce = pierce;
		B.bDamage = damage;
		M.speedMod = speed;
		M.oVelX = velX;
		M.oVelY = velY;
		S.oWidth *= size;
		S.oHeight *= size;
		B.lifespan = SDL_GetTicks64();
	}	
private:
};

class Game
{
public:
	//Start the window and game
	bool Start();
	//gameloop 
	void GameLoop();
	//close and free all textures
	void Close();
	//spawns enemy
	void EnemySpawn(SDL_FPoint offset, int hp, float drag, int size);
	void DrawXP(Player *player);
	//renders from variables in the different structs
	template <typename T> void Render(T sprite);
	//load a texture from a filepath
	void LoadTexture(std::string path);
	//Game window
	SDL_Window* gWindow = NULL;
	//Surface of game window
	SDL_Surface* gScreenSurface = NULL;
	//Displayed game window surface
	SDL_Surface* gCurrentSurface = NULL;
	//Renderer
	SDL_Renderer* gRenderer = NULL;
	//show stats
	void RenderText(Player player);
	//draw the fire rate bar over the player
	void DrawFireRate(Player player);
private:
	//timers using int64 to avoid resets
	Uint64 msTimer = 0;
	Uint64 spawnTimer = 0;
	Uint64 shootTimer = 0;
	//space modifier
	SDL_FPoint offset = {0,0};
	//to destrT.oY textures
	std::vector<SDL_Texture*> textures;
	//vectors of all enemies and bullets and damage text
	std::vector<Enemy> enemies;
	std::vector<Bullet> bullets;
	std::vector<Text> damageText;
};
