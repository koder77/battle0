#include "global_defs.h"

#include "network.h"

extern IPaddress ip;		/* Server address */	
extern TCPsocket sd;		/* Socket descriptor */

extern Uint8 world[WORLD_HEIGHT][WORLD_WIDTH];
extern struct unit unit[WORLD_HEIGHT][WORLD_WIDTH];

extern Uint8 play_token;

Sint16 send_world (TCPsocket sock)
{
	Sint16 wx, wy, j;
	Uint8 buf[4];
	
	for (wy = 0; wy < WORLD_HEIGHT; wy++)
	{
		for (wx = 0; wx < WORLD_WIDTH; wx += 4)
		{
			for (j = 0; j < 4; j++)
			{
				// pack 4 horizontal world tiles in one packet
				buf[j] = world[wy][wx + j];
			}
			
			printf ("sending world: %li / %li\n", wx, wy);
			if (send_data (sock, buf, sizeof (buf), BYTEORDER_DATA) != 0)
			{
				return (1);
			}
		}
	}
	
	return (0);
}

Sint16 recv_world (TCPsocket sock)
{
	Sint16 wx, wy, j;
	Uint8 buf[4];
	
	for (wy = 0; wy < WORLD_HEIGHT; wy++)
	{
		for (wx = 0; wx < WORLD_WIDTH; wx += 4)
		{
			printf ("loading world: %li / %li: ", wx, wy);
			if (recv_data (sock, buf, sizeof (buf), BYTEORDER_DATA) != 0)
			{
				printf ("\n");
				return (1);
			}

			for (j = 0; j < 4; j++)
			{
				world[wy][wx + j] = buf[j];
				unit[wy][wx + j].type = EMPTY;		// no unit set
				
				printf ("%li ", buf[j]);
			}
			printf ("\n");
		}
	}
	
	return (0);
}

Sint16 send_unit_tank (TCPsocket sock, Sint16 worldx, Sint16 worldy)
{
	Sint32 command, ack;
	struct tank *tank;
	
	tank = (struct tank *) unit[worldy][worldx].data;
	
	command = SEND_TANK;
	
	if (send_data (sock, (Uint8 *) &command, sizeof (command), BYTEORDER_NET) != 0)
	{
		return (1);
	}
	
	if (send_data (sock, (Uint8 *) &unit[worldy][worldx].color, sizeof (unit[worldy][worldx].color), BYTEORDER_NET) != 0)
	{
		return (1);
	}
	
	if (send_data (sock, &worldx, sizeof (worldx), BYTEORDER_NET) != 0)
	{
		return (1);
	}
	
	if (send_data (sock, &worldy, sizeof (worldy), BYTEORDER_NET) != 0)
	{
		return (1);
	}
	
	if (send_data (sock, (Uint8 *) &tank->health, sizeof (tank->health), BYTEORDER_NET) != 0)
	{
		return (1);
	}
	
	if (send_data (sock, (Uint8 *) &tank->aim_angle, sizeof (tank->aim_angle), BYTEORDER_NET) != 0)
	{
		return (1);
	}
	
	if (send_data (sock, (Uint8 *) &tank->aim_x, sizeof (tank->aim_x), BYTEORDER_NET) != 0)
	{
		return (1);
	}
	
	if (send_data (sock, (Uint8 *) &tank->aim_y, sizeof (tank->aim_y), BYTEORDER_NET) != 0)
	{
		return (1);
	}
	
	if (send_data (sock, (Uint8 *) &tank->max_ap, sizeof (tank->max_ap), BYTEORDER_NET) != 0)
	{
		return (1);
	}
	
	if (send_data (sock, (Uint8 *) &tank->ap, sizeof (tank->ap), BYTEORDER_NET) != 0)
	{
		return (1);
	}
	
	if (send_data (sock, (Uint8 *) &tank->move_x, sizeof (tank->move_x), BYTEORDER_NET) != 0)
	{
		return (1);
	}
	
	if (send_data (sock, (Uint8 *) &tank->move_y, sizeof (tank->move_y), BYTEORDER_NET) != 0)
	{
		return (1);
	}
	
	if (send_data (sock, (Uint8 *) &tank->timer_ready, sizeof (tank->timer_ready), BYTEORDER_NET) != 0)
	{
		return (1);
	}
	
	if (send_data (sock, (Uint8 *) &tank->timer_move, sizeof (tank->timer_move), BYTEORDER_NET) != 0)
	{
		return (1);
	}
	
	if (send_data (sock, (Uint8 *) &tank->in_move, sizeof (tank->in_move), BYTEORDER_NET) != 0)
	{
		return (1);
	}
	
	if (send_data (sock, (Uint8 *) &tank->timer_reload, sizeof (tank->timer_reload), BYTEORDER_NET) != 0)
	{
		return (1);
	}
	
	if (send_data (sock, (Uint8 *) &tank->ccs, sizeof (tank->ccs), BYTEORDER_NET) != 0)
	{
		return (1);
	}
	
	if (send_data (sock, (Uint8 *) &tank->hull, sizeof (tank->hull), BYTEORDER_NET) != 0)
	{
		return (1);
	}
	
	if (send_data (sock, (Uint8 *) &tank->motor, sizeof (tank->motor), BYTEORDER_NET) != 0)
	{
		return (1);
	}
	
	if (send_data (sock, (Uint8 *) &tank->cannon, sizeof (tank->cannon), BYTEORDER_NET) != 0)
	{
		return (1);
	}
	
	if (send_data (sock, (Uint8 *) &tank->give_up, sizeof (tank->give_up), BYTEORDER_NET) != 0)
	{
		return (1);
	}
	
	if (recv_data (sock, (Uint8 *) &ack, sizeof (ack), BYTEORDER_NET) != 0)
	{
		return (1);
	}
	
	return (0);
}

Sint16 recv_unit_tank (TCPsocket sock, Sint16 worldx, Sint16 worldy, Sint32 player_number)
{
	Sint32 ack;
	struct tank *tank;
	
	unit[worldy][worldx].data = (struct tank *) malloc (sizeof (struct tank));
	if (unit[worldy][worldx].data == NULL)
	{
		printf ("recv_unit_tank: ERROR no memory for unit!\n");
		return (1);
	}
	
	tank = (struct tank *) unit[worldy][worldx].data;
	
	unit[worldy][worldx].type = TANK;
	unit[worldy][worldx].color = player_number;
	
	if (recv_data (sock, (Uint8 *) &tank->health, sizeof (tank->health), BYTEORDER_NET) != 0)
	{
		return (1);
	}
	
	if (recv_data (sock, (Uint8 *) &tank->aim_angle, sizeof (tank->aim_angle), BYTEORDER_NET) != 0)
	{
		return (1);
	}
	
	if (recv_data (sock, (Uint8 *) &tank->aim_x, sizeof (tank->aim_x), BYTEORDER_NET) != 0)
	{
		return (1);
	}
	
	if (recv_data (sock, (Uint8 *) &tank->aim_y, sizeof (tank->aim_y), BYTEORDER_NET) != 0)
	{
		return (1);
	}
	
	if (recv_data (sock, (Uint8 *) &tank->max_ap, sizeof (tank->max_ap), BYTEORDER_NET) != 0)
	{
		return (1);
	}
	
	if (recv_data (sock, (Uint8 *) &tank->ap, sizeof (tank->ap), BYTEORDER_NET) != 0)
	{
		return (1);
	}
	
	if (recv_data (sock, (Uint8 *) &tank->move_x, sizeof (tank->move_x), BYTEORDER_NET) != 0)
	{
		return (1);
	}
	
	if (recv_data (sock, (Uint8 *) &tank->move_y, sizeof (tank->move_y), BYTEORDER_NET) != 0)
	{
		return (1);
	}
	
	if (recv_data (sock, (Uint8 *) &tank->timer_ready, sizeof (tank->timer_ready), BYTEORDER_NET) != 0)
	{
		return (1);
	}
	
	if (recv_data (sock, (Uint8 *) &tank->timer_move, sizeof (tank->timer_move), BYTEORDER_NET) != 0)
	{
		return (1);
	}
	
	if (recv_data (sock, (Uint8 *) &tank->in_move, sizeof (tank->in_move), BYTEORDER_NET) != 0)
	{
		return (1);
	}
	
	if (recv_data (sock, (Uint8 *) &tank->timer_reload, sizeof (tank->timer_reload), BYTEORDER_NET) != 0)
	{
		return (1);
	}
	
	if (recv_data (sock, (Uint8 *) &tank->ccs, sizeof (tank->ccs), BYTEORDER_NET) != 0)
	{
		return (1);
	}
	
	if (recv_data (sock, (Uint8 *) &tank->hull, sizeof (tank->hull), BYTEORDER_NET) != 0)
	{
		return (1);
	}
	
	if (recv_data (sock, (Uint8 *) &tank->motor, sizeof (tank->motor), BYTEORDER_NET) != 0)
	{
		return (1);
	}
	
	if (recv_data (sock, (Uint8 *) &tank->cannon, sizeof (tank->cannon), BYTEORDER_NET) != 0)
	{
		return (1);
	}
	
	if (recv_data (sock, (Uint8 *) &tank->give_up, sizeof (tank->give_up), BYTEORDER_NET) != 0)
	{
		return (1);
	}
	
	ack = OK;
	if (send_data (sock, (Uint8 *) &ack, sizeof (ack), BYTEORDER_NET) != 0)
	{
		return (1);
	}
	
	return (0);
}

/* TANK MOVE */
Sint16 send_unit_tank_move (TCPsocket sock, Sint16 worldx, Sint16 worldy)
{
	Sint32 command, ack;
	struct tank *tank;
	
	tank = (struct tank *) unit[worldy][worldx].data;
	
	command = SEND_TANK_MOVE;
	
	if (send_data (sock, (Uint8 *) &command, sizeof (command), BYTEORDER_NET) != 0)
	{
		return (1);
	}
	
	if (send_data (sock, (Uint8 *) &unit[worldy][worldx].color, sizeof (unit[worldy][worldx].color), BYTEORDER_NET) != 0)
	{
		return (1);
	}
	
	if (send_data (sock, (Uint8 *) &worldx, sizeof (worldx), BYTEORDER_NET) != 0)
	{
		return (1);
	}
	
	if (send_data (sock, (Uint8 *) &worldy, sizeof (worldy), BYTEORDER_NET) != 0)
	{
		return (1);
	}
	
	if (send_data (sock, (Uint8 *) &tank->move_x, sizeof (tank->move_x), BYTEORDER_NET) != 0)
	{
		return (1);
	}
	
	if (send_data (sock, (Uint8 *) &tank->move_y, sizeof (tank->move_y), BYTEORDER_NET) != 0)
	{
		return (1);
	}
	
	if (recv_data (sock, (Uint8 *) &ack, sizeof (ack), BYTEORDER_NET) != 0)
	{
		return (1);
	}
	
	return (0);
}

Sint16 recv_unit_tank_move (TCPsocket sock)
{
	Sint16 worldx, worldy;
	Sint32 ack, player_number;
	struct tank *tank;
	
	if (recv_data (sock, (Uint8 *) &player_number, sizeof (player_number), BYTEORDER_NET) != 0)
	{
		return (1);
	}
				
	if (recv_data (sock, (Uint8 *) &worldx, sizeof (worldx), BYTEORDER_NET) != 0)
	{
		return (1);
	}
	
	if (recv_data (sock, (Uint8 *) &worldy, sizeof (worldy), BYTEORDER_NET) != 0)
	{
		return (1);
	}
	
	tank = (struct tank *) unit[worldy][worldx].data;
	
	if (unit[worldy][worldx].color != player_number)
	{
		/* player number mismatch to unit -> sended data invalid */
		printf ("DATA INVALID!\n");
		return (1);
	}
	
	if (recv_data (sock, (Uint8 *) &tank->move_x, sizeof (tank->move_x), BYTEORDER_NET) != 0)
	{
		return (1);
	}
	
	if (recv_data (sock, (Uint8 *) &tank->move_y, sizeof (tank->move_y), BYTEORDER_NET) != 0)
	{
		return (1);
	}
	
	tank->in_move = 0;
	
	ack = OK;
	if (send_data (sock, (Uint8 *) &ack, sizeof (ack), BYTEORDER_NET) != 0)
	{
		return (1);
	}
	
	return (0);
}


/* TANK FIRE */
Sint16 send_unit_tank_fire (TCPsocket sock, Sint16 worldx, Sint16 worldy)
{
	Sint32 command, ack;
	struct tank *tank;
	
	tank = (struct tank *) unit[worldy][worldx].data;
	
	command = SEND_TANK_FIRE;
	
	if (send_data (sock, (Uint8 *) &command, sizeof (command), BYTEORDER_NET) != 0)
	{
		return (1);
	}
	
	if (send_data (sock, (Uint8 *) &unit[worldy][worldx].color, sizeof (unit[worldy][worldx].color), BYTEORDER_NET) != 0)
	{
		return (1);
	}
	
	if (send_data (sock, (Uint8 *) &worldx, sizeof (worldx), BYTEORDER_NET) != 0)
	{
		return (1);
	}
	
	if (send_data (sock, (Uint8 *) &worldy, sizeof (worldy), BYTEORDER_NET) != 0)
	{
		return (1);
	}
	
	if (send_data (sock, (Uint8 *) &tank->aim_x, sizeof (tank->aim_x), BYTEORDER_NET) != 0)
	{
		return (1);
	}
	
	if (send_data (sock, (Uint8 *) &tank->aim_y, sizeof (tank->aim_y), BYTEORDER_NET) != 0)
	{
		return (1);
	}
	
	if (send_data (sock, (Uint8 *) &tank->fire, sizeof (tank->fire), BYTEORDER_NET) != 0)
	{
		return (1);
	}
	
	if (recv_data (sock, (Uint8 *) &ack, sizeof (ack), BYTEORDER_NET) != 0)
	{
		return (1);
	}
	
	return (0);
}

Sint16 recv_unit_tank_fire (TCPsocket sock)
{
	Sint16 worldx, worldy;
	Sint32 ack, player_number;
	struct tank *tank;
	
	if (recv_data (sock, (Uint8 *) &player_number, sizeof (player_number), BYTEORDER_NET) != 0)
	{
		return (1);
	}
				
	if (recv_data (sock, (Uint8 *) &worldx, sizeof (worldx), BYTEORDER_NET) != 0)
	{
		return (1);
	}
	
	if (recv_data (sock, (Uint8 *) &worldy, sizeof (worldy), BYTEORDER_NET) != 0)
	{
		return (1);
	}
	
	tank = (struct tank *) unit[worldy][worldx].data;
	
	if (unit[worldy][worldx].color != player_number)
	{
		/* player number mismatch to unit -> sended data invalid */
		printf ("DATA INVALID!\n");
		return (1);
	}
	
	if (recv_data (sock, (Uint8 *) &tank->aim_x, sizeof (tank->aim_x), BYTEORDER_NET) != 0)
	{
		return (1);
	}
	
	if (recv_data (sock, (Uint8 *) &tank->aim_y, sizeof (tank->aim_y), BYTEORDER_NET) != 0)
	{
		return (1);
	}
	
	if (recv_data (sock, (Uint8 *) &tank->fire, sizeof (tank->fire), BYTEORDER_NET) != 0)
	{
		return (1);
	}
	
	ack = OK;
	if (send_data (sock, (Uint8 *) &ack, sizeof (ack), BYTEORDER_NET) != 0)
	{
		return (1);
	}
	
	return (0);
}


Sint16 send_units (TCPsocket sock)
{
	Sint16 wx, wy, j;
	Uint8 buf[4];
	Sint32 end;
	
	for (wy = 0; wy < WORLD_HEIGHT; wy++)
	{
		for (wx = 0; wx < WORLD_WIDTH; wx++)
		{
			if (unit[wy][wx].type != EMPTY)
			{
				/* send data */
				
				switch (unit[wy][wx].type)
				{
					case TANK:
						printf ("send_units: tank\n");
						
						if (send_unit_tank (sock, wx, wy) != 0)
						{
							return (1);		/* error */
						}
						break;
				}
			}
		}
	}
	
	/* send data end to client. */
	/* client closes connection now */
	
	end = SEND_END;
	if (send_data (sock, (Uint8 *) &end, sizeof (end), BYTEORDER_NET) != 0)
	{
		return (1);
	}
	
	return (0); /* all ok */
}

void free_units (void)
{
	Sint16 wx, wy, j;
	Uint8 buf[4];
	
	struct tank *tank;
	
	for (wy = 0; wy < WORLD_HEIGHT; wy++)
	{
		for (wx = 0; wx < WORLD_WIDTH; wx++)
		{
			if (unit[wy][wx].type != EMPTY)
			{
				free (unit[wy][wx].data);
				unit[wy][wx].type = EMPTY;				/* set as empty */
			}
		}
	}
}

Sint16 receive_units ()
{
	Sint16 worldx, worldy, j;
	Uint8 buf[4];
	Uint8 ok = FALSE;
	Sint32 command, player_number;
	
	struct tank *tank;
	
	free_units ();
	
	/* Open a connection with the IP provided (listen on the host's port) */
	if (!(sd = SDLNet_TCP_Open (&ip)))
	{
		fprintf (stderr, "SDLNet_TCP_Open: %s\n", SDLNet_GetError ());
		return (1);
	}
	
	command = RECEIVE_UNITS;
	
	if (send_data (sd, (Uint8 *) &command, sizeof (command), BYTEORDER_NET) != 0)
	{
		return (1);
	}
	
	while (! ok)
	{
		if (recv_data (sd, (Uint8 *) &command, sizeof (command), BYTEORDER_NET) != 0)
		{
			return (1);
		}
		
		switch (command)
		{
			case SEND_TANK:
				if (recv_data (sd, (Uint8 *) &player_number, sizeof (player_number), BYTEORDER_NET) != 0)
				{
					return (1);
				}
				
				if (recv_data (sd, &worldx, sizeof (worldx), BYTEORDER_NET) != 0)
				{
					return (1);
				}
	
				if (recv_data (sd, &worldy, sizeof (worldy), BYTEORDER_NET) != 0)
				{
					return (1);
				}
					
				recv_unit_tank (sd, worldx, worldy, player_number);
				break;
				
			case SEND_END:
				ok = TRUE;
				break;
		}
	}
	
	SDLNet_TCP_Close (sd);
	return (0);
}

Sint16 get_play_token (Sint32 player_number)
{
	Sint32 command, data;
	command = GET_PLAY_TOKEN;
	
	/* Open a connection with the IP provided (listen on the host's port) */
	if (!(sd = SDLNet_TCP_Open (&ip)))
	{
		fprintf (stderr, "SDLNet_TCP_Open: %s\n", SDLNet_GetError ());
		return (1);
	}
	
	if (send_data (sd, (Uint8 *) &command, sizeof (command), BYTEORDER_NET) != 0)
	{
		return (1);
	}

	if (send_data (sd, (Uint8 *) &player_number, sizeof (player_number), BYTEORDER_NET) != 0)
	{
		return (1);
	}
	
	if (recv_data (sd, (Uint8 *) &data, sizeof (data), BYTEORDER_NET) != 0)
	{
		return (1);
	}
	
	if (data == OK)
	{
		play_token = TRUE;
	}
	else
	{
		play_token = FALSE;
	}
	
	SDLNet_TCP_Close (sd);
	return (0);
}
