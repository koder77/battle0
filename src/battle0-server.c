// Battle0 - server V 0.7

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
Uint32 world_height = WORLD_HEIGHT, world_width = WORLD_WIDTH;		/* init world size */

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

void init_player_hashes ()
{
	int p, i, rand;
	Uint8 *ptr;
	Sint32 seed;

	/* start random number generator */
	seed = time (NULL);
	srand (seed);

	/* set hashes for all 4 players */

	for (p = 0; p < 4; p++)
	{
		ptr = (Uint8 *) &player[p].hash;

		for (i = 0; i < 4; i++)
		{
			rand = randint (255);
			*ptr++ = rand;
		}

		player[p].active = 1;
		
		printf ("init_player_hashes: player %i: hash: %li\n", p, player[p].hash);
	}
}



Sint16 load_random_world ()
{
	Sint16 wx, wy, i, j, max_base = 5;
	int random;

	world_seed = time (NULL);
	srand (world_seed);

	for (wy = 0; wy < world_height; wy++)
	{
		for (wx = 0; wx < world_width; wx++)
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
					world[wy][wx] = GRASS;
					break;
					
				case 7:
				case 8:
					world[wy][wx] = DIRT;
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
		wx = world_width - max_base - 64;

		for (j = 1; j <= max_base; j++)
		{
			world[wy][wx] = BASE_BLUE;
			wx++;
		}
		wy++;
	}

	wy = world_height - max_base - 64;
	for (i = 1; i <= max_base; i++)
	{
		wx = world_width - max_base - 64;

		for (j = 1; j <= max_base; j++)
		{
			world[wy][wx] = BASE_YELLOW;
			wx++;
		}
		wy++;
	}

	wy = world_height - max_base - 64;
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

	strcpy (worldname, "data/gfx/world/");
	
	if (world_width == 128)
	{
		strcat (worldname, "128/");
	}
		
	if (world_width == 256)
	{
		strcat (worldname, "256/");
	}
	
	if (world_width == 512)
	{
		strcat (worldname, "512/");
	}
	
	strcat (worldname, worldbmp);
	
	world_bmp = IMG_Load (worldname);
	if (! world_bmp)
	{
		printf ("ERROR: can't load world bmp!\n");
		return (1);
	}

	for (wy = 0; wy < world_height; wy++)
	{
		for (wx = 0; wx < world_width; wx++)
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
	Sint16 i;

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
	tank->timer_move = 0;
	tank->in_move = 0;
	tank->timer_ready = 1;
	tank->timer_reload = 3;
	tank->ccs = 100;
	tank->hull = 100;
	tank->motor = 100;
	tank->cannon = 100;
	tank->give_up = 0;

	/* init moves */

	for (i = 0; i < MAX_MOVES; i++)
	{
		tank->move_path[i][0] = -1;
		tank->move_path[i][1] = -1;
	}

	tank->move_field = 0;

	return (0);
}

Sint16 calculate_data (void)
{
	Sint16 wx, wy, j, i;
	Sint16 movex, movey;
	Uint8 buf[4];
	Sint32 data;
	int random;
	int next_player_ok = 0;

	struct tank *tank;
	struct tank *target_tank;

	for (wy = 0; wy < world_height; wy++)
	{
		for (wx = 0; wx < world_width; wx++)
		{
			if (unit[wy][wx].type != EMPTY)
			{
				unit[wy][wx].modify = 1;
			}
		}
	}


	for (wy = 0; wy < world_height; wy++)
	{
		for (wx = 0; wx < world_width; wx++)
		{
			if (unit[wy][wx].type != EMPTY && unit[wy][wx].modify == 1)
			{
				switch (unit[wy][wx].type)
				{
					case TANK:
						tank = (struct tank *) unit[wy][wx].data;

						movex = tank->move_path[tank->move_field][0];
						movey = tank->move_path[tank->move_field][1];

						unit[wy][wx].modify = 0;

						if (tank->in_move == 0 && movex > -1 && movey > -1)
						{
							tank->timer_move = move_times_tank[world[movey][movex]];
							tank->in_move = 1;
							printf ("tank move timer set to: %li\n", tank->timer_move);
						}

						if (movex > -1 && movey > -1)
						{
							printf ("tank move timer: %li\n", tank->timer_move);
							if (tank->timer_move == 0)
							{
								/* move unit to new field */

								printf ("tank moved\n");

								unit[movey][movex].data = (struct tank *) malloc (sizeof (struct tank));
								if (unit[movey][movex].data == NULL)
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

								unit[movey][movex].data = unit[wy][wx].data;
								unit[movey][movex].type = TANK;
								unit[movey][movex].color = unit[wy][wx].color;
								unit[movey][movex].modify = 0;

								if ((tank->move_path[tank->move_field + 1][0] == -1 && tank->move_path[tank->move_field + 1][1] == -1) || tank->move_field == MAX_MOVES - 1)
								{
									tank->timer_move = 0;
									tank->in_move = 0;
									tank->move_field = 0;

									for (i = 0; i < MAX_MOVES; i++)
									{
										tank->move_path[i][0] = -1; tank->move_path[i][1] = -1;
									}
								}
								else
								{
									if (tank->move_field < MAX_MOVES)
									{
										tank->move_field++;
									}
								}

								/* set old unit field empty */
								/* free (unit[wy][wx].data); */
								unit[wy][wx].type = EMPTY;
							}
							else
							{
								tank->timer_move--;
							}
						}

						if (tank->timer_reload <= TANK_FIRE_RELOAD)
						{
							if (tank->timer_reload > 0) tank->timer_reload--;	/* decrease reload timer: zero = loaded */
						}

						if (tank->aim_x > -1 && tank->aim_y > -1 && tank->fire == 1 && tank->timer_reload == 0)
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
											if (target_tank->hull < 0) target_tank->hull = 0;
											break;

										case 1:
											/* ccs direct hit */
											target_tank->ccs -= ((tank->cannon / 100) + (tank->ccs / 100)) / 2 * 15;
											if (target_tank->ccs < 0) target_tank->ccs = 0;
											break;

										case 2:
											/* engine direct hit */
											target_tank->motor -= ((tank->cannon / 100) + (tank->ccs / 100)) / 2 * 15;
											if (target_tank->motor < 0) target_tank->motor = 0;
											break;

										case 3:
											/* cannon direct hit */
											target_tank->cannon -= ((tank->cannon / 100) + (tank->ccs / 100)) / 2 * 15;
											if (target_tank->cannon < 0) target_tank->cannon = 0;
											break;

										default:
											break;
									}

									target_tank->health -= ((tank->cannon / 100) + (tank->ccs / 100)) / 2 * 10;
									if (target_tank->health < 0) target_tank->health = 0;
									if (random != 0)
									{
										target_tank->hull -= ((tank->cannon / 100) + (tank->ccs / 100)) / 2 * 10;
										if (target_tank->hull < 0) target_tank->hull = 0;
									}

									tank->fire = 0;		/* reset fire */
									tank->timer_reload = TANK_FIRE_RELOAD;

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
	
	while (next_player_ok == 0)
	{
		active_player++;
		
		if (active_player > player_ind)
		{
			active_player = 0;
		}
		
		if (player[active_player].active == 1)
		{
			next_player_ok = 1;
			printf ("calculate_data: next player: %i\n", active_player);
		}
	}
		
	return (0);
}

Sint16 check_base_captured (Sint16 player_number)
{
	Sint16 wx, wy, enemy_units = 0;
	Sint16 base;
	
	switch (player_number)
	{
		case 0:
			/* red */
			base = BASE_RED;
			break;
			
		case 1:
			/* blue */
			base = BASE_BLUE;
			break;
			
		case 2:
			/* yellow */
			base = BASE_YELLOW;
			break;
			
		case 3:
			/* green */
			base = BASE_GREEN;
			break;
	}
	
	for (wy = 0; wy < world_height; wy++)
	{
		for (wx = 0; wx < world_width; wx++)
		{
			if (world[wy][wx] == base)
			{
				if (unit[wy][wx].type == TANK)
				{
					if (unit[wy][wx].color != player_number)
					{
						/* enemy unit on base! */
						enemy_units++;
					}
				}
			}
		}
	}
	
	if (enemy_units >= 4)
	{
		player[player_number].active = 0;
		return (1);	/* at least 4 enemy units on base = base captured. Game over for player! */
	}
	else
	{
		player[player_number].active = 1;
		return (0);	/* player in game */
	}
}

int main (int ac, char *av[])
{
	Uint8 buffer[512];
	Sint32 command, data, playernum, playerhash;
	Sint16 worldx, worldy;

	Sint16 run = 1;

	Uint32 hash;

	printf ("battle0 - server 0.7 running...\n");

	if (ac == 2)
	{
		if (strcmp (av[1], "-random") == 0)
		{
			load_random_world ();
			printf ("using random world.\n");
		}
		else
		{
			if (load_world_bmp (av[1]) != 0)
			{
				printf ("load_world_bmp: ERROR can't load world bmp!\n");
				exit (EXIT_FAILURE);
			}
			else
			{
				printf ("using .bmp world: %s, size %i x %i.\n", av[1], world_width, world_height);
			}
		}
	}
	if (ac == 4)
	{
		if (strcmp (av[1], "-random") == 0)
		{
			world_width = atoi (av[2]);
			world_height = atoi (av[3]);
			
			if (world_width > WORLD_WIDTH)
			{
				printf ("ERROR: world width greater as 512!\n");
			}
			if (world_height > WORLD_HEIGHT)
			{
				printf ("ERROR: world height greater as 512!\n");
			}
		
			if (world_width > WORLD_WIDTH)
			{
				exit (EXIT_FAILURE);
			}
			if (world_height > WORLD_HEIGHT)
			{
				exit (EXIT_FAILURE);
			}
		
			load_random_world ();
			printf ("using random world, size %i x %i.\n", world_width, world_height);
		}
		else
		{
			world_width = atoi (av[2]);
			world_height = atoi (av[3]);
			
			if (world_width > WORLD_WIDTH)
			{
				printf ("ERROR: world width greater as 512!\n");
			}
			if (world_height > WORLD_HEIGHT)
			{
				printf ("ERROR: world height greater as 512!\n");
			}
		
			if (world_width > WORLD_WIDTH)
			{
				exit (EXIT_FAILURE);
			}
			if (world_height > WORLD_HEIGHT)
			{
				exit (EXIT_FAILURE);
			}
		
			if (load_world_bmp (av[1]) != 0)
			{
				printf ("load_world_bmp: ERROR can't load world bmp!\n");
				exit (EXIT_FAILURE);
			}
			else
			{
				printf ("using .bmp world: %s, size %i x %i.\n", av[1], world_width, world_height);
			}
		}
	}
	if (ac == 1)
	{
		printf ("battle0-server -random | world-file\n\n");
		printf ("battle0-server -random xsize ysize\n");
		printf ("battle0-server world-file xsize ysize\n\n"); 
		exit (1);
	}

	init_player_hashes ();

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

						hash = player[player_ind].hash;

						if (send_data (csd, &hash, sizeof (hash), BYTEORDER_NET) != 0)
						{
							exit (EXIT_FAILURE);
						}

						printf ("player: %li requested player number.\n", player_ind);
						break;

					case GET_WORLD:
						printf ("get world request...\n");
						if (recv_data (csd, &playernum, sizeof (playernum), BYTEORDER_NET) == 0)
						{
							printf ("player: %li requested world data.\n", playernum);

							send_world (csd);
						}
						break;

					case SET_UNIT_TANK:
						printf ("set unit tank request...\n");
						if (recv_data (csd, &playernum, sizeof (playernum), BYTEORDER_NET) == 0)
						{
							printf ("player: %li request.\n", player);
						}

						if (recv_data (csd, &playerhash, sizeof (playerhash), BYTEORDER_NET) == 0)
						{
							printf ("player: %li hash.\n", player);
						}

						if (recv_data (csd, &worldx, sizeof (worldx), BYTEORDER_NET) == 0)
						{
							printf ("worldx: %li\n", worldx);
						}

						if (recv_data (csd, &worldy, sizeof (worldy), BYTEORDER_NET) == 0)
						{
							printf ("worldy: %li\n", worldy);
						}

						if (playerhash == player[playernum].hash)
						{
							/* authorized by right hash */

							if (set_tank_server (playernum, worldx, worldy) == 0)
							{
								data = OK;
							}
							else
							{
								data = ERR;
							}
						}
						else
						{
							/* sender was not right player! */

							data = ERR;
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
						if (recv_data (csd, &playernum, sizeof (playernum), BYTEORDER_NET) == 0)
						{
							// printf ("player: %li request.\n", player);
						}

						if ((playernum == active_player) && (player[playernum].active == 1))
						{
							data = OK;
						}
						else
						{
							data = ERR;
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
						if (recv_data (csd, &playernum, sizeof (playernum), BYTEORDER_NET) == 0)
						{
							printf ("player: %li request move end.\n", playernum);
						}

						if (recv_data (csd, &playerhash, sizeof (playerhash), BYTEORDER_NET) == 0)
						{
							printf ("player: %li hash.\n", player);
						}

						if ((playerhash == player[playernum].hash) && (playernum == active_player))
						{
							/* authorized by right hash */

	
							calculate_data ();
							check_base_captured (playernum);
							if (send_data (csd, &player[playernum].active, sizeof (player[playernum].active), BYTEORDER_NET) == 0)
							{
								printf ("check base captured data sent.\n");
							}
								
						}
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
