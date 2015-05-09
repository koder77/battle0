// Battle0 - server V 0.4

#include "global_defs.h"
#include "network.h"

/* unit move times from 0 = DESERT to 6 = WATER */
/* -1 = no move to this field possible */
Sint16 move_times_tank[7] =
{
	1, 2, 2, 1, -1, 2, -1
};

/* game map picture */
SDL_Surface *world_bmp;

/* game map */
Uint8 world[WORLD_HEIGHT][WORLD_WIDTH];
Sint32 world_seed;

/* units map */
struct unit unit[WORLD_HEIGHT][WORLD_WIDTH];

struct player player[MAX_PLAYERS];
Sint32 player_ind = -1;				/* < 0 == no player connected */
Sint32 active_player = 0;

Uint8 play_token;

TCPsocket sd, csd; /* Socket descriptor, Client socket descriptor */
IPaddress ip, *remoteIP;

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

Sint16 load_random_world ()
{
	Sint16 wx, wy, i, j, max_base = 5;
	int random;
	
	world_seed = time (NULL);
	srand (world_seed);
	
	for (wy = 0; wy < WORLD_HEIGHT; wy++)
	{
		for (wx = 0; wx < WORLD_WIDTH; wx++)
		{
			random = randint (15);
			
			switch (random)
			{
				case 0:
				case 1:
				case 2:
				case 3:
				case 4:
				case 5:
				case 6:
				case 7:
				case 8:
					world[wy][wx] = GRASS;
					break;
					
				case 9:
				case 10:
				case 11:
					world[wy][wx] = FORREST;
					break;

				case 12:
				case 13:
					world[wy][wx] = MOUNTAIN;
					break;
					
				case 14:
				case 15:
					world[wy][wx] = WATER;
					break;
			}
					
			unit[wy][wx].type = EMPTY;
		}
	}
	
	/* place bases */
	wy = 0;
	for (i = 1; i <= max_base; i++)
	{
		wx = 0;
		
		for (j = 1; j <= max_base; j++)
		{
			world[wy][wx] = BASE_RED;
			wx++;
		}
		wy++;
	}
	
	wy = 0;
	for (i = 1; i <= max_base; i++)
	{
		wx = WORLD_WIDTH - max_base - 1;
		
		for (j = 1; j <= max_base; j++)
		{
			world[wy][wx] = BASE_BLUE;
			wx++;
		}
		wy++;
	}
	
	wy = WORLD_HEIGHT - max_base - 1;
	for (i = 1; i <= max_base; i++)
	{
		wx = WORLD_WIDTH - max_base - 1;
		
		for (j = 1; j <= max_base; j++)
		{
			world[wy][wx] = BASE_YELLOW;
			wx++;
		}
		wy++;
	}
	
	wy = WORLD_HEIGHT - max_base - 1;
	for (i = 1; i <= max_base; i++)
	{
		wx = 0;
		
		for (j = 1; j <= max_base; j++)
		{
			world[wy][wx] = BASE_GREEN;
			wx++;
		}
		wy++;
	}
}

Sint16 load_world_bmp (unsigned char *worldbmp)
{
	Sint16 wx, wy;
	Uint32 pixel;
	Uint8 r, g, b;
	
	unsigned char worldname[256];
	
	strcpy (worldname, "data/gfx/");
	strcat (worldname, worldbmp);
	
	world_bmp = IMG_Load (worldname);
	if (! world_bmp)
	{
		printf ("ERROR: can't load world bmp!\n");
		return (1);
	}
	
	for (wy = 0; wy < WORLD_HEIGHT; wy++)
	{
		for (wx = 0; wx < WORLD_WIDTH; wx++)
		{
			if (SDL_LockSurface (world_bmp) < 0)
            {
                printf ("load_world: can't lock source surface!\n");
                return (1);
            }

            pixel = getpixel (world_bmp, wx, wy);
            SDL_UnlockSurface (world_bmp);
            SDL_GetRGB (pixel, world_bmp->format, &r, &g, &b);
			
			/* fields */
			if (r == 215 && g == 220 && b == 35)
			{
				world[wy][wx] = DESERT;
			}
			
			if (r == 68 && g == 69 && b == 32)
			{
				world[wy][wx] = DIRT;
			}
	
			if (r == 36 && g == 98 && b == 35)
			{
				world[wy][wx] = FORREST;
			}
			
			if (r == 34 && g == 220 && b == 33)
			{
				world[wy][wx] = GRASS;
			}
			
			if (r == 218 && g == 213 && b == 187)
			{
				world[wy][wx] = MOUNTAIN;
			}
			
			if (r == 255 && g == 255 && b == 255)
			{
				world[wy][wx] = SNOW;
			}
			
			if (r == 47 && g == 119 && b == 212)
			{
				world[wy][wx] = WATER;
			}
			
			/* bases */
			if (r == 255 && g == 0 && b == 0)
			{
				world[wy][wx] = BASE_RED;
			}
			
			if (r == 00 && g == 0 && b == 255)
			{
				world[wy][wx] = BASE_BLUE;
			}
			
			if (r == 190 && g == 246 && b == 6)
			{
				world[wy][wx] = BASE_YELLOW;
			}
			
			if (r == 0 && g == 255 && b == 0)
			{
				world[wy][wx] = BASE_GREEN;
			}
			
			unit[wy][wx].type = EMPTY;
		}
	}
	
	return (0);
}
			
Sint16 set_tank_server (Sint32 color, Sint16 x, Sint16 y)
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
	tank->aim_angle = 0;
	tank->aim_x = -1;
	tank->aim_y = -1;
	tank->fire = 0;
	tank->ap = AP_TANK_MAX;
	tank->max_ap = AP_TANK_MAX;
	tank->move_x = -1;		/* no move < 0 */
	tank->move_y = -1;
	tank->timer_move = 0;
	tank->in_move = 0;
	tank->timer_ready = 1;
	tank->timer_reload = 3;
	tank->ccs = 100;
	tank->hull = 100;
	tank->motor = 100;
	tank->cannon = 100;
	tank->give_up = 0;
	
	return (0);
}
			
Sint16 calculate_data (void)
{
	Sint16 wx, wy, j;
	Uint8 buf[4];
	Sint32 data;
	int random;
	
	struct tank *tank;
	struct tank *target_tank;
	
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
						
						if (tank->in_move == 0 && tank->move_x > -1 && tank->move_y > -1)
						{
							tank->timer_move = move_times_tank[world[tank->move_y][tank->move_x]];
							tank->in_move = 1;
							printf ("tank move timer set to: %li\n", tank->timer_move);
						}
						
						if (tank->move_x > -1 && tank->move_y > -1)
						{
							printf ("tank move timer: %li\n", tank->timer_move);
							if (tank->timer_move == 0)
							{
								/* move unit to new field */
								
								printf ("tank moved\n");
								
								unit[tank->move_y][tank->move_x].data = (struct tank *) malloc (sizeof (struct tank));
								if (unit[tank->move_y][tank->move_x].data == NULL)
								{
									printf ("ERROR calculate_data no memory for unit!\n");
									return (1);
								}
								
								/*
								if (memcpy (&unit[tank->move_y][tank->move_y].data, &unit[wy][wx].data, sizeof (struct tank)) == NULL)
								{
									printf ("ERROR calculate_data memcpy()\n");
									return (1);
								}
								*/
								
								unit[tank->move_y][tank->move_x].data = unit[wy][wx].data;
								unit[tank->move_y][tank->move_x].type = TANK;
								unit[tank->move_y][tank->move_x].color = unit[wy][wx].color;
								
								tank->move_x = -1; tank->move_y = -1;
								tank->timer_move = 0;
								tank->in_move = 0;
								
								/* set old unit field empty */
								/* free (unit[wy][wx].data); */
								unit[wy][wx].type = EMPTY;
							}
							else
							{
								tank->timer_move--;
							}	
						}
						
						if (tank->aim_x > -1 && tank->aim_y > -1 && tank->fire == 1)
						{
							/* check if fire is possible */
							if ((tank->aim_x >= wx - TANK_FIRE_RAD && tank->aim_x <= wx + TANK_FIRE_RAD) && (tank->aim_y >= wy - TANK_FIRE_RAD && tank->aim_y <= wy + TANK_FIRE_RAD))
							{
								if (unit[tank->aim_y][tank->aim_x].type != EMPTY && unit[tank->aim_y][tank->aim_x].color != active_player)
								{
									/* fire is possible, calculate damage */
									target_tank = (struct tank *) unit[tank->aim_y][tank->aim_x].data;
									
									random = randint (7);
									switch (random)
									{
										case 0:
											/* hull damage direct hit */
											target_tank->hull -= ((tank->cannon / 100) + (tank->ccs / 100)) / 2 * 15;
											break;
											
										case 1:
											/* ccs direct hit */
											target_tank->ccs -= ((tank->cannon / 100) + (tank->ccs / 100)) / 2 * 15;
											break;
											
										case 2:
											/* engine direct hit */
											target_tank->motor -= ((tank->cannon / 100) + (tank->ccs / 100)) / 2 * 15;
											break;
											
										case 3:
											/* cannon direct hit */
											target_tank->cannon -= ((tank->cannon / 100) + (tank->ccs / 100)) / 2 * 15;
											break;
											
										default:
											break;
									}
									
									target_tank->health -= ((tank->cannon / 100) + (tank->ccs / 100)) / 2 * 10;
									if (random != 0)
									{
										target_tank->hull -= ((tank->cannon / 100) + (tank->ccs / 100)) / 2 * 10;
									}
									
									tank->fire = 0;		/* reset fire */
									
									printf ("TANK FIRED\n");
								}
							}
						}
						
						tank->ap = AP_TANK_MAX;		/* reset APs */
						
						break;
				}
			}
		}
	}
	
	/* set active player */
	active_player++;
	if (active_player > player_ind)
	{
		active_player = 0;
	}
	
	return (0);
}
			
int main (int ac, char *av[])
{
	Uint8 buffer[512];
	Sint32 command, data, player;
	Sint16 worldx, worldy;
	
	Sint16 run = 1;
	
	printf ("battle0 - server 0.1 running...\n");
	
	if (ac == 2)
	{
		if (strcmp (av[1], "-random") == 0)
		{
			load_random_world ();
			printf ("using random map.\n");
		}
		else
		{
			if (load_world_bmp (av[1]) != 0)
			{
				printf ("load_world_bmp: ERROR can't load world bmp!\n");
				exit (1);
			}
		}
	}
	else
	{
		printf ("battle0-server -random | world-file\n");
		exit (1);
	}

	if (SDLNet_Init () < 0)
	{
		fprintf (stderr, "SDLNet_Init: %s\n", SDLNet_GetError ());
		exit (EXIT_FAILURE);
	}
 
	/* Resolving the host using NULL make network interface to listen */
	if (SDLNet_ResolveHost (&ip, NULL, 2010) < 0)
	{
		fprintf (stderr, "SDLNet_ResolveHost: %s\n", SDLNet_GetError ());
		exit (EXIT_FAILURE);
	}
 
	/* Open a connection with the IP provided (listen on the host's port) */
	if (!(sd = SDLNet_TCP_Open (&ip)))
	{
		fprintf (stderr, "SDLNet_TCP_Open: %s\n", SDLNet_GetError ());
		exit (EXIT_FAILURE);
	}
	
	while (run)
	{
		/* This check the sd if there is a pending connection.
		* If there is one, accept that, and open a new socket for communicating */
		SDL_Delay (100);
		
		if ((csd = SDLNet_TCP_Accept (sd)))
		{
			/* Now we can communicate with the client using csd socket
			* sd will remain opened waiting other connections */
 
			/* Get the remote address */
			if ((remoteIP = SDLNet_TCP_GetPeerAddress (csd)))
			{
				/* Print the address, converting in the host format */
				// printf ("Host connected: %x %d\n", SDLNet_Read32 (&remoteIP->host), SDLNet_Read16 (&remoteIP->port));
			}
			else
			{	
				// fprintf (stderr, "SDLNet_TCP_GetPeerAddress: %s\n", SDLNet_GetError ());
			}
			
			if (recv_data (csd, &command, sizeof (command), BYTEORDER_NET) == 0)
			{
				switch (command)
				{
					case GET_PLAYER_NUMBER:
						if (player_ind < MAX_PLAYERS - 1)
						{
							player_ind++;
						}
						
						data = player_ind;
						
						if (send_data (csd, &data, sizeof (data), BYTEORDER_NET) != 0)
						{
							exit (EXIT_FAILURE);
						}
						
						printf ("player: %li requested player number.\n", player_ind);
						break;
						
					case GET_WORLD:
						printf ("get world request...\n");
						if (recv_data (csd, &player, sizeof (player), BYTEORDER_NET) == 0)
						{
							printf ("player: %li requested world data.\n", player);
							
							send_world (csd);
						}
						break;
						
					case SET_UNIT_TANK:
						printf ("set unit tank request...\n");
						if (recv_data (csd, &player, sizeof (player), BYTEORDER_NET) == 0)
						{
							printf ("player: %li request.\n", player);
						}
						
						if (recv_data (csd, &worldx, sizeof (worldx), BYTEORDER_NET) == 0)
						{
							printf ("worldx: %li\n", worldx);
						}
							
						if (recv_data (csd, &worldy, sizeof (worldy), BYTEORDER_NET) == 0)
						{
							printf ("worldy: %li\n", worldy);
						}	
							
						if (set_tank_server (player, worldx, worldy) == 0)
						{
							data = OK;
						}
						else
						{
							data = ERROR;
						}
						
						if (send_data (csd, &data, sizeof (data), BYTEORDER_NET) != 0)
						{
							printf ("ERROR sending ACK!\n");
						}
						break;
						
					case RECEIVE_UNITS:
						if (send_units (csd) != 0)
						{
							printf ("ERROR send_units!\n");
						}
						break;
					
					case GET_PLAY_TOKEN:
						// printf ("get play token request...\n");
						if (recv_data (csd, &player, sizeof (player), BYTEORDER_NET) == 0)
						{
							// printf ("player: %li request.\n", player);
						}
						
						if (player == active_player)
						{
							data = OK;
						}
						else
						{
							data = ERROR;
						}
						
						if (send_data (csd, &data, sizeof (data), BYTEORDER_NET) != 0)
						{
							printf ("ERROR sending ACK!\n");
						}
						break;
						
					case SEND_TANK_MOVE:
						printf ("tank move...\n");
						if (recv_unit_tank_move (csd) != 0)
						{
							printf ("ERROR recv_tank_move!\n");
						}
						else
						{
							printf ("tank move OK\n");
						}
						break;
					
					case SEND_TANK_FIRE:
						printf ("tank fire...\n");
						if (recv_unit_tank_fire (csd) != 0)
						{
							printf ("ERROR recv_tank_fire!\n");
						}
						else
						{
							printf ("tank fire OK\n");
						}
						break;
						
					case USER_MOVE_END:
						if (recv_data (csd, &player, sizeof (player), BYTEORDER_NET) == 0)
						{
							printf ("player: %li request move end.\n", player);
						}
						
						calculate_data ();	/* dummy call for debugging */
						break;
				}
				
			}
			/* Close the client socket */
			SDLNet_TCP_Close (csd);
		}
	}
	
	SDLNet_TCP_Close (sd);
	SDLNet_Quit ();
 
	return (EXIT_SUCCESS);
}
	