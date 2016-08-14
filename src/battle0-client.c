// Battle0 - client V 0.7

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
Uint32 world_height, world_width;		/* world size, sent by server */

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
struct player player[MAX_PLAYERS];

/* player wants to know info about unit, stored here: */
Sint16 unit_info_wx = -1, unit_info_wy = -1;

IPaddress ip;		/* Server address */	
TCPsocket sd, csd;		/* Socket descriptor */
Uint8 ip_str[80], user_ip[80];
Uint8 av_str[80];
Sint16 ac_g;

Uint8 play_token = FALSE;
Uint8 net = FALSE;				/* set to true by network init thread */
SDL_mutex *net_mutex;

/* screen functions */
SDL_Surface *screen_copy = NULL;

SDL_Joystick *joystick;			/* global joystick handle */


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
	
	
	draw_text_ttf (hud_font, request, 50, 50, 255, 255, 255);
	SDL_UpdateRect (screen, 0, 0, 0, 0);
	update_screen ();
	
	SDL_Delay (4000);
	
#if __ANDROID__
	/* show on screen keyboard */
	SDL_ANDROID_GetScreenKeyboardTextInput (ip_str, 79);
	
	strcpy (request, "server ip: ");
	strcat (request, ip_str);
		
	boxRGBA (screen, 0, 0, screen_width - 1, screen_height - 1, 0, 0, 0, 255);
	draw_text_ttf (hud_font, request, 50, 50, 255, 255, 255);
	update_screen ();
	
	SDL_Delay (4000);
	
	return (0);
}
	
#else
	
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
									
									return (1);
								}

								/* Use alpha blending */
								if (SDL_SetAlpha (screen, SDL_SRCALPHA, 0) < 0)
								{
									fprintf (stderr, "Can't set alpha channel!\n");
									
									return (1);
								}
								
								restore_screen ();			/* copy screen buffer back to screen (made by last update_screen () call */
								
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
		update_screen ();
	}
	
	return (0);
}		

#endif


static int handle_network_init (void *ptr)
{
	Uint8 debug[256];
	Uint8 buffer[512];
	Sint32 command, data, bytes;
	Uint8 ok = FALSE;
	
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
	
	LOGD("got server ip");
	
	/* Resolve the host we are connecting to */
	if (SDLNet_ResolveHost (&ip, ip_str, 2010) < 0)
	{
		fprintf (stderr, "SDLNet_ResolveHost: %s\n", SDLNet_GetError ());
	    strcpy (debug, SDLNet_GetError ());
		LOGE("%s\n", debug);
		exit (EXIT_FAILURE);
	}
	
	#else
	
	if (ac_g < 2)
	{
		/* no server ip set -> ask user on screen */
		
		get_server_ip ();
	}
	else
	{
		strcpy (ip_str, user_ip);
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
	update_screen ();
	LOGD("loading graphics...");
	
	load_ground_bitmaps ();
	
	
	
	LOGD("connecting to server...");
	
	draw_text_ttf (hud_font, "connecting to server, please wait...", 50, 200, 255, 255, 255);
	update_screen ();
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
	
	player[player_number].active = 1;
	
	if (recv_data (sd, &player[player_number].hash, sizeof (player[player_number].hash), BYTEORDER_NET) != 0)
	{
		exit (EXIT_FAILURE);
	}
	
	printf ("player hash: %li\n\n", player[player_number].hash);
	
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
	update_screen ();
	recv_world (sd);
	
	SDLNet_TCP_Close (sd);
	
	set_player_home (player_number, zoom);
	
	printf ("player: %li, home: %li / %li\n", player_number, world_up_x, world_up_y);
	
	draw_world (zoom);
	
	
	while (! ok)
	{
		SDL_Delay (10);
		if (SDL_mutexP (net_mutex) != -1)
		{
			net = TRUE;		/* set network as initialized as message for main thread */
			ok = TRUE;
			SDL_mutexV (net_mutex);
		}
	}
		
	return (0);
}
	
void place_start_units (Sint16 zoom)
{
	Uint8 tanks[80], tanks_val[80];
	Sint16 mx, my;
	Sint16 button = 0;
	Sint16 worldx, worldy, hud_bottom, max_x, max_y;
	Sint16 i;
	Sint16 tanks_to_do = 10;
	
	Sint16 hud_x = 30; Sint16 hud_y = (screen_height - TILE_WIDTH * 4) + 5;
	Sint16 ret;
	
	
	while (tanks_to_do > 0)
	{
		/* left colummn */
		boxRGBA (screen, 0, screen_height - TILE_WIDTH * 4, screen_width - 1, screen_height - 1, 238, 231, 38, 255);
		snprintf (tanks_val, 80, "%li", tanks_to_do);
		strcpy (tanks, "tanks to place: ");
		strcat (tanks, tanks_val);
		draw_text_ttf (hud_font, tanks, hud_x, hud_y, 0, 0, 0);
		update_screen ();
		
		ret = user_place_tank (zoom);
		
		if (ret == 0) 
		{
			tanks_to_do--;
			draw_text_ttf (hud_font, "OK                ", hud_x, hud_y + 30, 0, 0, 0);
			update_screen ();
		}
			
		if (ret == 2)
		{
			draw_text_ttf (hud_font, "unit too far away!", hud_x, hud_y + 30, 255, 0, 0);
			update_screen ();
		}
	}
}
	
	
int main (int ac, char *av[])
{
	const SDL_VideoInfo *info;
	/* Initialize the SDL library */
	
    int audio_rate = 44100;
    Uint16 audio_format = AUDIO_S16; 
    int audio_channels = 2;
    int audio_buffers = 4096;
	
	Sint16 screen_reopened;
	int ret;
	int net_ok;
	
	Uint8 debug[256];
	
	Uint8 buffer[512];
	Sint32 command, data, bytes;

	if (ac >= 2)
	{
		strcpy (user_ip, av[1]);
	}
	
	if (ac == 4)
	{
		screen_width = atoi (av[2]);
		screen_height = atoi (av[3]);
	}
	
	ac_g = ac;
	
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
	
	net_mutex = SDL_CreateMutex();
	
	
	/* start input handling thread */
	SDL_Thread *thread;
    int threadReturnValue;

    // Simply create a thread
    thread = SDL_CreateThread (handle_network_init, (void *)NULL);

    if (NULL == thread) 
	{
        printf("\nSDL_CreateThread failed: %s\n", SDL_GetError());
		return (1);
	}
	
	LOGD("network init thread started");

	// wait for "q" key	to EXIT
	SDL_Event event;

    SDL_JoystickEventState (SDL_ENABLE);
    joystick = SDL_JoystickOpen (0);
	
	net_ok = 0;
	while (net_ok == 0)
	{
		SDL_Delay (50);
		if (SDL_mutexP (net_mutex) != -1)
		{
			if (net == TRUE)
			{
				place_start_units (zoom);
				net_ok = 1;
			}
			SDL_mutexV (net_mutex);
		}
	}
	

wait:
	SDL_Delay (50);
	if (SDL_mutexP (net_mutex) != -1)
	{
		if (net == TRUE)
		{
			/* network initialized */
			if (play_token == FALSE)
			{
				get_play_token (player_number);
				
				if (play_token == TRUE)
				{
					receive_units ();
					draw_world (zoom);
				}
			}
		}
		SDL_mutexV (net_mutex);
	}
	
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
									
							restore_screen ();
							
							screen_reopened = 1;
						}
					}
				}
				break;
			
			case SDL_KEYDOWN:
                switch (event.key.keysym.sym)
                {
					case SDLK_b:
						set_player_home (1, zoom);		/* blue basis */
						break;
						
					case SDLK_d:
						receive_units ();
						draw_world (zoom);
						break;
					
					case SDLK_f:
						user_fire_unit (zoom);
						break;
					
					case SDLK_g:
						set_player_home (3, zoom);		/* green basis */
						break;
						
					case SDLK_i:
						user_unit_info (zoom);
						break;
					
					case SDLK_h:
						set_player_home (player_number, zoom);
						draw_world (zoom);
						break;
						
					case SDLK_m:
						if (play_token == TRUE) user_move_unit (zoom);
						break;
						
                     case SDLK_q:
						SDLNet_Quit ();
                        exit (0);
                        break;
					
					 case SDLK_r:
						 set_player_home (0, zoom);
						 break;
						
					 case SDLK_SPACE:
						 if (play_token == TRUE)
						 {
							moves_finished ();
						 
							// receive_units ();
							play_token = FALSE;
							draw_world (zoom);
						 }
						 break;
						 
					 case SDLK_t:
						 if (play_token == TRUE)
						 {
							user_place_tank (zoom);
						 }
						 break;
						
					 case SDLK_y:
						 set_player_home (2, zoom);
						 break;
						 
					 case SDLK_KP_PLUS:
						 zoom = 2;
						 draw_world (zoom);
						 break;
						 
					case SDLK_KP_MINUS:
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
				if (event.button.button == SDL_BUTTON_RIGHT)
				{
					user_fire_unit (zoom);
				}
				else
				{
					user_unit_info (zoom);
				}
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

