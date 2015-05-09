// global includes/definitions

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>

#if __ANDROID__

#include <SDL.h>
#include <SDL_byteorder.h>
#include <SDL_image.h>
#include <SDL_gfxPrimitives.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>
#include <SDL_net.h>

#else

#include <SDL/SDL.h>
#include <SDL/SDL_byteorder.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_gfxPrimitives.h>
#include <SDL/SDL_mixer.h>
#include <SDL/SDL_ttf.h>
#include <SDL/SDL_net.h>

#endif

#define SOUND_ON			1

#if __ANDROID__

// 1280x720

#define SCREEN_WIDTH		800
// #define SCREEN_HEIGHT		480
#define SCREEN_HEIGHT		600
	
#else

#define SCREEN_WIDTH 		1024
//#define SCREEN_WIDTH		1366
//#define SCREEN_HEIGHT		768
#define SCREEN_HEIGHT		600		// DEBUG!!

#endif

#define WORLD_WIDTH			512
#define WORLD_HEIGHT		512

#define TILE_WIDTH			20

/* unit colors */
#define RED					0
#define BLUE				1
#define YELLOW				2
#define GREEN				3

/* units */

#define EMPTY				-1
#define TANK				0

/* fire radius */
#define TANK_FIRE_RAD		4

/* tank APs */
#define AP_TANK_MAX			6
#define AP_TANK_MOVE		6
#define AP_TANK_AIM			2
#define AP_TANK_FIRE		2

/* world, grounds */
#define DESERT 		0
#define DIRT 		1
#define FORREST 	2
#define GRASS		3
#define MOUNTAIN 	4
#define SNOW		5
#define WATER		6

/* bases */
#define BASE_RED		7
#define BASE_BLUE		8
#define BASE_YELLOW		9
#define BASE_GREEN		10


#define MAX_PLAYERS	4

/* fonts */
#define FONT_FREEMONO		"data/fonts/FreeMono.ttf"

	
/* server/client communication */
#define GET_PLAYER_NUMBER	0
#define GET_WORLD			1
#define SET_UNIT_TANK		2
#define RECEIVE_UNITS		3
#define SEND_TANK			4 
#define SEND_END			5

#define GET_PLAY_TOKEN		6
#define SEND_TANK_MOVE		7
#define SEND_TANK_FIRE		8
#define USER_MOVE_END		9

/* error codes */
#define OK					0
#define ERROR				1

#define TRUE				1
#define FALSE				0

struct unit_color
{
	Uint8 r;
	Uint8 g;
	Uint8 b;
};

struct tank
{
	Sint16 health;
	Sint16 aim_angle;
	Sint16 aim_x;			/* coordinate for fire */
	Sint16 aim_y;
	Sint16 fire;			/* user selected fire */ 
	Sint16 max_ap;			/* max action points */
	Sint16 ap;
	Sint16 move_x;			/* move to coordinates */
	Sint16 move_y;
	Sint16 timer_ready;
	Sint16 timer_reload;
	Sint16 timer_move;
	Sint16 in_move;
	Sint16 ccs;				/* combat control system */
	Sint16 hull;
	Sint16 motor;
	Sint16 cannon;
	Sint16 give_up;
};

struct unit
{
	Sint16 type;
	Sint32 color;
	void *data;
};

struct player
{
	Uint8 name[30];
	Uint32 points;
};
