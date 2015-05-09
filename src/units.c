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


extern SDL_Surface *screen;
extern SDL_Surface *screen_copy;
extern SDL_Surface *ground[11];

extern TTF_Font *hud_font;

extern Uint8 world[WORLD_HEIGHT][WORLD_WIDTH];
extern struct unit unit[WORLD_HEIGHT][WORLD_WIDTH];
extern struct unit_color unit_color[4];

extern Sint16 move_times_tank[7];

extern Sint16 screen_width;
extern Sint16 screen_height;
extern Uint8 video_bpp;
extern Sint16 world_up_x;
extern Sint16 world_up_y;
extern Uint8 play_token;
extern Sint32 player_number;
extern Sint16 unit_info_wx;
extern Sint16 unit_info_wy;
extern TCPsocket sd;
extern TCPsocket csd;
extern IPaddress ip;

Sint16 user_fire_unit (Sint16 zoom)
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
			
			if ((worldx >= unit_info_wx - TANK_FIRE_RAD && worldx <= unit_info_wx + TANK_FIRE_RAD) && (worldy >= unit_info_wy - TANK_FIRE_RAD && worldy <= unit_info_wy + TANK_FIRE_RAD))
			{
				if (unit[worldy][worldx].type != EMPTY && unit[worldy][worldx].color != player_number)
				{
					/* enemy tank selected check APs */
					if (tank->move_x > -1 && tank->move_y > -1)
					{
						/* cancel move, reset AP */
						
						tank->move_x = -1; tank->move_y = -1;
						tank->ap += AP_TANK_MOVE;
					}
					
					if (tank->aim_x != worldx && tank->aim_y != worldy)
					{
						if (tank->ap >= AP_TANK_AIM + AP_TANK_FIRE) tank->ap -= (AP_TANK_AIM + AP_TANK_FIRE);
					}
					else
					{
						if (tank->ap >= AP_TANK_FIRE) 
						{
							tank->ap -= AP_TANK_FIRE;
					
							tank->aim_x = worldx; tank->aim_y = worldy; tank->fire = TRUE;
					
					
							/* set cannon angle on 90 degree axis */
							if (unit_info_wy == worldy)
							{
								if (unit_info_wx > worldx)
								{
									tank->aim_angle = 90;
								}
								else
								{
									tank->aim_angle = 270;
								}
							}
					
							if (unit_info_wx == worldx)
							{
								if (unit_info_wy > worldy)
								{
									tank->aim_angle = 0;
								}
								else
								{
									tank->aim_angle = 180;
								}
							}
					
							/* set cannon angle on 45 degree edge axis, rough direction only */
							if (worldx != unit_info_wx && worldy != unit_info_wy)
							{
								if (worldx < unit_info_wx && worldy < unit_info_wy)
								{
									tank->aim_angle = 45;
								}
						
								if (worldx > unit_info_wx && worldy < unit_info_wy)
								{
									tank->aim_angle = 315;
								}
						
								if (worldx > unit_info_wx && worldy > unit_info_wy)
								{
									tank->aim_angle = 225;
								}
						
								if (worldx < unit_info_wx && worldy > unit_info_wy)
								{
									tank->aim_angle = 135;
								}
							}
					
							draw_world (zoom);
							draw_shot_anim (unit_info_wx, unit_info_wy, worldx, worldy, TILE_WIDTH * zoom);
							draw_world (zoom);
					
							printf ("TANK FIRE!\n");
						}
					}
				}
			}
			break;
	}
	return (0);
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
				if (unit[worldy][worldx].type == EMPTY && tank->timer_move == 0 && tank->ap >= AP_TANK_MOVE)
				{
					if (move_times_tank[world[worldy][worldx]] != EMPTY)
					{
						/* set move one field forward mark */
						tank->move_x = worldx; tank->move_y = worldy;
						tank->ap -= AP_TANK_MOVE;
						draw_world (zoom);
					}
				}
			}
			else
			{
				/* fire? */
				user_fire_unit (zoom);
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
	Uint8 key;
	
	printf ("unit info\n");
	
	get_mouse_state (&mx, &my, &button, 1);
		
	printf ("mouse: %i / %i\n", mx, my);
		
	worldx = world_up_x + mx / (TILE_WIDTH * zoom);
	worldy = world_up_y + my / (TILE_WIDTH * zoom);
		
	printf ("world: %i / %i\n", worldx, worldy);	
	

	/* select own unit, check move */
	
	if (unit[worldy][worldx].color != player_number)
	{
		user_fire_unit (zoom);		/* enemy unit -> go to fire mode */
	}
	else
	{
		unit_info_wx = worldx; unit_info_wy = worldy;
		
		draw_world (zoom);
	
		button = 1;
		while (button == -1 || button > 0)
		{
			get_mouse_state (&mx, &my, &button, 1);
			get_key_state (&key);
		
			if (key == 'f') user_fire_unit (zoom);
		
			worldx = world_up_x + mx / (TILE_WIDTH * zoom);
			worldy = world_up_y + my / (TILE_WIDTH * zoom);
		
			printf ("button: %li\n", button);
			printf ("worldx: %li / unit_info_wx: %li, worldy: %li / unit_info_wy: %li\n", worldx, unit_info_wx, worldy, unit_info_wy); 
		
		
			if ((worldx != unit_info_wx || worldy != unit_info_wy) || (worldx != unit_info_wx && worldy != unit_info_wy))
			{			
				/* user wants touch move */
			
				printf ("TOUCH MOVE\n");
			
				if (play_token == TRUE) user_move_unit (zoom);
				button = 0;
			}
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
	
	if (world[worldy][worldx] == WATER)
	{
		/* can't set tank on water */
		printf ("field is water!\n");
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
	tank->aim_x = -1;
	tank->aim_y = -1;
	tank->fire = 0;
	tank->ap = AP_TANK_MAX;
	tank->max_ap = AP_TANK_MAX;
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
			if (unit[wy][wx].type != EMPTY && unit[wy][wx].color == player_number)
			{
				switch (unit[wy][wx].type)
				{
					case TANK:
						tank = (struct tank *) unit[wy][wx].data;
						
						if (tank->move_x > -1 && tank->move_y > -1 && tank->in_move == 0)
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
						
						if (tank->aim_x > -1 && tank->aim_y > -1 && tank->fire == TRUE)
						{
							/* Open a connection with the IP provided (listen on the host's port) */
							if (!(sd = SDLNet_TCP_Open (&ip)))
							{
								fprintf (stderr, "SDLNet_TCP_Open: %s\n", SDLNet_GetError ());
								exit (EXIT_FAILURE);
							}
							/* tank got move command, send data... */
							printf ("send unit tank fire\n");
							
							if (send_unit_tank_fire (sd, wx, wy) != 0)
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
