// Battle0 - client V 0.1

#include "global_defs.h"

#include "network.h"

#if __ANDROID__

#include <android/log.h>

#define  LOG_TAG    "battle0-log"

#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

#else

#define LOGD(...) 
#define LOGE(...)

#endif


SDL_Surface *ground[11];

SDL_Surface *screen;
Uint8 video_bpp;		/* video bits per pixel */

TTF_Font *hud_font;

Sint16 screen_width = SCREEN_WIDTH, screen_height = SCREEN_HEIGHT;

/* game map */
Uint8 world[WORLD_HEIGHT][WORLD_WIDTH];

/* upper left corner of game map */
Sint16 world_up_x = 0, world_up_y = 0, zoom = 1;

/* units map */
struct unit unit[WORLD_HEIGHT][WORLD_WIDTH];

struct unit_color unit_color[4] = 
{
	{ 204, 24, 24 },
	{ 40, 24, 228 },
	{ 228, 205, 24 },
	{ 24, 184, 31 }
};

/* unit move times from 0 = DESERT to 6 = WATER */
/* -1 = no move to this field possible */
Sint16 move_times_tank[7] =
{
	1, 2, 2, 1, -1, 2, -1
};

Sint32 player_number;

/* player wants to know info about unit, stored here: */
Sint16 unit_info_wx = -1, unit_info_wy = -1;

IPaddress ip;		/* Server address */	
TCPsocket sd, csd;		/* Socket descriptor */
Uint8 ip_str[80];


Sint16 load_ground_bitmaps ()
{
	ground[DESERT] = IMG_Load ("data/gfx/desert.bmp");
	if (! ground[DESERT])
	{
		printf ("ERROR: can't load gfx/desert.bmp!\n");
		return (1);
	}
	
	ground[DIRT] = IMG_Load ("data/gfx/dirt.bmp");
	if (! ground[DIRT])
	{
		printf ("ERROR: can't load gfx/dirt.bmp!\n");
		return (1);
	}
	
	ground[FORREST] = IMG_Load ("data/gfx/forrest.bmp");
	if (! ground[FORREST])
	{
		printf ("ERROR: can't load gfx/forrest.bmp!\n");
		return (1);
	}
	
	ground[GRASS] = IMG_Load ("data/gfx/grass.bmp");
	if (! ground[GRASS])
	{
		printf ("ERROR: can't load gfx/grass.bmp!\n");
		return (1);
	}
	
	ground[MOUNTAIN] = IMG_Load ("data/gfx/mountain.bmp");
	if (! ground[MOUNTAIN])
	{
		printf ("ERROR: can't load gfx/mountain.bmp!\n");
		return (1);
	}
	
	ground[SNOW] = IMG_Load ("data/gfx/snow.bmp");
	if (! ground[SNOW])
	{
		printf ("ERROR: can't load gfx/snow.bmp!\n");
		return (1);
	}
	
	ground[WATER] = IMG_Load ("data/gfx/water.bmp");
	if (! ground[WATER])
	{
		printf ("ERROR: can't load gfx/water.bmp!\n");
		return (1);
	}
	
	/* bases */
	ground[BASE_RED] = IMG_Load ("data/gfx/red.bmp");
	if (! ground[BASE_RED])
	{
		printf ("ERROR: can't load gfx/red.bmp!\n");
		return (1);
	}
	
	ground[BASE_BLUE] = IMG_Load ("data/gfx/blue.bmp");
	if (! ground[BASE_BLUE])
	{
		printf ("ERROR: can't load gfx/blue.bmp!\n");
		return (1);
	}
	
	ground[BASE_YELLOW] = IMG_Load ("data/gfx/yellow.bmp");
	if (! ground[BASE_YELLOW])
	{
		printf ("ERROR: can't load gfx/yellow.bmp!\n");
		return (1);
	}
	
	ground[BASE_GREEN] = IMG_Load ("data/gfx/green.bmp");
	if (! ground[BASE_GREEN])
	{
		printf ("ERROR: can't load gfx/green.bmp!\n");
		return (1);
	}
	
	return (0);
}

int randint(int n) {
  if ((n - 1) == RAND_MAX) {
    return rand();
  } else {
    // Chop off all of the values that would cause skew...
    long end = RAND_MAX / n; // truncate skew
    assert (end > 0L);
    end *= n;

    // ... and ignore results from rand() that fall above that limit.
    // (Worst case the loop condition should succeed 50% of the time,
    // so we can expect to bail out of this loop pretty quickly.)
    int r;
    while ((r = rand()) >= end);

    return r % n;
  }
}

Sint16 draw_text_ttf (TTF_Font *font, unsigned char *textstr, Sint16 x, Sint16 y, Uint8 r, Uint8 g, Uint8 b)
{
    SDL_Surface *text;
    SDL_Rect dstrect;
    SDL_Color color;

    color.r = r;
    color.g = g;
    color.b = b;

    text = TTF_RenderText_Blended (font, textstr, color);
    if (text == NULL)
    {
        printf ("draw_text_ttf: can't render text! %s\n", SDL_GetError ());
        return (FALSE);
    }

    dstrect.x = x;
    dstrect.y = y;
    dstrect.w = text->w;
    dstrect.h = text->h;

    SDL_BlitSurface (text, NULL, screen, &dstrect);
    SDL_FreeSurface (text);

	SDL_UpdateRect (screen, 0, 0, 0, 0);
    SDL_Flip (screen);
	
    return (TRUE);
}

void draw_user_selected_unit (SDL_Surface *screen, Sint16 wx, Sint16 wy, Sint16 zoom)
{
	/* center of selected field */
	Sint16 x_rel = wx - world_up_x;
	Sint16 y_rel = wy - world_up_y;
	
	Sint16 x1 = (x_rel * zoom) + (zoom / 4);
	Sint16 y1 = (y_rel * zoom) + (zoom / 4);
	Sint16 x2 = (x_rel * zoom) + (zoom - (zoom / 4));
	Sint16 y2 = (y_rel * zoom) + (zoom - (zoom / 4));
	
	Sint16 mark_x = x1 + (x2 - x1) / 2;
	Sint16 mark_y = y1 + (y2 - y1) / 2;
	
	circleRGBA (screen, mark_x, mark_y, zoom / 2, 204, 24, 24, 255);
	circleRGBA (screen, mark_x, mark_y, zoom, 204, 24, 24, 255);
}

void draw_move_to (SDL_Surface *screen, Sint16 wx, Sint16 wy, Sint16 zoom)
{
	/* center of selected field */
	Sint16 x_rel = wx - world_up_x;
	Sint16 y_rel = wy - world_up_y;
	
	Sint16 x1 = (x_rel * zoom) + (zoom / 4);
	Sint16 y1 = (y_rel * zoom) + (zoom / 4);
	Sint16 x2 = (x_rel * zoom) + (zoom - (zoom / 4));
	Sint16 y2 = (y_rel * zoom) + (zoom - (zoom / 4));
	
	boxRGBA (screen, x1, y1, x2, y2, 204, 24, 24, 255);
}

Sint16 draw_unit_info (Sint16 wx, Sint16 wy, Sint16 zoom)
{
	Uint8 health[80], health_val[80];
	Uint8 hull[80], hull_val[80];
	Uint8 ccs[80], ccs_val[80];
	Uint8 motor[80], motor_val[80];
	Uint8 cannon[80], cannon_val[80];
	Uint8 timer_ready[80], timer_ready_val[80];
	Uint8 timer_reload[80], timer_reload_val[80];
	Uint8 ap[80], ap_val[80];
	
	Sint16 hud_x = 30; Sint16 hud_y = (screen_height - TILE_WIDTH * 4) + 5;
	
	struct tank *tank;
	
	if (wx < 0 && wy < 0)
	{
		/* undefined, return */
		return (1);
	}
	
	if (unit[wy][wx].color != player_number)
	{
		/* can't show info of enemy tank */
		return (1);
	}
	
	if (unit[wy][wx].type == EMPTY)
	{
		/* no unit here */
		return (1);
	}
	
	if (unit[wy][wx].type == TANK)
	{
		tank = (struct tank *) unit[wy][wx].data;
		
		/* left colummn */
		snprintf (health_val, 80, "%li", tank->health);
		strcpy (health, "health: ");
		strcat (health, health_val);
		draw_text_ttf (hud_font, health, hud_x, hud_y, 0, 0, 0);
		
		snprintf (hull_val, 80, "%li", tank->hull);
		strcpy (hull, "hull: ");
		strcat (hull, hull_val);
		draw_text_ttf (hud_font, hull, hud_x, hud_y + 20, 0, 0, 0);
		
		snprintf (ccs_val, 80, "%li", tank->ccs);
		strcpy (ccs, "ccs: ");
		strcat (ccs, ccs_val);
		draw_text_ttf (hud_font, ccs, hud_x, hud_y + 40, 0, 0, 0);
		
		/* middle colummn 1*/
		snprintf (motor_val, 80, "%li", tank->motor);
		strcpy (motor, "engine: ");
		strcat (motor, motor_val);
		draw_text_ttf (hud_font, motor, hud_x + 150, hud_y, 0, 0, 0);
		
		snprintf (cannon_val, 80, "%li", tank->cannon);
		strcpy (cannon, "cannon: ");
		strcat (cannon, cannon_val);
		draw_text_ttf (hud_font, cannon, hud_x + 150, hud_y + 20, 0, 0, 0);
		
		/* middle colummn 2 */
		snprintf (timer_ready_val, 80, "%li", tank->timer_ready);
		strcpy (timer_ready, "ready in: ");
		strcat (timer_ready, timer_ready_val);
		draw_text_ttf (hud_font, timer_ready, hud_x + 300, hud_y, 0, 0, 0);
		
		snprintf (timer_reload_val, 80, "%li", tank->timer_reload);
		strcpy (timer_reload, "reload in: ");
		strcat (timer_reload, timer_reload_val);
		draw_text_ttf (hud_font, timer_reload, hud_x + 300, hud_y + 20, 0, 0, 0);
		
		/* right colummn */
		snprintf (ap_val, 80, "%li", tank->ap);
		strcpy (ap, "AP: ");
		strcat (ap, ap_val);
		draw_text_ttf (hud_font, ap, hud_x + 450, hud_y, 0, 0, 0);
		
		if (tank->move_x > -1 && tank->move_y > -1)
		{
			draw_move_to (screen, tank->move_x, tank->move_y, zoom);
		}
	}
	
	draw_user_selected_unit (screen, wx, wy, zoom);
	return (0);
}

void draw_tank (Sint16 x, Sint16 y, Uint8 color, Sint16 tank_cannon_angle, Sint16 zoom);

Sint16 draw_world (Sint16 zoom)
{
	Sint16 x, y, wx, wy, gtype, hud_bottom;
	Sint16 max_x, max_y;

	SDL_Rect src;
	SDL_Rect dst;
	
	src.x = 0;
	src.y = 0;
	src.w = TILE_WIDTH;
	src.h = TILE_WIDTH;
	
	struct tank *tank;
	
	/* check limits */
	if (world_up_x < 0) world_up_x = 0;
	if (world_up_y < 0) world_up_y = 0;
	if (world_up_x > WORLD_WIDTH - 1) world_up_x = WORLD_WIDTH - 1;
	if (world_up_y > WORLD_HEIGHT - 1) world_up_x = WORLD_HEIGHT - 1;
	
	if (zoom == 1)
	{
		hud_bottom = 4;		/* tile height of HUD */
	}
	else
	{
		hud_bottom = 2;		/* tile height of HUD */
	}
	
	max_x = world_up_x + (screen_width / (TILE_WIDTH * zoom));
	max_y = world_up_y + (screen_height / (TILE_WIDTH * zoom)) - hud_bottom;
	
	if (max_x >= WORLD_WIDTH) max_x = WORLD_WIDTH - 1;
	if (max_y >= WORLD_HEIGHT) max_y = WORLD_HEIGHT - 1;
	
	boxRGBA (screen, 0, 0, screen_width - 1, screen_height - 1, 238, 231, 38, 255);
	SDL_UpdateRect (screen, 0, 0, 0, 0);
	
	y = 0;
	for (wy = world_up_y; wy < max_y; wy++)
	{
		x = 0;
		for (wx = world_up_x; wx < max_x; wx++)
		{
			dst.x = x;
			dst.y = y;
			dst.w = TILE_WIDTH;
			dst.h = TILE_WIDTH;
			
			gtype = world[wy][wx];
			
			if (zoom == 1)
			{
				if (SDL_BlitSurface (ground[gtype], &src, screen, &dst) != 0)
				{
					printf("SDL_BlitSurface on world tile failed: %s\n", SDL_GetError());
				}
			}
			else
			{
				if (SDL_BlitSurface (ground[gtype], &src, screen, &dst) != 0)
				{
					printf("SDL_BlitSurface on world tile failed: %s\n", SDL_GetError());
				}
				
				dst.x = x + TILE_WIDTH;
				
				if (SDL_BlitSurface (ground[gtype], &src, screen, &dst) != 0)
				{
					printf("SDL_BlitSurface on world tile failed: %s\n", SDL_GetError());
				}
				
				dst.y = y + TILE_WIDTH;
					
				if (SDL_BlitSurface (ground[gtype], &src, screen, &dst) != 0)
				{
					printf("SDL_BlitSurface on world tile failed: %s\n", SDL_GetError());
				}
				
				dst.x = x;
				
				if (SDL_BlitSurface (ground[gtype], &src, screen, &dst) != 0)
				{
					printf("SDL_BlitSurface on world tile failed: %s\n", SDL_GetError());
				}
			}
			
			x += (TILE_WIDTH * zoom);
		}
		y += (TILE_WIDTH * zoom);
	}
	
	/* draw units zoomed in 1x */
	
	for (wy = world_up_y; wy < max_y; wy++)
	{
		for (wx = world_up_x; wx < max_x; wx++)
		{
			if (unit[wy][wx].type == TANK)
			{
				printf ("TANK: %i / %i\n", wx, wy);
				tank = (struct tank *) unit[wy][wx].data;
				
				draw_tank (wx, wy, unit[wy][wx].color, tank->aim_angle, TILE_WIDTH * zoom);
			}
		}
	}
	
	draw_unit_info (unit_info_wx, unit_info_wy, TILE_WIDTH * zoom);
	
	SDL_UpdateRect (screen, 0, 0, 0, 0);
    SDL_Flip (screen);	
}



Sint16 get_mouse_state (Sint16 *mx, Sint16 *my, Sint16 *button, Uint8 wait_event)
{
	SDL_Event event;
	Uint8 buttonmask;
	
	Uint8 wait = 1;

	*button = 0;
	
	if (wait_event)
	{
		while (wait)
		{
			if (! SDL_WaitEvent (&event))
			{
				printf ("get_mouse_state: error can't wait for event!\n");
				return (1);
			}

			switch (event.type)
			{
				case SDL_MOUSEBUTTONDOWN:
					if (event.button.button == SDL_BUTTON_LEFT)
					{
						*mx = event.button.x;
						*my = event.button.y;
						*button = 1;
						
						wait = 0;
					}
					break;
					
					if (event.button.button == SDL_BUTTON_RIGHT)
					{
						*mx = event.button.x;
						*my = event.button.y;
						*button = 2;
						
						wait = 0;
					}
					break;
					
					if (event.button.button == SDL_BUTTON_MIDDLE)
					{
						*mx = event.button.x;
						*my = event.button.y;
						*button = 3;
						
						wait = 0;
					}
					break;
				
				case SDL_MOUSEBUTTONUP:
					*mx = event.button.x;
					*my = event.button.y;
					*button = 0;
					
					wait = 0;
					break;
				
				case SDL_MOUSEMOTION:
					*mx = event.motion.x;
					*my = event.motion.y;
					*button = -1; 
					
					wait = 0;
					break;
			}
		}
	}
	else
	{
		/* don't wait for event, get current mouse state */
		
		SDL_PumpEvents ();
		buttonmask = SDL_GetMouseState ((int *) mx, (int *) my);

		printf ("get_mouse_state: %i / %i\n", *mx, *my);
		
		if (buttonmask & SDL_BUTTON (1))
		{
			*button = 1;
		}

		if (buttonmask & SDL_BUTTON (2))
		{
			*button = 2;
		}

		if (buttonmask & SDL_BUTTON (3))
		{
			*button = 3;
		}
	}
	
	return (0);		/* no error */
}



Sint16 user_move_unit (Sint16 zoom)
{
	Sint16 mx, my;
	Sint16 button = 0;
	Sint16 worldx, worldy, hud_bottom, max_x, max_y;

	struct tank *tank;
	
	if (unit_info_wx == -1 && unit_info_wy == -1)
	{
		/* no unit selected */
		return (1);
	}

	while (button == 0)
	{
		get_mouse_state (&mx, &my, &button, 1);
		
		printf ("mouse: %i / %i\n", mx, my);
		
		worldx = world_up_x + mx / (TILE_WIDTH * zoom);
		worldy = world_up_y + my / (TILE_WIDTH * zoom);
		
		printf ("world: %i / %i\n", worldx, worldy);	
	}
	
	switch (unit[unit_info_wy][unit_info_wx].type)
	{
		case TANK:
			tank = (struct tank *) unit[unit_info_wy][unit_info_wx].data;
			
			if ((worldx >= unit_info_wx - 1 && worldx <= unit_info_wx + 1) && (worldy >= unit_info_wy - 1 && worldy <= unit_info_wy + 1))
			{
				if (unit[worldy][worldx].type == EMPTY && tank->timer_move == 0)
				{
					if (move_times_tank[world[worldy][worldx]] != EMPTY)
					{
						/* set move one field forward mark */
						tank->move_x = worldx; tank->move_y = worldy;
						draw_world (zoom);
					}
				}
			}
			break;
	}
	return (0);
}

Sint16 user_unit_info (Sint16 zoom)
{
	Sint16 mx, my;
	Sint16 button = 0;
	Sint16 worldx, worldy, max_x, max_y;
	
	printf ("unit info\n");
	
	get_mouse_state (&mx, &my, &button, 1);
		
	printf ("mouse: %i / %i\n", mx, my);
		
	worldx = world_up_x + mx / (TILE_WIDTH * zoom);
	worldy = world_up_y + my / (TILE_WIDTH * zoom);
		
	printf ("world: %i / %i\n", worldx, worldy);	
	
	unit_info_wx = worldx; unit_info_wy = worldy;
	
	draw_world (zoom);
	
	button = 1;
	while (button == -1 || button > 0)
	{
		get_mouse_state (&mx, &my, &button, 1);
		
		worldx = world_up_x + mx / (TILE_WIDTH * zoom);
		worldy = world_up_y + my / (TILE_WIDTH * zoom);
		
		printf ("button: %li\n", button);
		printf ("worldx: %li / unit_info_wx: %li, worldy: %li / unit_info_wy: %li\n", worldx, unit_info_wx, worldy, unit_info_wy); 
		
		
		if ((worldx != unit_info_wx || worldy != unit_info_wy) || (worldx != unit_info_wx && worldy != unit_info_wy))
		{
			/* user wants touch move */
			
			printf ("TOUCH MOVE\n");
			
			user_move_unit (zoom);
			button = 0;
		}
	}
	
	return (0);
}
					
Sint16 set_tank (Sint16 x, Sint16 y, Uint8 color, Sint16 angle, Sint16 zoom);

Sint16 user_place_tank (Sint16 zoom)
{
	Sint16 mx, my;
	Sint16 button = 0;
	Sint16 worldx, worldy, hud_bottom, max_x, max_y;
	
	Sint32 command;
	Sint32 ack;
	
	if (zoom == 1)
	{
		hud_bottom = 4;		/* tile height of HUD */
	}
	else
	{
		hud_bottom = 2;		/* tile height of HUD */
	}
	
	max_x = world_up_x + (screen_width / (TILE_WIDTH * zoom));
	max_y = world_up_y + (screen_height / (TILE_WIDTH * zoom)) - hud_bottom;
	
	while (button == 0)
	{
		get_mouse_state (&mx, &my, &button, 1);
		
		printf ("mouse: %i / %i\n", mx, my);
		
		worldx = world_up_x + mx / (TILE_WIDTH * zoom);
		worldy = world_up_y + my / (TILE_WIDTH * zoom);
		
		printf ("world: %i / %i\n", worldx, worldy);	
	}
	
	if (unit[worldy][worldx].type != EMPTY)
	{
		/* field not empty, can't set unit! */
		printf ("field not empty!\n");
		return (1);
	}
	
	if (world[worldy][worldx] == MOUNTAIN)
	{
		/* can't set tank on mountain */
		printf ("field is mountain!\n");
		return (1);
	}
	
	/* Open a connection with the IP provided (listen on the host's port) */
	if (!(sd = SDLNet_TCP_Open (&ip)))
	{
		fprintf (stderr, "SDLNet_TCP_Open: %s\n", SDLNet_GetError ());
		return (1);
	}
	
	command = SET_UNIT_TANK;
	
	if (send_data (sd, &command, sizeof (command), BYTEORDER_NET) != 0)
	{
		return (1);
	}
	
	if (send_data (sd, &player_number, sizeof (player_number), BYTEORDER_NET) != 0)
	{
		return (1);
	}
	
	if (send_data (sd, &worldx, sizeof (worldx), BYTEORDER_NET) != 0)
	{
		return (1);
	}
	
	if (send_data (sd, &worldy, sizeof (worldy), BYTEORDER_NET) != 0)
	{
		return (1);
	}
	
	if (recv_data (sd, &ack, sizeof (ack), BYTEORDER_NET) != 0)
	{
		return (1);
	}
	
	SDLNet_TCP_Close (sd);
	
	if (ack == OK)
	{
		set_tank (worldx, worldy, player_number, 0, zoom * TILE_WIDTH);
		draw_world (zoom);
		return (0);
	}
	else
	{
		return (1);
	}
}
	
#if __ANDROID__

/* no thickLineRGBA in Pelyas SDL lib */

/*!
\brief The structure passed to the internal Bresenham iterator.
*/
typedef struct {
	Sint16 x, y;
	int dx, dy, s1, s2, swapdir, error;
	Uint32 count;
} SDL_gfxBresenhamIterator;

/*!
\brief The structure passed to the internal Murphy iterator.
*/
typedef struct {
	Uint32 color;
	SDL_Surface *dst;
	int u, v;		/* delta x , delta y */
	int ku, kt, kv, kd;	/* loop constants */
	int oct2;
	int quad4;
	Sint16 last1x, last1y, last2x, last2y, first1x, first1y, first2x, first2y, tempx, tempy;
} SDL_gfxMurphyIterator;

/*!
\brief Internal function to initialize the Bresenham line iterator.

Example of use:
SDL_gfxBresenhamIterator b;
_bresenhamInitialize (&b, x1, y1, x2, y2);
do { 
plot(b.x, b.y); 
} while (_bresenhamIterate(&b)==0); 

\param b Pointer to struct for bresenham line drawing state.
\param x1 X coordinate of the first point of the line.
\param y1 Y coordinate of the first point of the line.
\param x2 X coordinate of the second point of the line.
\param y2 Y coordinate of the second point of the line.

\returns Returns 0 on success, -1 on failure.
*/
int _bresenhamInitialize(SDL_gfxBresenhamIterator *b, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2)
{
	int temp;

	if (b==NULL) {
		return(-1);
	}

	b->x = x1;
	b->y = y1;

	/* dx = abs(x2-x1), s1 = sign(x2-x1) */
	if ((b->dx = x2 - x1) != 0) {
		if (b->dx < 0) {
			b->dx = -b->dx;
			b->s1 = -1;
		} else {
			b->s1 = 1;
		}
	} else {
		b->s1 = 0;	
	}

	/* dy = abs(y2-y1), s2 = sign(y2-y1)    */
	if ((b->dy = y2 - y1) != 0) {
		if (b->dy < 0) {
			b->dy = -b->dy;
			b->s2 = -1;
		} else {
			b->s2 = 1;
		}
	} else {
		b->s2 = 0;	
	}

	if (b->dy > b->dx) {
		temp = b->dx;
		b->dx = b->dy;
		b->dy = temp;
		b->swapdir = 1;
	} else {
		b->swapdir = 0;
	}

	b->count = (b->dx<0) ? 0 : (unsigned int)b->dx;
	b->dy <<= 1;
	b->error = b->dy - b->dx;
	b->dx <<= 1;	

	return(0);
}


/*!
\brief Internal function to move Bresenham line iterator to the next position.

Maybe updates the x and y coordinates of the iterator struct.

\param b Pointer to struct for bresenham line drawing state.

\returns Returns 0 on success, 1 if last point was reached, 2 if moving past end-of-line, -1 on failure.
*/
int _bresenhamIterate(SDL_gfxBresenhamIterator *b)
{	
	if (b==NULL) {
		return (-1);
	}

	/* last point check */
	if (b->count==0) {
		return (2);
	}

	while (b->error >= 0) {
		if (b->swapdir) {
			b->x += b->s1;
		} else  {
			b->y += b->s2;
		}

		b->error -= b->dx;
	}

	if (b->swapdir) {
		b->y += b->s2;
	} else {
		b->x += b->s1;
	}

	b->error += b->dy;	
	b->count--;		

	/* count==0 indicates "end-of-line" */
	return ((b->count) ? 0 : 1);
}



/*!
\brief Internal function to to draw parallel lines with Murphy algorithm.

\param m Pointer to struct for murphy iterator.
\param x X coordinate of point.
\param y Y coordinate of point.
\param d1 Direction square/diagonal.
*/
void _murphyParaline(SDL_gfxMurphyIterator *m, Sint16 x, Sint16 y, int d1)
{
	int p;
	d1 = -d1;

	/*
	* Lock the surface 
	*/
	if (SDL_MUSTLOCK(m->dst)) {
		SDL_LockSurface(m->dst);
	}

	for (p = 0; p <= m->u; p++) {

		pixelColorNolock(m->dst, x, y, m->color);

		if (d1 <= m->kt) {
			if (m->oct2 == 0) {
				x++;
			} else {
				if (m->quad4 == 0) {
					y++;
				} else {
					y--;
				}
			}
			d1 += m->kv;
		} else {	
			x++;
			if (m->quad4 == 0) {
				y++;
			} else {
				y--;
			}
			d1 += m->kd;
		}
	}

	/* Unlock surface */
	if (SDL_MUSTLOCK(m->dst)) {
		SDL_UnlockSurface(m->dst);
	}

	m->tempx = x;
	m->tempy = y;
}

/*!
\brief Internal function to to draw one iteration of the Murphy algorithm.

\param m Pointer to struct for murphy iterator.
\param miter Iteration count.
\param ml1bx X coordinate of a point.
\param ml1by Y coordinate of a point.
\param ml2bx X coordinate of a point.
\param ml2by Y coordinate of a point.
\param ml1x X coordinate of a point.
\param ml1y Y coordinate of a point.
\param ml2x X coordinate of a point.
\param ml2y Y coordinate of a point.

*/
void _murphyIteration(SDL_gfxMurphyIterator *m, Uint8 miter, 
	Uint16 ml1bx, Uint16 ml1by, Uint16 ml2bx, Uint16 ml2by, 
	Uint16 ml1x, Uint16 ml1y, Uint16 ml2x, Uint16 ml2y)
{
	int atemp1, atemp2;
	int ftmp1, ftmp2;
	Uint16 m1x, m1y, m2x, m2y;	
	Uint16 fix, fiy, lax, lay, curx, cury;
	Uint16 px[4], py[4];
	SDL_gfxBresenhamIterator b;

	if (miter > 1) {
		if (m->first1x != -32768) {
			fix = (m->first1x + m->first2x) / 2;
			fiy = (m->first1y + m->first2y) / 2;
			lax = (m->last1x + m->last2x) / 2;
			lay = (m->last1y + m->last2y) / 2;
			curx = (ml1x + ml2x) / 2;
			cury = (ml1y + ml2y) / 2;

			atemp1 = (fix - curx);
			atemp2 = (fiy - cury);
			ftmp1 = atemp1 * atemp1 + atemp2 * atemp2;
			atemp1 = (lax - curx);
			atemp2 = (lay - cury);
			ftmp2 = atemp1 * atemp1 + atemp2 * atemp2;

			if (ftmp1 <= ftmp2) {
				m1x = m->first1x;
				m1y = m->first1y;
				m2x = m->first2x;
				m2y = m->first2y;
			} else {
				m1x = m->last1x;
				m1y = m->last1y;
				m2x = m->last2x;
				m2y = m->last2y;
			}

			atemp1 = (m2x - ml2x);
			atemp2 = (m2y - ml2y);
			ftmp1 = atemp1 * atemp1 + atemp2 * atemp2;
			atemp1 = (m2x - ml2bx);
			atemp2 = (m2y - ml2by);
			ftmp2 = atemp1 * atemp1 + atemp2 * atemp2;

			if (ftmp2 >= ftmp1) {
				ftmp1 = ml2bx;
				ftmp2 = ml2by;
				ml2bx = ml2x;
				ml2by = ml2y;
				ml2x = ftmp1;
				ml2y = ftmp2;
				ftmp1 = ml1bx;
				ftmp2 = ml1by;
				ml1bx = ml1x;
				ml1by = ml1y;
				ml1x = ftmp1;
				ml1y = ftmp2;
			}

			/*
			* Lock the surface 
			*/
			if (SDL_MUSTLOCK(m->dst)) {
				SDL_LockSurface(m->dst);
			}

			_bresenhamInitialize(&b, m2x, m2y, m1x, m1y);
			do {
				pixelColorNolock(m->dst, b.x, b.y, m->color);
			} while (_bresenhamIterate(&b)==0);

			_bresenhamInitialize(&b, m1x, m1y, ml1bx, ml1by);
			do {
				pixelColorNolock(m->dst, b.x, b.y, m->color);
			} while (_bresenhamIterate(&b)==0);

			_bresenhamInitialize(&b, ml1bx, ml1by, ml2bx, ml2by);
			do {
				pixelColorNolock(m->dst, b.x, b.y, m->color);
			} while (_bresenhamIterate(&b)==0);

			_bresenhamInitialize(&b, ml2bx, ml2by, m2x, m2y);
			do {
				pixelColorNolock(m->dst, b.x, b.y, m->color);
			} while (_bresenhamIterate(&b)==0);

			/* Unlock surface */
			if (SDL_MUSTLOCK(m->dst)) {
				SDL_UnlockSurface(m->dst);
			}

			px[0] = m1x;
			px[1] = m2x;
			px[2] = ml1bx;
			px[3] = ml2bx;
			py[0] = m1y;
			py[1] = m2y;
			py[2] = ml1by;
			py[3] = ml2by;			
			polygonColor(m->dst, px, py, 4, m->color);						
		}
	}

	m->last1x = ml1x;
	m->last1y = ml1y;
	m->last2x = ml2x;
	m->last2y = ml2y;
	m->first1x = ml1bx;
	m->first1y = ml1by;
	m->first2x = ml2bx;
	m->first2y = ml2by;
}


#define HYPOT(x,y) sqrt((double)(x)*(double)(x)+(double)(y)*(double)(y)) 



/*!
\brief Internal function to to draw wide lines with Murphy algorithm.

Draws lines parallel to ideal line.

\param m Pointer to struct for murphy iterator.
\param x1 X coordinate of first point.
\param y1 Y coordinate of first point.
\param x2 X coordinate of second point.
\param y2 Y coordinate of second point.
\param width Width of line.
\param miter Iteration count.

*/
void _murphyWideline(SDL_gfxMurphyIterator *m, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Uint8 width, Uint8 miter)
{	
	float offset = (float)width / 2.f;

	Sint16 temp;
	Sint16 ptx, pty, ptxx, ptxy, ml1x, ml1y, ml2x, ml2y, ml1bx, ml1by, ml2bx, ml2by;

	int d0, d1;		/* difference terms d0=perpendicular to line, d1=along line */

	int q;			/* pel counter,q=perpendicular to line */
	int tmp;

	int dd;			/* distance along line */
	int tk;			/* thickness threshold */
	double ang;		/* angle for initial point calculation */
	double sang, cang;

	/* Initialisation */
	m->u = x2 - x1;	/* delta x */
	m->v = y2 - y1;	/* delta y */

	if (m->u < 0) {	/* swap to make sure we are in quadrants 1 or 4 */
		temp = x1;
		x1 = x2;
		x2 = temp;
		temp = y1;
		y1 = y2;
		y2 = temp;		
		m->u *= -1;
		m->v *= -1;
	}

	if (m->v < 0) {	/* swap to 1st quadrant and flag */
		m->v *= -1;
		m->quad4 = 1;
	} else {
		m->quad4 = 0;
	}

	if (m->v > m->u) {	/* swap things if in 2 octant */
		tmp = m->u;
		m->u = m->v;
		m->v = tmp;
		m->oct2 = 1;
	} else {
		m->oct2 = 0;
	}

	m->ku = m->u + m->u;	/* change in l for square shift */
	m->kv = m->v + m->v;	/* change in d for square shift */
	m->kd = m->kv - m->ku;	/* change in d for diagonal shift */
	m->kt = m->u - m->kv;	/* diag/square decision threshold */

	d0 = 0;
	d1 = 0;
	dd = 0;

	ang = atan((double) m->v / (double) m->u);	/* calc new initial point - offset both sides of ideal */	
	sang = sin(ang);
	cang = cos(ang);

	if (m->oct2 == 0) {
		ptx = x1 + (Sint16)lrint(offset * sang);
		if (m->quad4 == 0) {
			pty = y1 - (Sint16)lrint(offset * cang);
		} else {
			pty = y1 + (Sint16)lrint(offset * cang);
		}
	} else {
		ptx = x1 - (Sint16)lrint(offset * cang);
		if (m->quad4 == 0) {
			pty = y1 + (Sint16)lrint(offset * sang);
		} else {
			pty = y1 - (Sint16)lrint(offset * sang);
		}
	}

	/* used here for constant thickness line */
	tk = (int) (4. * HYPOT(ptx - x1, pty - y1) * HYPOT(m->u, m->v));

	if (miter == 0) {
		m->first1x = -32768;
		m->first1y = -32768;
		m->first2x = -32768;
		m->first2y = -32768;
		m->last1x = -32768;
		m->last1y = -32768;
		m->last2x = -32768;
		m->last2y = -32768;
	}
	ptxx = ptx;
	ptxy = pty;

	for (q = 0; dd <= tk; q++) {	/* outer loop, stepping perpendicular to line */

		_murphyParaline(m, ptx, pty, d1);	/* call to inner loop - right edge */
		if (q == 0) {
			ml1x = ptx;
			ml1y = pty;
			ml1bx = m->tempx;
			ml1by = m->tempy;
		} else {
			ml2x = ptx;
			ml2y = pty;
			ml2bx = m->tempx;
			ml2by = m->tempy;
		}
		if (d0 < m->kt) {	/* square move */
			if (m->oct2 == 0) {
				if (m->quad4 == 0) {
					pty++;
				} else {
					pty--;
				}
			} else {
				ptx++;
			}
		} else {	/* diagonal move */
			dd += m->kv;
			d0 -= m->ku;
			if (d1 < m->kt) {	/* normal diagonal */
				if (m->oct2 == 0) {
					ptx--;
					if (m->quad4 == 0) {
						pty++;
					} else {
						pty--;
					}
				} else {
					ptx++;
					if (m->quad4 == 0) {
						pty--;
					} else {
						pty++;
					}
				}
				d1 += m->kv;
			} else {	/* double square move, extra parallel line */
				if (m->oct2 == 0) {
					ptx--;
				} else {
					if (m->quad4 == 0) {
						pty--;
					} else {
						pty++;
					}
				}
				d1 += m->kd;
				if (dd > tk) {
					_murphyIteration(m, miter, ml1bx, ml1by, ml2bx, ml2by, ml1x, ml1y, ml2x, ml2y);
					return;	/* breakout on the extra line */
				}
				_murphyParaline(m, ptx, pty, d1);
				if (m->oct2 == 0) {
					if (m->quad4 == 0) {
						pty++;
					} else {

						pty--;
					}
				} else {
					ptx++;
				}
			}
		}
		dd += m->ku;
		d0 += m->kv;
	}

	_murphyIteration(m, miter, ml1bx, ml1by, ml2bx, ml2by, ml1x, ml1y, ml2x, ml2y);
}


/*!
\brief Draw a thick line with alpha blending.

\param dst The surface to draw on.
\param x1 X coordinate of the first point of the line.
\param y1 Y coordinate of the first point of the line.
\param x2 X coordinate of the second point of the line.
\param y2 Y coordinate of the second point of the line.
\param width Width of the line in pixels. Must be >0.
\param color The color value of the line to draw (0xRRGGBBAA). 

\returns Returns 0 on success, -1 on failure.
*/
int thickLineColor(SDL_Surface * dst, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Uint8 width, Uint32 color)
{	
	int wh;
	SDL_gfxMurphyIterator m;

	if (dst == NULL) return -1;
	if (width < 1) return -1;

	/* Special case: thick "point" */
	if ((x1 == x2) && (y1 == y2)) {
		wh = width / 2;
		return boxColor(dst, x1 - wh, y1 - wh, x2 + width, y2 + width, color);		
	}

	m.dst = dst;
	m.color = color;

	_murphyWideline(&m, x1, y1, x2, y2, width, 0);
	_murphyWideline(&m, x1, y1, x2, y2, width, 1);

	return(0);
}

/*!
\brief Draw a thick line with alpha blending.

\param dst The surface to draw on.
\param x1 X coordinate of the first point of the line.
\param y1 Y coordinate of the first point of the line.
\param x2 X coordinate of the second point of the line.
\param y2 Y coordinate of the second point of the line.
\param width Width of the line in pixels. Must be >0.
\param r The red value of the character to draw. 
\param g The green value of the character to draw. 
\param b The blue value of the character to draw. 
\param a The alpha value of the character to draw.

\returns Returns 0 on success, -1 on failure.
*/	
int thickLineRGBA(SDL_Surface * dst, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Uint8 width, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	return (thickLineColor(dst, x1, y1, x2, y2, width, 
		((Uint32) r << 24) | ((Uint32) g << 16) | ((Uint32) b << 8) | (Uint32) a));
}

#endif


void draw_line_rotated (SDL_Surface *screen, Sint16 x, Sint16 y, Sint16 length, Sint16 angle, Sint16 width, Uint8 r, Uint8 g, Uint8 b)
{
	double aspect_ratio = 1.0, dangle, rads, dr;
	double dradius = length / (aspect_ratio * 2.0);
	int ra, x2, y2;
	
	dangle = angle;
	rads = dangle + 90.0 * M_PI / 180.0;
			
	dr = cos (rads);
	dr = dr * dradius;
	ra = dr;
		
	x2 = x + ra;
		
	dr = sin (rads);
	dr = dr * dradius * aspect_ratio;
		
	ra = dr;
		
	y2 = y - ra;
	
	thickLineRGBA (screen, x, y, x2, y2, width, r, g, b, 255);
}
				
void draw_tank (Sint16 x, Sint16 y, Uint8 color, Sint16 tank_cannon_angle, Sint16 zoom)
{
	Sint16 x_rel = x - world_up_x;
	Sint16 y_rel = y - world_up_y;
	
	Sint16 tank_x1 = (x_rel * zoom) + (zoom / 4);
	Sint16 tank_y1 = (y_rel * zoom) + (zoom / 4);
	Sint16 tank_x2 = (x_rel * zoom) + (zoom - (zoom / 4));
	Sint16 tank_y2 = (y_rel * zoom) + (zoom - (zoom / 4));

	Sint16 tank_left_chain_x = tank_x1 - (zoom / 8);
	Sint16 tank_right_chain_x = tank_x2 + (zoom / 8);
	
	Sint16 tank_cannon_x = tank_x1 + (tank_x2 - tank_x1) / 2;
	Sint16 tank_cannon_y = tank_y1 + (tank_y2 - tank_y1) / 2;
	Sint16 tank_cannon_width = zoom / 8;
	Sint16 tank_cannon_length = zoom;
	
/* draw tank */

	boxRGBA (screen, tank_x1, tank_y1, tank_x2, tank_y2, unit_color[color].r, unit_color[color].g, unit_color[color].b, 255);
	
	boxRGBA (screen, tank_left_chain_x, tank_y1, tank_x1, tank_y2, 0, 0, 0, 255);
	boxRGBA (screen, tank_right_chain_x, tank_y1, tank_x2, tank_y2, 0, 0, 0, 255);
	
	filledCircleRGBA (screen, tank_cannon_x, tank_cannon_y, tank_cannon_width, 0, 0, 0, 255);
	
	draw_line_rotated (screen, tank_cannon_x, tank_cannon_y, tank_cannon_length, tank_cannon_angle, tank_cannon_width / 2, 0, 0, 0);
	
	SDL_UpdateRect (screen, 0, 0, 0, 0);
    SDL_Flip (screen);
}
	
Sint16 set_tank (Sint16 x, Sint16 y, Uint8 color, Sint16 angle, Sint16 zoom)
{
	struct tank *tank;
	
	printf ("SET TANK: %i / %i\n", x, y);
	
	unit[y][x].type = TANK;
	unit[y][x].color = color;
	unit[y][x].data = (struct tank *) malloc (sizeof (struct tank));
	if (unit[y][x].data == NULL)
	{
		printf ("ERROR no memory for unit!\n");
		return (1);
	}
	
	
	/* set data */
	tank = (struct tank *) unit[y][x].data;
	
	tank->health = 100;
	tank->aim_angle = angle;
	tank->ap = 5;
	tank->max_ap = 5;
	tank->move_x = -1;		/* no move < 0 */
	tank->move_y = -1;
	tank->timer_ready = 1;
	tank->timer_move = 0;
	tank->timer_reload = 3;
	tank->ccs = 100;
	tank->hull = 100;
	tank->motor = 100;
	tank->cannon = 100;
	tank->give_up = 0;
	
	draw_tank (x, y, color, angle, zoom);
	
	return (0);
}

Sint16 moves_finished (void)
{
	/* check tank moves and send them */
	
	Sint16 wx, wy, j;
	Uint8 buf[4];
	Sint32 data;
	
	struct tank *tank;
	
	for (wy = 0; wy < WORLD_HEIGHT; wy++)
	{
		for (wx = 0; wx < WORLD_WIDTH; wx++)
		{
			if (unit[wy][wx].type != EMPTY)
			{
				switch (unit[wy][wx].type)
				{
					case TANK:
						tank = (struct tank *) unit[wy][wx].data;
						
						if (tank->move_x > -1 && tank->move_y > -1 && tank->timer_move == 0)
						{
							/* Open a connection with the IP provided (listen on the host's port) */
							if (!(sd = SDLNet_TCP_Open (&ip)))
							{
								fprintf (stderr, "SDLNet_TCP_Open: %s\n", SDLNet_GetError ());
								exit (EXIT_FAILURE);
							}
							/* tank got move command, send data... */
							printf ("send unit tank move\n");
							
							if (send_unit_tank_move (sd, wx, wy) != 0)
							{
								return (1);
							}
							
							SDLNet_TCP_Close (sd);
						}
						break;
				}
			}
		}
	}
	
	
	/* Open a connection with the IP provided (listen on the host's port) */
	if (!(sd = SDLNet_TCP_Open (&ip)))
	{
		fprintf (stderr, "SDLNet_TCP_Open: %s\n", SDLNet_GetError ());
		exit (EXIT_FAILURE);
	}
	
	printf ("send user move end\n");
	
	data = USER_MOVE_END;
	if (send_data (sd, (Uint8 *) &data, sizeof (data), BYTEORDER_NET) != 0)
	{
		return (1);
	}
	
	if (send_data (sd, (Uint8 *) &player_number, sizeof (player_number), BYTEORDER_NET) != 0)
	{
		return (1);
	}
	
	SDLNet_TCP_Close (sd);
	
	return (0);
}


Sint16 get_server_ip ()
{
	Uint8 request[256];
	SDL_Event event;
    SDLKey key;
	Uint8 wait;
	Sint16 ip_len;
	Sint16 screen_reopened;
	
	strcpy (ip_str, "192.168.1.1");
	strcpy (request, "enter server ip: ");
	strcat (request, ip_str);
	ip_len = strlen (ip_str);
	
	/*
	draw_text_ttf (hud_font, request, 50, 50, 255, 255, 255);
	SDL_UpdateRect (screen, 0, 0, 0, 0);
	SDL_Flip (screen);
	*/
	
	#if __ANDROID__
	/* show on screen keyboard */
	SDL_ANDROID_ToggleScreenKeyboardWithoutTextInput ();
	#endif
	
	wait = TRUE;

    while (wait)
    {
		SDL_Delay (50);
		while (SDL_PollEvent (&event))
		{
			screen_reopened = 0;
			switch (event.type)
			{
				case SDL_ACTIVEEVENT:
				case SDL_VIDEORESIZE:
					while (screen_reopened == 0)
					{
						while (SDL_PollEvent (&event))
						{
							if (event.type == SDL_VIDEORESIZE || event.type == SDL_ACTIVEEVENT)
							{
								//If the window was iconified or restored
				
								LOGD("reopening screen...");
								
								screen = SDL_SetVideoMode (event.resize.w, event.resize.h, video_bpp, SDL_SWSURFACE | SDL_SRCALPHA | SDL_RESIZABLE);
								if (screen == NULL)
								{
									fprintf (stderr, "Couldn't set %i x %i x %i video mode: %s\n", screen_width, screen_height, 24, SDL_GetError ());
									
									#if __ANDROID__
									/* hide on screen keyboard */
									SDL_ANDROID_ToggleScreenKeyboardWithoutTextInput ();
									#endif
									
									return (1);
								}

								/* Use alpha blending */
								if (SDL_SetAlpha (screen, SDL_SRCALPHA, 0) < 0)
								{
									fprintf (stderr, "Can't set alpha channel!\n");
									
									#if __ANDROID__
									/* hide on screen keyboard */
									SDL_ANDROID_ToggleScreenKeyboardWithoutTextInput ();
									#endif
									
									return (1);
								}
									
								screen_reopened = 1;
							}
						}
					}
					break;

				case SDL_KEYDOWN:
					key = event.key.keysym.sym;
					switch (key)
					{
						case SDLK_BACKSPACE:
							printf ("BACKSPACE\n");

							ip_len = strlen (ip_str);
							if (ip_len > 0)
							{
								ip_str[ip_len - 1] = '\0';
							}
							break;
						
						case SDLK_RETURN:
							wait = FALSE;
							break;
						
						default:
							ip_len = strlen (ip_str);
							if (ip_len < 256)
							{
								ip_str[ip_len] = event.key.keysym.unicode;
								ip_str[ip_len + 1] = '\0';
							}
							break;
					}
					break;
			}
		}
		
		strcpy (request, "enter server ip: ");
		strcat (request, ip_str);
		
		boxRGBA (screen, 0, 0, screen_width - 1, screen_height - 1, 0, 0, 0, 255);
		draw_text_ttf (hud_font, request, 50, 50, 255, 255, 255);
		SDL_UpdateRect (screen, 0, 0, 0, 0);
		SDL_Flip (screen);
	}
	
	#if __ANDROID__
	/* hide on screen keyboard */
	SDL_ANDROID_ToggleScreenKeyboardWithoutTextInput ();
	#endif
	
	return (0);
}		

							 
int main (int ac, char *av[])
{
	const SDL_VideoInfo *info;
	/* Initialize the SDL library */

    /* Same setup as before */
    int audio_rate = 44100;
    Uint16 audio_format = AUDIO_S16; 
    int audio_channels = 2;
    int audio_buffers = 4096;
	
	Sint16 screen_reopened;
	int ret;
	
	Uint8 debug[256];
	
	Uint8 buffer[512];
	Sint32 command, data, bytes;

	LOGD("initializing SDL...");
	printf ("Initializing SDL.\n");   
    
	#if SOUND_ON
    if (SDL_Init (SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK) < 0)
    {
        fprintf (stderr, "Couldn't initialize SDL: %s\n", SDL_GetError ());
		strcpy (debug, SDL_GetError ());
		LOGE("%s\n", debug);
        exit (EXIT_FAILURE);
    }
    
    if (Mix_OpenAudio (audio_rate, audio_format, audio_channels, audio_buffers) != 0)
    {
        fprintf (stderr, "SDL Unable to open audio!\n");
		strcpy (debug, SDL_GetError ());
		LOGE("%s\n", debug);
        exit (EXIT_FAILURE);
    }
    
	#else
    if (SDL_Init (SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) < 0)
    {
        fprintf (stderr, "Couldn't initialize SDL: %s\n", SDL_GetError ());
		strcpy (debug, SDL_GetError ());
		LOGE("%s\n", debug);
        exit (EXIT_FAILURE);
    }
	#endif

	/* Initialize the TTF library */

    if (TTF_Init () < 0)
    {
        fprintf (stderr, "Couldn't initialize TTF: %s\n", SDL_GetError ());
		strcpy (debug, SDL_GetError ());
		LOGE("%s\n", debug);
        exit (1);
    }
    
    /* Clean up on exit */
    atexit (SDL_Quit);

	info = SDL_GetVideoInfo();
    if (info->vfmt->BitsPerPixel > 8)
    {
        video_bpp = info->vfmt->BitsPerPixel;
    }
    else
    {
        video_bpp = 16;
    }

    #if __ANDROID__
		screen_width = info->current_w;
		screen_height = info->current_h;
	
		screen = SDL_SetVideoMode (screen_width, screen_height, video_bpp, SDL_SWSURFACE | SDL_SRCALPHA | SDL_RESIZABLE);
	#else
		screen = SDL_SetVideoMode (screen_width, screen_height, video_bpp, SDL_SWSURFACE | SDL_SRCALPHA | SDL_RESIZABLE);
    #endif
	if (screen == NULL)
    {
        fprintf (stderr, "Couldn't set %i x %i x %i video mode: %s\n", screen_width, screen_height, 24, SDL_GetError ());
        exit (1);
	}
    
    /* Use alpha blending */
    if (SDL_SetAlpha (screen, SDL_SRCALPHA, 0) < 0)
    {
        fprintf (stderr, "Can't set alpha channel!\n");
        exit (1);
    }
  
   	SDL_WM_SetCaption ("battle0", "battle0");
	
	hud_font = TTF_OpenFont (FONT_FREEMONO, 16);
    if (hud_font == NULL)
    {
        printf ("load_font_ttf: can't load font fonts/FreeMono.ttf\n");
		LOGE("can't load fonts/FreeMono.ttf!");
        exit (EXIT_FAILURE);
    }
	
	/* key input settings */

    SDL_EnableUNICODE (SDL_ENABLE);
    SDL_EnableKeyRepeat (500, 125);
	
	LOGD("screen opened");
	

	/* initialize networking */
	if (SDLNet_Init () < 0)
	{
		fprintf (stderr, "SDLNet_Init: %s\n", SDLNet_GetError ());
		strcpy (debug, SDLNet_GetError ());
		LOGE("%s\n", debug);
		exit (EXIT_FAILURE);
	}
	
	#if __ANDROID__
	
	get_server_ip ();
	
	/* Resolve the host we are connecting to */
	if (SDLNet_ResolveHost (&ip, ip_str, 2010) < 0)
	{
		fprintf (stderr, "SDLNet_ResolveHost: %s\n", SDLNet_GetError ());
	    strcpy (debug, SDLNet_GetError ());
		LOGE("%s\n", debug);
		exit (EXIT_FAILURE);
	}
	
	#else
	
	if (ac < 2)
	{
		/* no server ip set -> ask user on screen */
		
		get_server_ip ();
	}
	else
	{
		strcpy (ip_str, av[1]);
	}
	
	/* Resolve the host we are connecting to */
	if (SDLNet_ResolveHost (&ip, ip_str, 2010) < 0)
	{
		fprintf (stderr, "SDLNet_ResolveHost: %s\n", SDLNet_GetError ());
		strcpy (debug, SDLNet_GetError ());
		LOGE("%s\n", debug);
		exit (EXIT_FAILURE);
	}
 
	#endif
 
	/* Open a connection with the IP provided (listen on the host's port) */
	if (!(sd = SDLNet_TCP_Open (&ip)))
	{
		fprintf (stderr, "SDLNet_TCP_Open: %s\n", SDLNet_GetError ());
		strcpy (debug, SDLNet_GetError ());
		LOGE("%s\n", debug);
		exit (EXIT_FAILURE);
	}
  
	LOGD("SDL OK");
    printf ("SDL initialized.\n");
    
	draw_text_ttf (hud_font, "loading graphics, please wait...", 50, 100, 255, 255, 255);
	
	LOGD("loading graphics...");
	
	load_ground_bitmaps ();
	
	
	
	LOGD("connecting to server...");
	
	draw_text_ttf (hud_font, "connecting to server, please wait...", 50, 200, 255, 255, 255);
	
	/* get player number from server */
	
	command = GET_PLAYER_NUMBER;
	
	if (send_data (sd, &command, sizeof (command), BYTEORDER_NET) != 0)
	{
		exit (EXIT_FAILURE);
	}
 
	if (recv_data (sd, &data, sizeof (data), BYTEORDER_NET) == 0)
	{
		/* get player number */
		
		player_number = data;
		
		printf ("this is player: %li\n", player_number);
	}
	
	SDLNet_TCP_Close (sd);
	
	/* get world data from server */
	
	/* Open a connection with the IP provided (listen on the host's port) */
	if (!(sd = SDLNet_TCP_Open (&ip)))
	{
		fprintf (stderr, "SDLNet_TCP_Open: %s\n", SDLNet_GetError ());
		exit (EXIT_FAILURE);
	}
	
	command = GET_WORLD;
	
	if (send_data (sd, &command, sizeof (command), BYTEORDER_NET) != 0)
	{
		exit (EXIT_FAILURE);
	}
	
	if (send_data (sd, &player_number, sizeof (player_number), BYTEORDER_NET) != 0)
	{
		exit (EXIT_FAILURE);
	}
	
	LOGD("receiving world...");
	
	printf ("receiving world...\n");
	
	draw_text_ttf (hud_font, "receiving world, please wait...", 50, 250, 255, 255, 255);
	
	recv_world (sd);
	
	SDLNet_TCP_Close (sd);
	
	draw_world (zoom);
	
// wait for "q" key	to EXIT
	SDL_Event event;

	SDL_Joystick *joystick;

    SDL_JoystickEventState (SDL_ENABLE);
    joystick = SDL_JoystickOpen (0);
	
wait:
	SDL_Delay (50);
	while (SDL_PollEvent (&event))
    {
        switch (event.type)
        {
			screen_reopened = 0;
			case SDL_ACTIVEEVENT:
				while (screen_reopened == 0)
				{
					while (SDL_PollEvent (&event))
					{
						if (event.type == SDL_VIDEORESIZE)
						{
							//If the window was iconified or restored
				
							screen = SDL_SetVideoMode (event.resize.w, event.resize.h, video_bpp, SDL_SWSURFACE | SDL_SRCALPHA | SDL_RESIZABLE);
							if (screen == NULL)
							{
								fprintf (stderr, "Couldn't set %i x %i x %i video mode: %s\n", screen_width, screen_height, 24, SDL_GetError ());
								return (1);
							}

							/* Use alpha blending */
							if (SDL_SetAlpha (screen, SDL_SRCALPHA, 0) < 0)
							{
								fprintf (stderr, "Can't set alpha channel!\n");
								return (1);
							}
									
							screen_reopened = 1;
						}
					}
				}
				break;
			
			case SDL_KEYDOWN:
                switch (event.key.keysym.sym)
                {
					case SDLK_d:
						receive_units ();
						draw_world (zoom);
						break;
					
					case SDLK_i:
						user_unit_info (zoom);
						break;
					
					case SDLK_m:
						user_move_unit (zoom);
						break;
						
                     case SDLK_q:
						SDLNet_Quit ();
                        exit (0);
                        break;
					
					 case SDLK_SPACE:
						 moves_finished ();
						 
						 receive_units ();
						 draw_world (zoom);
						 break;
						 
					 case SDLK_t:
						 user_place_tank (zoom);
						 break;
						
					 case SDLK_KP_PLUS:
					 case SDLK_x:
						 zoom = 2;
						 draw_world (zoom);
						 break;
						 
					case SDLK_KP_MINUS:
					case SDLK_y:
						 zoom = 1;
						 draw_world (zoom);
						 break;	 
						 
					 case SDLK_DOWN:
						 world_up_y++;
						 draw_world (zoom);
						 break;
						 
					 case SDLK_UP:
						 world_up_y--;
						 draw_world (zoom);
						 break;
						 
					case SDLK_LEFT:
						 world_up_x--;
						 draw_world (zoom);
						 break;
						 
					 case SDLK_RIGHT:
						 world_up_x++;
						 draw_world (zoom);
						 break;
				}
				break;

			case SDL_MOUSEBUTTONDOWN:
				user_unit_info (zoom);
				break;
				
			
				
			case SDL_JOYAXISMOTION:  /* Handle Joystick Motion */
				printf ("JOYSTICK MOTION\n");
				if ( ( event.jaxis.value < -3200 ) || (event.jaxis.value > 3200 ) ) 
				{
					if (event.jaxis.axis == 0) 
					{
						/* Left-right movement code goes here */
						/* left */
								
						if (event.jaxis.value < -26213)
						{
							world_up_x -= 10;
							draw_world (zoom);
							break;
						}
								
						if (event.jaxis.value < -19660)
						{
							world_up_x -= 8;
							draw_world (zoom);
							break;
						}
								
						if (event.jaxis.value < -13106)
						{
							world_up_x -= 6;
							draw_world (zoom);
							break;
						}
								
						if (event.jaxis.value < -6553)
						{
							world_up_x -= 4;
							draw_world (zoom);
							break;
						}
								
						if (event.jaxis.value < -3200)
						{
									
							world_up_x -= 2;
							draw_world (zoom);
							break;
						}
								
#if __ANDROID__
						if (event.jaxis.value < 0)
						{
									
							world_up_x -= 2;
							draw_world (zoom);
							break;
						}
#endif


						/* right */
								
						if (event.jaxis.value > 26213)
						{
							world_up_x += 10;
							draw_world (zoom);
							break;
						}
								
						if (event.jaxis.value > 19660)
						{
							world_up_x += 8;
							draw_world (zoom);
							break;
						}
								
						if (event.jaxis.value > 13106)
						{
							world_up_x += 6;
							draw_world (zoom);
							break;
						}
								
						if (event.jaxis.value > 6553)
						{
							world_up_x += 4;
							draw_world (zoom);
							break;
						}
								
						if (event.jaxis.value > 3200)
						{
							world_up_x += 2;
							draw_world (zoom);
							break;
						}
								
#if __ANDROID__
						if (event.jaxis.value > 0)
						{
									
							world_up_x += 2;
							draw_world (zoom);
							break;
						}
#endif
								
					}

					if (event.jaxis.axis == 1) 
					{
						/* Up-Down movement code goes here */
						/* up */
								
						if (event.jaxis.value < -26213)
						{
							world_up_y -= 10;
							draw_world (zoom);
							break;
						}
								
						if (event.jaxis.value < -19660)
						{
							world_up_y -= 8;
							draw_world (zoom);
							break;
						}
								
						if (event.jaxis.value < -13106)
						{
							world_up_y -= 6;
							draw_world (zoom);
							break;
						}
								
						if (event.jaxis.value < -6553)
						{
							world_up_y -= 4;
							draw_world (zoom);
							break;
						}
								
						if (event.jaxis.value < -3200)
						{		
							world_up_y -= 2;
							draw_world (zoom);
							break;
						}
								
#if __ANDROID__
						if (event.jaxis.value < 0)
						{
									
							world_up_y -= 2;
							draw_world (zoom);
							break;
						}
#endif
								
								
						/* down */
								
						if (event.jaxis.value > 26213)
						{
							world_up_y += 10;
							draw_world (zoom);
							break;
						}
								
						if (event.jaxis.value > 19660)
						{
							world_up_y += 8;
							draw_world (zoom);
							break;
						}
								
						if (event.jaxis.value > 13106)
						{
							world_up_y += 6;
							draw_world (zoom);
							break;
						}
								
						if (event.jaxis.value > 6553)
						{
							world_up_y += 4;
							draw_world (zoom);
							break;
						}
								
						if (event.jaxis.value > 3200)
						{
							world_up_y += 2;
							draw_world (zoom);
							break;
						}
								
#if __ANDROID__
						if (event.jaxis.value > 0)
						{
									
							world_up_y += 2;
							draw_world (zoom);
							break;
						}
#endif
					}
				}
				break;
				
			default:
				break;
		}
	}
	goto wait;
}
	