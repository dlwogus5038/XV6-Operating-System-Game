#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"
#include "fcntl.h"

#define MAX_X 20
#define MAX_Y 20
#define MaxSize (1000 + 10)
#define true 1
#define false 0
#define chrDtn ((char)(206))
#define chrWall ((char)(219))
#define chrHero ((char)(219))
#define chrExit ('x')
#define chrRestart ('y')
typedef enum { DOWN, UP, LEFT, RIGHT } Dir;
typedef enum { BEGINNING, RUNNING, OVER, VICTORY, WAITING, EXIT } GameStatus;
typedef int bool;
typedef struct
{
	int x;
	int y;
} Pos;

typedef struct
{
	Pos pos;
	Pos prePos;
	Dir dir;
	bool moveBool;
} Hero;

typedef struct
{
	Pos pos[MAX_X][MAX_Y];
	int flag[MAX_X][MAX_Y];
} Wall;

Pos Dtn;
Hero hero;
Wall wall;

// if the hero is arrived at the destination
bool arrived = 0;
int stage = 1;
int time = 0;
int timeState = 0;
GameStatus gameStatus;

void main_thread();
void init();
int game();
void gameOver();
void gameVictory();
bool inWall(const Pos pos);
bool isPosEqual(const Pos p1, const Pos p2);

// move the hero and check whether hero is arrived at the Dtn or not.
void logic();

// draw the hero and Dtn
void draw();
void draw_hero();
void draw_wall();
void draw_Dtn();
void draw_black();
void draw_stage();
void draw_time();

// Change a char to lowercase if c is uppercase
// Nothing will be done if c is lowercase or not a English letter.
char charLower(const char c);

int Maze1[MAX_X][MAX_Y] =
{
	{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 },
	{ 1,0,1,0,0,0,0,0,1,1,0,0,0,0,0,1,0,0,0,1 },
	{ 1,0,1,0,1,0,1,0,0,0,0,1,0,1,0,1,0,1,0,1 },
	{ 1,0,1,0,1,0,1,1,1,1,1,1,0,1,1,1,0,1,1,1 },
	{ 1,0,1,0,1,1,1,1,0,0,0,1,0,0,0,0,0,0,0,1 },
	{ 1,0,1,0,0,1,0,0,0,1,1,1,1,0,1,1,1,1,0,1 },
	{ 1,0,1,0,1,1,1,1,0,1,0,0,1,0,1,0,0,1,0,1 },
	{ 1,0,1,0,0,0,0,1,0,1,1,0,1,0,1,0,1,1,0,1 },
	{ 1,0,1,0,1,1,0,0,0,0,1,0,1,0,1,0,0,0,0,1 },
	{ 1,0,1,1,1,1,1,1,1,0,1,0,1,0,1,0,1,1,1,1 },
	{ 1,0,1,0,0,0,0,0,0,0,1,0,1,0,1,0,0,0,0,1 },
	{ 1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,0,0 },
	{ 1,0,1,0,1,0,0,0,0,0,0,0,1,0,1,1,1,1,0,1 },
	{ 1,0,1,0,1,0,1,1,1,1,0,1,1,0,1,0,0,1,0,1 },
	{ 1,0,0,0,1,0,1,0,0,1,0,1,0,0,0,0,1,1,1,1 },
	{ 1,0,1,0,1,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1 },
	{ 1,0,1,0,0,0,0,0,0,1,0,0,0,0,1,0,1,0,0,1 },
	{ 1,0,1,1,1,1,1,1,1,1,1,1,1,0,1,0,1,0,1,1 },
	{ 1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1 },
	{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 }
};

int Maze2[MAX_X][MAX_Y] =
{
   { 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 },   
   { 1,0,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1 },
   { 1,1,1,0,1,1,1,1,0,1,0,1,1,1,1,1,1,1,0,1 },
   { 1,0,0,0,1,0,0,0,0,1,0,1,0,0,0,0,0,0,0,1 },
   { 1,0,1,0,1,0,1,1,1,1,0,1,1,1,1,1,1,1,1,1 },
   { 1,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,1 },
   { 1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,0,1 },
   { 1,0,0,0,1,0,0,1,0,1,0,1,0,0,0,0,0,1,0,1 },
   { 1,1,1,0,1,0,1,1,0,1,0,1,0,1,0,1,1,1,0,1 },
   { 1,0,0,0,1,0,1,1,0,1,0,1,0,1,0,0,0,0,0,1 },
   { 1,1,1,0,1,0,1,1,0,1,0,1,0,1,0,1,1,1,1,1 },
   { 1,0,1,0,1,0,0,0,0,1,0,1,0,1,0,1,0,0,0,1 },
   { 1,0,1,0,1,1,1,1,0,1,0,1,0,1,0,1,0,1,1,1 },
   { 1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1 },
   { 1,0,1,0,1,0,0,0,0,0,0,0,0,1,0,1,0,1,0,1 },
   { 1,0,1,0,1,1,1,1,1,1,1,1,1,1,0,0,0,1,0,1 },
   { 1,0,1,0,1,0,0,0,0,0,0,0,0,1,0,0,0,1,0,1 },
   { 1,0,1,0,1,0,1,0,1,1,1,1,0,1,0,1,1,0,0,1 },
   { 1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,0,1,1 },
   { 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 }
};

int Maze3[MAX_X][MAX_Y] =
{
	{ 1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,0,1 },
	{ 1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,1 },
	{ 1,0,1,0,1,1,1,1,0,0,1,1,1,1,1,1,0,1,0,1 },
	{ 1,0,1,0,1,0,0,1,0,0,0,1,0,0,0,0,0,0,0,1 },
	{ 1,0,1,0,1,0,0,1,1,1,0,1,0,0,0,0,0,0,0,1 },
	{ 1,0,1,0,1,0,0,0,0,1,0,1,1,1,1,1,0,1,0,1 },
	{ 1,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,0,0,1 },
	{ 1,0,1,1,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,1 },
	{ 1,0,0,0,0,0,1,0,0,1,1,1,0,1,0,0,0,1,0,1 },
	{ 1,0,1,1,1,1,1,1,0,0,0,1,0,0,0,0,0,1,0,1 },
	{ 1,0,0,0,0,0,1,1,1,1,0,1,1,1,0,0,0,1,0,1 },
	{ 1,1,1,1,1,0,0,0,0,1,0,0,0,1,0,0,0,1,0,1 },
	{ 1,0,0,0,1,0,0,1,0,1,0,0,0,1,1,1,0,1,0,1 },
	{ 1,0,1,0,1,0,0,1,0,1,0,0,0,1,0,1,0,1,0,1 },
	{ 1,0,1,0,1,1,1,1,1,1,0,1,1,1,0,1,0,1,0,1 },
	{ 1,0,1,0,0,0,0,0,0,0,0,1,0,0,0,1,0,1,0,1 },
	{ 1,0,1,1,1,1,1,1,1,1,1,1,0,0,0,1,0,1,0,1 },
	{ 1,0,0,1,0,0,0,0,0,0,0,1,0,1,0,0,0,0,0,1 },
	{ 1,0,0,0,0,0,0,0,0,0,0,1,0,1,0,0,0,0,0,1 },
	{ 1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1 },
};

int Maze4[MAX_X][MAX_Y] =
{
	{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 },
	{ 1,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,1 },
	{ 1,1,1,1,0,1,0,1,1,1,1,1,0,1,0,1,1,1,0,1 },
	{ 1,0,0,1,0,1,0,1,0,0,0,1,0,1,0,0,1,1,0,1 },
	{ 1,0,1,1,0,1,0,1,0,1,1,0,0,1,1,0,1,1,0,1 },
	{ 1,0,0,0,0,1,0,1,0,1,0,1,0,0,0,0,1,1,0,1 },
	{ 1,0,1,1,0,1,1,1,0,1,0,1,0,1,1,0,1,0,0,1 },
	{ 1,0,1,1,0,0,0,0,0,1,0,1,0,1,1,0,1,1,1,1 },
	{ 1,0,1,0,1,1,1,1,1,1,0,0,0,0,1,0,0,0,1,1 },
	{ 1,0,1,0,0,0,0,0,0,0,1,0,1,0,1,1,0,1,1,1 },
	{ 1,0,1,1,0,1,0,0,1,0,0,0,1,0,0,0,0,1,1,1 },
	{ 1,0,1,0,1,0,1,1,1,1,0,1,0,0,0,1,0,1,1,1 },
	{ 1,0,1,0,1,0,1,1,1,1,0,1,0,1,1,1,0,0,1,1 },
	{ 1,0,0,1,1,0,0,0,0,1,0,1,0,1,1,1,1,0,1,1 },
	{ 1,1,0,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,1 },
	{ 1,0,1,0,1,1,0,0,1,1,0,1,1,1,1,0,1,1,0,1 },
	{ 1,0,1,0,1,1,1,0,0,1,0,1,1,1,1,0,0,1,0,1 },
	{ 1,0,0,1,1,1,1,1,0,1,0,1,1,1,1,1,0,1,0,1 },
	{ 1,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1 },
	{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 }
};

int Maze5[MAX_X][MAX_Y] =
{
   { 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 },   
   { 1,0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,1,1,0,1 },
   { 1,0,1,0,1,0,1,0,1,0,1,0,1,1,1,0,1,1,0,1 },
   { 1,0,0,0,1,0,1,0,1,0,1,0,0,0,1,0,0,0,0,1 },
   { 1,1,1,1,1,0,1,0,1,1,1,0,1,1,1,1,1,1,0,1 },
   { 1,0,0,0,1,0,1,0,0,0,1,0,1,0,0,0,0,1,0,1 },
   { 1,0,1,1,1,0,1,1,1,0,1,0,1,0,1,0,1,1,0,1 },
   { 1,0,1,0,0,0,0,0,1,0,1,1,1,0,1,0,1,0,0,1 },
   { 1,0,1,0,1,1,1,1,1,0,1,0,0,0,1,0,0,0,1,1 },
   { 1,0,1,0,1,0,0,0,1,0,1,0,1,1,1,0,1,1,1,1 },
   { 1,0,0,0,1,0,1,1,1,1,1,0,1,0,0,0,0,0,0,1 },
   { 1,0,1,0,1,0,1,0,0,0,1,0,1,0,1,1,1,1,0,1 },
   { 1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1 },
   { 1,0,1,1,1,0,1,0,1,0,1,0,1,0,1,0,1,1,0,1 },
   { 1,0,0,0,0,0,1,0,1,0,1,0,0,0,1,0,1,1,0,1 },
   { 1,1,1,0,1,0,1,0,1,0,1,1,1,0,1,0,1,0,0,1 },
   { 1,0,0,0,1,0,1,0,1,0,0,0,0,0,1,0,1,0,1,1 },
   { 1,0,1,1,1,1,1,0,1,1,1,0,1,1,1,0,1,0,1,1 },
   { 1,0,0,0,0,0,0,0,1,0,0,0,1,0,0,0,0,0,1,1 },
   { 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 }
};

void main_thread()
{
	init();
	set_console_parameters(CONS_NO_BUFFER | CONS_CDEFAULT);
	while (gameStatus != EXIT)
	{
		char c = ' ';
		read(0, &c, 1);
		c = charLower(c);
		if (gameStatus == WAITING)
		{
			while (c != chrRestart && c != chrExit)
			{
				read(0, &c, 1);
				c = charLower(c);
			}
			if (c == chrRestart)
			{
				init();
				continue;
			}
			else
			{
				gameStatus = EXIT;
				break;
			}

		}
		else if (gameStatus == RUNNING || gameStatus == BEGINNING)
		{
			switch (c)
			{
			case 'w':
				hero.dir = UP;
				hero.moveBool = 0;
				if (gameStatus == BEGINNING)
					gameStatus = RUNNING;
				break;
			case 'a':
				hero.dir = LEFT;
				hero.moveBool = 0;
				if (gameStatus == BEGINNING)
					gameStatus = RUNNING;
				break;
			case 'd':
				hero.dir = RIGHT;
				hero.moveBool = 0;
				if (gameStatus == BEGINNING)
					gameStatus = RUNNING;
				break;
			case 's':
				hero.dir = DOWN;
				hero.moveBool = 0;
				if (gameStatus == BEGINNING)
					gameStatus = RUNNING;
				break;
			case chrExit:
				gameStatus = EXIT;
				break;
			default:
				continue;
			}
		}
	}
}

bool inWall(const Pos pos)
{
	if (wall.flag[Dtn.x][Dtn.y] == 1)
		return true;
	return false;
}

void init()
{
	clear_screen();
	hero.pos.x = 1;
	hero.pos.y = 1;
	hero.moveBool = 0;
	hero.prePos.x = 50;
	hero.prePos.y = 50;
	hero.dir = DOWN;
	Dtn.x = 11;
	Dtn.y = 19;
	timeState = 0;

	draw_wall();

	gameStatus = BEGINNING;

	draw_stage();
	draw_Dtn();
	draw_hero();
}

int game()
{
	if (gameStatus == OVER)
		gameOver();
	else if (gameStatus == VICTORY)
		gameVictory();
	else if (gameStatus == RUNNING)
	{
		logic();
		draw();
	}
	else if (gameStatus == EXIT)
	{
		clear_screen();
		set_console_parameters(CONS_BUFFER | CONS_CDEFAULT);
		exit();
	}
	else if (gameStatus == WAITING)
	{
		// do nothing.
	}

	return 0;
}

void gameOver()
{
	stage = 1;
	timeState = 0;
	time = 0;
	clear_screen();
	printf(1, "Game over!\npress %c to restart\npress %c to exit\n", chrRestart, chrExit);
	gameStatus = WAITING;
}

void gameVictory()
{
	stage = 1;
	timeState = 0;
	time = 0;
	clear_screen();
	printf(1, "Vicotry!\npress %c to restart\npress %c to exit\n", chrRestart, chrExit);
	gameStatus = WAITING;
}

void logic()
{
	int x = hero.pos.x;
	int y = hero.pos.y;
	switch (hero.dir)
	{
	case DOWN:
		if (hero.moveBool == 0 && !(wall.flag[x][y + 1] == 1))
		{
			hero.prePos = hero.pos;
			hero.pos.y++;
			hero.moveBool = 1;
		}
		break;
	case UP:
		if (hero.moveBool == 0 && !(wall.flag[x][y - 1] == 1))
		{
			hero.prePos = hero.pos;
			hero.pos.y--;
			hero.moveBool = 1;
		}
		break;
	case RIGHT:
		if (hero.moveBool == 0 && !(wall.flag[x + 1][y] == 1))
		{
			hero.prePos = hero.pos;
			hero.pos.x++;
			hero.moveBool = 1;
		}
		break;
	case LEFT:
		if (hero.moveBool == 0 && !(wall.flag[x - 1][y] == 1))
		{
			hero.prePos = hero.pos;
			hero.pos.x--;
			hero.moveBool = 1;
		}
		break;
	default:
		break;
	}

	if (isPosEqual(hero.pos, Dtn) && arrived == 0)
	{
		arrived = 1;
		if (stage == 5)
		{
			gameStatus = VICTORY;
			return;
		}
		stage++;
		time = 0;
		clear_screen();
		hero.pos.x = 1;
		hero.pos.y = 1;
		hero.moveBool = 0;
		hero.prePos.x = 50;
		hero.prePos.y = 50;
		hero.dir = DOWN;
		arrived = 0;

		draw_wall();

		gameStatus = BEGINNING;

		draw_stage();
		draw_Dtn();
		draw_hero();
	}
	else
		arrived = 0;
}

bool isPosEqual(const Pos p1, const Pos p2)
{
	if (p1.x == p2.x && p1.y == p2.y)
		return true;
	return false;
}

void draw()
{
	if (gameStatus != RUNNING)
		return;
	draw_hero();
	if (arrived == 0 && hero.moveBool == 1) {
		draw_black();
	}
	else
	{
		//do nothing.
	}
}

void draw_black()
{
	int x = hero.prePos.x;
	int y = hero.prePos.y;
	write_at(2 * x, y, ' ');
	write_at(2 * x + 1, y, ' ');
}

void draw_wall(int index)
{
	int i, j;
	set_console_parameters(CONS_NO_BUFFER | (CONS_CWALL * stage));

	for (i = 0; i < MAX_X; i++)
	{
		for (j = 0; j < MAX_Y; j++)
		{
			wall.pos[i][j].x = i;
			wall.pos[i][j].y = j;
			switch (stage)
			{
			case 1:
				timeState = 1;
				wall.flag[i][j] = Maze1[i][j];
				Dtn.x = 11;
				Dtn.y = 19;
				break;
			case 2:
				timeState = 2;
				wall.flag[i][j] = Maze2[i][j];
				hero.pos.x = 1;
				hero.pos.y = 1;
				Dtn.x = 3;
				Dtn.y = 12;
				break;
			case 3:
				timeState = 3;
				wall.flag[i][j] = Maze3[i][j];
				hero.pos.x = 0;
				hero.pos.y = 9;
				Dtn.x = 19;
				Dtn.y = 9;
				break;
			case 4:
				timeState = 4;
				wall.flag[i][j] = Maze4[i][j];
				hero.pos.x = 1;
				hero.pos.y = 1;
				Dtn.x = 5;
				Dtn.y = 6;
				break;
			case 5:
				timeState = 5;
				wall.flag[i][j] = Maze5[i][j];
				hero.pos.x = 1;
				hero.pos.y = 1;
				Dtn.x = 12;
				Dtn.y = 16;
				break;
			}
			if (wall.flag[i][j] == 1)
			{
				write_at(2 * (i), j, chrWall);
				write_at(2 * (i)+1, j, chrWall);
			}
		}
	}
	set_console_parameters(CONS_NO_BUFFER | CONS_CDEFAULT);
}

void draw_Dtn()
{
	set_console_parameters(CONS_NO_BUFFER | CONS_CGOAL);
	write_at(2 * Dtn.x, Dtn.y, chrDtn);
	write_at(2 * Dtn.x + 1, Dtn.y, chrDtn);
	set_console_parameters(CONS_NO_BUFFER | CONS_CDEFAULT);
}

void draw_hero()
{
	set_console_parameters(CONS_NO_BUFFER | CONS_CHERO);
	int x = hero.pos.x;
	int y = hero.pos.y;
	write_at(2 * x, y, chrHero);
	write_at(2 * x + 1, y, chrHero);
	set_console_parameters(CONS_NO_BUFFER | CONS_CDEFAULT);
}

void draw_stage()
{
	set_console_parameters(CONS_NO_BUFFER | (CONS_CWALL * stage));
	set_cursor(15, MAX_Y);
	printf(1, "STAGE : %d", stage);
	set_console_parameters(CONS_NO_BUFFER | CONS_CDEFAULT);
}

void draw_time()
{
	set_console_parameters(CONS_NO_BUFFER | (CONS_CWALL * stage));
	set_cursor(15, MAX_Y + 1);
	printf(1, "time : %d", time);
	set_console_parameters(CONS_NO_BUFFER | CONS_CDEFAULT);
}

char charLower(char c)
{
	if (c >= 'A' && c <= 'Z')
		return(c - 'A' + 'a');
	return c;
}


//multi-thread


void *thread(void *arg)
{
	sleep(10);
	int id = *(int*)arg;
	printf(1, "thread %d: started...\n", id);

	if (id == 0)
	{
		while (sleep(10), gameStatus != EXIT)
			game();
	}
	else if(id == 1)
	{
		while (sleep(100), (gameStatus != EXIT && timeState != 0))
		{
			time++;
			draw_time();

			//Game over?
			switch(timeState)
			{
				case 1:
					if(time >= 30)
					{
						gameStatus = OVER;
						time = 0;
						timeState = 0;
					}
					break;
				case 2:
					if(time >= 25)
					{
						gameStatus = OVER;
						time = 0;
						timeState = 0;
					}
					break;
				case 3:
					if(time >= 20)
					{
						gameStatus = OVER;
						time = 0;
						timeState = 0;
					}
					break;
				case 4:
					if(time >= 15)
					{
						gameStatus = OVER;
						time = 0;
						timeState = 0;
					}
					break;
				case 5:
					if(time >= 10)
					{
						gameStatus = OVER;
						time = 0;
						timeState = 0;
					}
					break;
			}
		}
	}
	else {
		main_thread();
		clear_screen();
		set_console_parameters(CONS_BUFFER | CONS_CDEFAULT);
	}

	exit();
}

int main(int argc, char **argv)
{
	int i;

	void *stacks[3];
	int *args[3];

	for (i = 0; i<3; i++) {
		stacks[i] = (void*)malloc(4096);
		args[i] = (int*)malloc(4);
		*args[i] = i;
	}


	for (i = 0; i<3; i++) {
		clone(thread, args[i], stacks[i]);
	}

	for (i = 0; i<3; i++) {
		void *_;
		join(&_);
	}

	exit();
}
