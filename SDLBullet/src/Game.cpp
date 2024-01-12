#include "Game.h"

bool Game::Start()
{
	//Initializes SDL, TTF and the renderer + window.
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		printf(SDL_GetError());
		return false;
	}
	if (TTF_Init() < 0)
	{
		printf(TTF_GetError());
		return false;
	}
	gWindow = SDL_CreateWindow("BulletGame", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
	if (gWindow == NULL)
	{
		printf(SDL_GetError());
		return false;
	}
	gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED);
	if (gRenderer == NULL)
	{
		printf(SDL_GetError());
		return false;
	}
	gScreenSurface = SDL_GetWindowSurface(gWindow);
	//Here i load all the textures which get put into a vector and used with a enum so i remember which one it is
	LoadTexture("src/Graphic/Player.png");
	LoadTexture("src/Graphic/Shooter.png");
	LoadTexture("src/Graphic/Grass.png");
	LoadTexture("src/Graphic/Bullet.png");
	LoadTexture("src/Graphic/Shooter.png");
	//here i also get one extra so that i have the damaged shooter texture that i can use when they take damage
	SDL_SetTextureColorMod(textures[DAMAGEDSHOOTERTEXTURE], 255, 0, 0);
	return true;
}

void Game::GameLoop()
{
	//starts the music
	Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 64);
	Mix_Music* backgroundMusic = Mix_LoadMUS("src/Graphic/music.wav");
	Mix_PlayMusic(backgroundMusic, -1);
	//event variable
	SDL_Event e;
	SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
	//sets the offset which is so that player stays in the centre while the other can still move
	float xOff = SCREEN_WIDTH / 2;
	float yOff = SCREEN_HEIGHT / 2;
	//makes the player and background
	Player playerObj(textures[PLAYERTEXTURE]);
	Background background(textures[BACKGROUNDTEXTURE],xOff,yOff);
	bool quit = false;
	bool bossExist = false;
	int upgrades = 0;
	spawnTimer = SDL_GetTicks64(); 
	//starts the loop
	while (!quit)
	{
		//spawn enemy every 1000 ms which get faster the more leveled the player is
		if (spawnTimer + 1000*pow(0.95f,(playerObj.P.level+1)) < SDL_GetTicks64() && enemies.size()*pow(1.1f,playerObj.P.level+1) < 40)
		{
			spawnTimer = SDL_GetTicks64();
			//spawns an enemy on the sides of thddddse screen randomly
			EnemySpawn(offset, (floor(playerObj.P.level/5))+2,0.985f, 1);
		}
		if (SDL_GetTicks64() > 20000 && !bossExist)
		{
			//spawns the boss
			bossExist = true;
			EnemySpawn(offset, 30, 0.999f, 5);
		}
		//updatetime
		msTimer = SDL_GetTicks64();
		//handles events
		while (SDL_PollEvent(&e) != 0)
		{
			playerObj.P.HandleEvent(e, playerObj.M);
			//User requests quit
			if (e.type == SDL_QUIT || e.key.keysym.sym == SDLK_ESCAPE)
			{
				quit = true;
			}
			//creates a bullet flying towards the users click
			if (e.type == SDL_MOUSEBUTTONDOWN && shootTimer + playerObj.P.fireRate * 1000 < SDL_GetTicks64()) 
			{
				shootTimer = SDL_GetTicks64();
				int x, y;
				//gets mous position
				SDL_GetMouseState(&x, &y);
				//gets mouse position relative to everything
				x += offset.x;
				y += offset.y;
				//gets the vector between mouse and player
				float diffx = x - playerObj.T.oX;
				float diffy = y - playerObj.T.oY;
				//gets an angle out of it
				double angle = atan2(diffy, diffx) * 180 / M_PI;
				//gives the bullet all its stats, angle and velocity
				Bullet bullet(textures[BULLETTEXTURE], playerObj.T.oX, playerObj.T.oY, angle , playerObj.P.pierce, playerObj.P.damage, playerObj.P.bspeed, diffx, diffy, playerObj.P.size);
				bullets.push_back(bullet);
			}
		}
		//move
		playerObj.M.Move(playerObj.T);
		//fixes movement for all enemies
		for (int i = 0; i < enemies.size(); i++)
		{
			//makes the vectors point towards the player
			enemies[i].E.ToPlayer(playerObj.T, enemies[i].T, enemies[i].M);
			enemies[i].M.Move(enemies[i].T);
		}
		for (int i = 0; i < bullets.size(); i++)
		{
			//moves the bullets
			bullets[i].M.Move(bullets[i].T);
			//makes a rect out of the bullets hitbT.oX
			SDL_Rect bulletRect = { bullets[i].T.oX - offset.x - bullets[i].S.oWidth / 2, bullets[i].T.oY - offset.y - bullets[i].S.oHeight / 2, bullets[i].S.oWidth, bullets[i].S.oHeight };
			if (bullets[i].B.lifespan +10000 < SDL_GetTicks64())
			{
				//bullet dissapears after existing for 10seconds
				bullets.erase(bullets.begin() + i);
				i--;
				continue;
			}
			for (int k = 0; k < enemies.size(); k++)
			{
				//makes an enemy rect
				SDL_Rect enemyRect = { enemies[k].T.oX - offset.x - enemies[k].S.oWidth / 2, enemies[k].T.oY - offset.y - enemies[k].S.oHeight / 2, enemies[k].S.oWidth, enemies[k].S.oHeight };
				//if they intersect and it hasnt intersected before
				if (SDL_HasIntersection(&bulletRect, &enemyRect) && std::count(bullets[i].B.history.begin(), bullets[i].B.history.end(), enemies[k].E.id) == 0)
				{
					//remove enemy hp
					enemies[k].E.hp -= bullets[i].B.bDamage;
					//make dmg text
					Text text(enemies[k].T.oX, enemies[k].T.oY, SDL_GetTicks64(), std::to_string(bullets[i].B.bDamage));
					damageText.push_back(text);
					//get the id so it cant hit same enemy
					bullets[i].B.history.push_back(enemies[k].E.id);
					//if its not a boss they lose all momentum
					if (enemies[k].E.xp != 60)
					{
						enemies[k].M.oVelX = 0;
						enemies[k].M.oVelY = 0;
					}
					//if they die kill them
					if (enemies[k].E.hp < 1)
					{
						playerObj.P.xp += enemies[k].E.xp;
						enemies.erase(enemies.begin() + k);
						k--;
					}
					else enemies[k].E.damageFrames += 10;
					bullets[i].B.bPierce--;
					//if bullets die kill it
					if (bullets[i].B.bPierce < 1)
					{
						bullets.erase(bullets.begin() + i);
						i--;
						break;
					}
				}
			}
		}
		//look for collisions for player (a bit more forgiving than the enemeis and bullets)
		SDL_Rect playerRect = { playerObj.T.oX - offset.x - (playerObj.S.oWidth/1.7f) / 2, playerObj.T.oY - offset.y - (playerObj.S.oHeight/1.7) / 2, playerObj.S.oWidth/1.7f, playerObj.S.oHeight/1.7f};
		for (int k = 0; k < enemies.size(); k++)
		{
			//made the rect for the enemies
			SDL_Rect enemyRect = { enemies[k].T.oX - offset.x - enemies[k].S.oWidth / 2, enemies[k].T.oY - offset.y - enemies[k].S.oHeight / 2, enemies[k].S.oWidth, enemies[k].S.oHeight };
			//hardcore game :)
			if (SDL_HasIntersection(&playerRect, &enemyRect) )
			{
				quit = true;
			}
		}
		//update offset
		offset = {playerObj.T.oX-xOff,playerObj.T.oY-yOff};
		//backgroundmovement
		background.Bg.BackgroundIllusion(offset, background.T);
		//Clear screen
		SDL_RenderClear(gRenderer);
		//renders
		Render(background);
		for (int i = 0; i < bullets.size(); i++)
		{
			Render(bullets[i]);
		}
		Render(playerObj);
		//if the enemies were damaged change their texture for the duration of the damage frames
		for (int i = 0; i < enemies.size(); i++)
		{
			if (enemies[i].E.damageFrames > 0)
			{
				enemies[i].E.damageFrames--;
				if (enemies[i].E.damageFrames == 0)
				{
					enemies[i].S.oTexture = textures[ENEMYTEXTURE];
				}
				else enemies[i].S.oTexture = textures[DAMAGEDSHOOTERTEXTURE];
			}
			Render(enemies[i]);
		}
		//draw the xp bar
		DrawXP(&playerObj);
		//render the dmg text
		RenderText(playerObj);
		//draw the fire rate bar
		DrawFireRate(playerObj);
		//Update screen
		SDL_RenderPresent(gRenderer);
		//limits fps to 60
		if(((SDL_GetTicks64() - msTimer) < 16.666f)) SDL_Delay(16.666f - (SDL_GetTicks64() - msTimer));
	}
	Close();
}

void Game::Close()
{
	//close and shutdown everything
	while (!textures.empty())
	{
		SDL_DestroyTexture(textures[0]);
		textures.erase(textures.begin());
	}
	SDL_DestroyRenderer(gRenderer);
	SDL_DestroyWindow(gWindow);
	gWindow = NULL;
	TTF_Quit();
	SDL_Quit();
}

template<typename T> void Game::Render(T sprite)
{
	//where to render
	SDL_Rect renderQuad = { sprite.T.oX-offset.x-sprite.S.oWidth/2, sprite.T.oY-offset.y-sprite.S.oHeight/2, sprite.S.oWidth, sprite.S.oHeight };
	//Render to screen
	SDL_RenderCopyEx(gRenderer, sprite.S.oTexture, NULL, &renderQuad, sprite.T.oAngle, NULL, SDL_FLIP_NONE);
}

void Game::DrawXP(Player *player)
{
	//if you level up
	if (player->P.xp > 100)
	{
		player->P.xp -= 100;
		player->P.level++;
	}	
	//make a rect and render it
	SDL_Rect renderQuad = { player->T.oX - offset.x-SCREEN_WIDTH/2, player->T.oY - offset.y+SCREEN_HEIGHT/2-SCREEN_HEIGHT/80, (SCREEN_WIDTH/100)*player->P.xp, SCREEN_HEIGHT/80 };
	SDL_RenderFillRect(gRenderer, &renderQuad);
}


void Game::DrawFireRate(Player player)
{
	//make a float of how many % is left of reload
	float fireBar = ((shootTimer + player.P.fireRate * 1000) - SDL_GetTicks64()) / 13;
	if (fireBar > 0)
	{
		//render the fireratebar
		SDL_Rect renderQuad = { player.T.oX - offset.x-fireBar/2, player.T.oY - offset.y-player.S.oHeight/2-SCREEN_HEIGHT/60, fireBar, SCREEN_HEIGHT / 130 };
		SDL_RenderFillRect(gRenderer, &renderQuad);
	}
}

void Game::LoadTexture(std::string path)
{
	//texture to return
	SDL_Texture* newTexture = NULL;
	//Load image to texture
	SDL_Surface* loadedSurface = IMG_Load(path.c_str());
	if (loadedSurface == NULL)
	{
		IMG_GetError();
		return;
	}
	//Create texture from loaded surface
	newTexture = SDL_CreateTextureFromSurface(gRenderer, loadedSurface);
	if (newTexture == NULL)
	{
		printf(SDL_GetError());
		return;
	}
	//free the old surface
	SDL_FreeSurface(loadedSurface);
	textures.push_back(newTexture);
}

void Game::EnemySpawn(SDL_FPoint offset, int hp, float drag, int size)
{
	//get the position relative to the player
	int x = offset.x;
	int y = offset.y;
	//make an int of the circumference of the screen
	int lenghtAround = rand() % (SCREEN_HEIGHT * 2 + SCREEN_WIDTH * 2);
	//places the enemy randomly across the circumference
	if (lenghtAround < SCREEN_WIDTH)
	{
		x += lenghtAround;
	}
	else if (lenghtAround < SCREEN_WIDTH + SCREEN_HEIGHT)
	{
		x += SCREEN_WIDTH;
		y += lenghtAround - SCREEN_WIDTH;
	}
	else if (lenghtAround < SCREEN_WIDTH * 2 + SCREEN_HEIGHT)
	{
		x += lenghtAround - SCREEN_HEIGHT - SCREEN_WIDTH;
		y += SCREEN_HEIGHT;
	}
	else
	{
		y += lenghtAround - SCREEN_HEIGHT - SCREEN_WIDTH*2;
	}
	//makes the enemy object and put it in the list
	Enemy enemy(textures[ENEMYTEXTURE], x, y, hp, size, drag);
	enemies.push_back(enemy);
}

void player::HandleEvent(SDL_Event& e, movement &M)
{
	//handles player movement and upgrades by getting inputs
	if (e.type == SDL_KEYDOWN && e.key.repeat == 0)
	{
		switch (e.key.keysym.sym)
		{
		case SDLK_w: M.oVelY--; break;
		case SDLK_s: M.oVelY++; break;
		case SDLK_a: M.oVelX--; break;
		case SDLK_d: M.oVelX++; break;
		case SDLK_1: {if (level > upgrades) { upgrades++; damage += 1.0f; } break; }
		case SDLK_2: {if (level > upgrades) { upgrades++; pierce += 1.0f; } break; }
		case SDLK_3: {if (level > upgrades) { upgrades++; fireRate *= 0.9f; } break; }
		case SDLK_4: {if (level > upgrades) { upgrades++;  bspeed += 1.5; } break; }
		case SDLK_5: {if (level > upgrades) { upgrades++;  size += 0.5f; } break; }
		}
	}
	else if (e.type == SDL_KEYUP && e.key.repeat == 0)
	{
		switch (e.key.keysym.sym)
		{
		case SDLK_w: M.oVelY++; break;
		case SDLK_s: M.oVelY--; break;
		case SDLK_a: M.oVelX++; break;
		case SDLK_d: M.oVelX--; break;
		}
	}
}

void movement::Move(transforms &T)
{
	//get the velocity of the object and normalizes it to the magnitude of one
	float tempX = oVelX;
	float tempY = oVelY;
	int diff = sqrt(pow(tempX, 2) + pow(tempY, 2));
	if (diff != 0)
	{
		tempX /= diff;
		tempY /= diff;
	}
	//then using speedmod which is the actual indicator for speed
	T.oX += tempX *speedMod;
	T.oY += tempY *speedMod;
}

void background::BackgroundIllusion(SDL_FPoint offset, transforms &T)
{
	//background is made out of tiles and by moving exactly one tile the image wont change from the players perspective
	//moves it the 4 directions if one tile is avalible on the hidden side
	if (offset.x > bgState.x+tileSize - SCREEN_WIDTH)
	{
		bgState.x += tileSize;
		T.oX += tileSize;
	}
	if (offset.x < bgState.x - tileSize - SCREEN_WIDTH)
	{
		bgState.x -= tileSize;
		T.oX -= tileSize;
	}
	if (offset.y > bgState.y + tileSize - SCREEN_HEIGHT)
	{
		bgState.y += tileSize;
		T.oY += tileSize;
	}
	if (offset.y < bgState.y - tileSize - SCREEN_HEIGHT)
	{
		bgState.y -= tileSize;
		T.oY -= tileSize;
	}
}

void enemy::ToPlayer(transforms pT, transforms &T, movement &M)
{
	//gets vector to player from enemy
	float tempX = (pT.oX-T.oX);
	float tempY = (pT.oY-T.oY);
	//gets the vector and magnitude of it to 1
	int diff = sqrt(pow(tempX, 2) + pow(tempY, 2));
	if (diff != 0)
	{
		tempX /= diff;
		tempY /= diff;
	}
	//adds it to the velocity 
	M.oVelX += tempX;
	M.oVelY += tempY;
	//adds drag so it doesnt reach too high speeds
	M.oVelX *= oDrag;
	M.oVelY *= oDrag;
	//the more velocity the faster the enemy gets, making it easier to dodge than just running exactly at you all the time
	M.speedMod = sqrt(pow(M.oVelX, 2) + pow(M.oVelY, 2))/30;
}

void Game::RenderText(Player player)
{
	//opens the font
	TTF_Font* font = TTF_OpenFont("src/Graphic/title.ttf", 16);
	//sets text color
	SDL_Color color = { 255, 255, 255};
	//writes all the stats to a string
	std::string text = "Damage: " + std::to_string(player.P.damage).substr(0,3) + " Pierce: " + std::to_string(player.P.pierce).substr(0, 3) + " Firerate: " + std::to_string(player.P.fireRate).substr(0, 3) + " BSpeed: " + std::to_string(player.P.bspeed).substr(0, 3) + " BSize: " + std::to_string(player.P.size).substr(0, 3);
	//Renders the string on a surface wrapped around the different values
	SDL_Surface* surfaceMessage = TTF_RenderUTF8_Solid_Wrapped(font, text.c_str(), color, 145);
	//makes it into a texture
	SDL_Texture* texture = SDL_CreateTextureFromSurface(gRenderer, surfaceMessage);
	SDL_Rect renderQuad = { 0, SCREEN_HEIGHT-250 - SCREEN_HEIGHT/80, 350, 250 };
	//renders that texture
	SDL_RenderCopy(gRenderer, texture, NULL, &renderQuad);
	//changes the color for the dmg text
	color = { 255, 0, 0 };
	//Frees the old surface
	SDL_FreeSurface(surfaceMessage);
	//destrT.oYs the old texture
	SDL_DestroyTexture(texture);
	for (int i = 0; i < damageText.size(); i++)
	{
		//gets so that if the text is too old it dissapears
		if (damageText[i].spawnTime + 400 < SDL_GetTicks64())
		{
			damageText.erase(damageText.begin() + i);
			i--;
			break;
		}
		//same thing as before but at the enemy
		SDL_Surface* surfaceMessage = TTF_RenderUTF8_Solid(font, damageText[i].text.c_str(), color);
		SDL_Texture* texture = SDL_CreateTextureFromSurface(gRenderer, surfaceMessage);
		SDL_Rect renderQuads = { damageText[i].oX-offset.x, damageText[i].oY-offset.y, 20, 50 };
		SDL_RenderCopy(gRenderer, texture, NULL, &renderQuads);
		SDL_FreeSurface(surfaceMessage);
		SDL_DestroyTexture(texture);
	}
	//closes the font at the end
	TTF_CloseFont(font);
}