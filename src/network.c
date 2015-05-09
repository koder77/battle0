// socket send/receive packets functions

#include "network.h"

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>

#if __ANDROID__

#include <SDL.h>
#include <SDL_net.h>

#else

#include <SDL/SDL.h>
#include <SDL/SDL_net.h>

#endif

#define PACKET_SIZE		4
#define ACK_BYTE		255

#define MAX_DATA_SIZE 	4

Sint16 send_data (TCPsocket sock, Uint8 *data, Sint32 data_length, Sint16 byteorder)
{
	/* send data as packets with fixed size */

	Sint32 i = 0, j, to_do;
	Sint32 packet_size;			/* size in bytes */
	Sint32 data_length_net;
	Sint32 data_read;
	
	Uint8 buf[PACKET_SIZE];
	
	Sint16 data_s;
	Sint32 data_l;
	Uint8 *data_ptr;
	Uint8 data_send[MAX_DATA_SIZE];
	
	if (byteorder == BYTEORDER_NET && data_length > MAX_DATA_SIZE)
	{
		printf ("send_data: ERROR data size to big for BYTEORDER_NET!\n");
		return (1);
	}
	
	/* DEBUG */
	for (i = 0; i < PACKET_SIZE; i++)
	{
		buf[i] = 0;
	}
	
	if (byteorder == BYTEORDER_NET)
	{
		/* put data in network byteorder */
	
		switch (data_length)
		{
			case 1:
				byteorder = BYTEORDER_DATA;
				break;
				
			case 2:
				data_ptr = (Uint8 *) &data_s;
				data_ptr[0] = data[0];
				data_ptr[1] = data[1];
				
				data_s = htons (data_s);
				
				for (i = 0; i < 2; i++)
				{
					data_send[i] = data_ptr[i];
				}
				break;
				
			case 4:
				data_ptr = (Uint8 *) &data_l;
				data_ptr[0] = data[0];
				data_ptr[1] = data[1];
				data_ptr[2] = data[2];
				data_ptr[3] = data[3];
				
				data_l = htonl (data_l);
				
				for (i = 0; i < 4; i++)
				{
					data_send[i] = data_ptr[i];
					// printf ("send_data: buf %i = %i\n", i, data_send[i]);
				}
				break;
				
			default:
				printf ("send_data: ERROR unknown data size for BYTEORDER_NET!\n");
				return (1);
		}
	}
				
	/* send data size */
	data_length_net = htonl (data_length);
	
	if (SDLNet_TCP_Send (sock, (void *) &data_length_net, sizeof (data_length_net)) < sizeof (data_length_net))
	{
		// fprintf (stderr, "SDLNet_TCP_Send: %s\n", SDLNet_GetError ());
		
		fprintf (stderr, "SDLNet_TCP_Send error! data length\n");
		return (1);
	}
	
	to_do = data_length; i = 0;
	while (to_do > 0)
	{
		if (to_do < PACKET_SIZE)
		{
			packet_size = to_do;
		}
		else
		{
			packet_size = PACKET_SIZE;
		}
		
		for (j = 0; j < packet_size; j++)
		{
			if (byteorder == BYTEORDER_NET)
			{
				buf[j] = data_send[i];
			}
			else
			{
				buf[j] = data[i];
			}
			i++;
		}
		
		if (SDLNet_TCP_Send (sock, (void *) &buf, PACKET_SIZE) < PACKET_SIZE)
		{
			// fprintf (stderr, "SDLNet_TCP_Send: %s\n", SDLNet_GetError ());
			
			fprintf (stderr, "SDLNet_TCP_Send error! data\n");
			return (1);
		}
		
		to_do = to_do - packet_size;
	}
	
	// wait for ACK byte
	
	data_read = SDLNet_TCP_Recv (sock, &buf, 1);
	if (data_read != 1)
	{
		fprintf (stderr, "SDLNet_TCP_Recv error! ACK\n");
		return (1);
	}
	
	if (buf[0] != ACK_BYTE)
	{
		fprintf (stderr, "send_data error! ACK byte corrupt!\n");
		return (1);
	}
	
	return (0);
}

Sint16 recv_data (TCPsocket sock, Uint8 *data, Sint32 data_size, Sint16 byteorder)
{
	/* receive data as packets with fixed size */

	Sint32 i = 0, j, to_do;
	Sint32 packet_size;			/* size in bytes */
	Sint32 data_length_net, data_length;
	Sint32 data_read;
	
	Uint8 buf[PACKET_SIZE];
	Uint8 *ptr = (Uint8 *) &data_length_net;
	
	Sint16 data_s, datah_s;
	Sint32 data_l, datah_l;
	Uint8 *data_ptr;
	
	if (byteorder == BYTEORDER_NET && data_size > MAX_DATA_SIZE)
	{
		printf ("recv_data: ERROR data size to big for BYTEORDER_NET!\n");
		return (1);
	}
	
	/* receive data size */
	to_do = sizeof (data_length_net);
	while (to_do > 0)
	{
		data_read = SDLNet_TCP_Recv (sock, &buf, to_do);
		
		for (j = 0; j < data_read; j++)
		{
			*ptr++ = buf[j];
		}
		to_do -= data_read;
	}
	
	data_length = ntohl (data_length_net);
	
	// printf ("recv_data: reading %li bytes...\n", data_length);
	
	to_do = data_length;
	while (to_do > 0)
	{
		if (to_do < PACKET_SIZE)
		{
			packet_size = to_do;
		}
		else
		{
			packet_size = PACKET_SIZE;
		}
		
		data_read = SDLNet_TCP_Recv (sock, &buf, PACKET_SIZE);
		
		for (j = 0; j < data_read; j++)
		{
			if (i < data_size)
			{
				// printf ("recv_data: buf %i = %i\n", i, buf[j]);
				data[i] = buf[j];
				i++;
			}
		}
		to_do -= data_read;
	}
	
	// send ACK byte
	
	buf[0] = ACK_BYTE;
	if (SDLNet_TCP_Send (sock, (void *) &buf, 1) < 1)
	{
		fprintf (stderr, "SDLNet_TCP_Send error! ACK byte\n");
		return (1);
	}
	
	if (byteorder == BYTEORDER_NET)
	{
		/* put data in host byteorder */
	
		switch (data_length)
		{
			case 1:
				byteorder = BYTEORDER_DATA;
				break;
				
			case 2:
				data_ptr = (Uint8 *) &data_s;
				
				for (i = 0; i < 2; i++)
				{	
					data_ptr[i] = data[i];
				}
				
				datah_s = ntohs (data_s);
				data_ptr = (Uint8 *) &datah_s;
				
				for (i = 0; i < 2; i++)
				{
					data[i] = data_ptr[i];
				}
				break;
				
			case 4:
				data_ptr = (Uint8 *) &data_l;
				
				for (i = 0; i < 4; i++)
				{	
					data_ptr[i] = data[i];
				}
				
				datah_l = ntohl (data_l);
				
				// printf ("recv_data: data: %li\n", datah_l);
				
				data_ptr = (Uint8 *) &datah_l;
				
				for (i = 0; i < 4; i++)
				{
					data[i] = data_ptr[i];
				}
				break;
				
			default:
				printf ("recv_data: ERROR unknown data size for BYTEORDER_NET!\n");
				return (1);
		}
	}
	
	return (0);
}
