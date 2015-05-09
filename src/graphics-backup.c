#include "global_defs.h"

#if __ANDROID__

#include <android/log.h>

#define  LOG_TAG    "battle0-log"

#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

#include "sdl-graphics-lines.c"

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

extern Sint16 screen_width;
extern Sint16 screen_height;
extern Uint8 video_bpp;
extern Sint16 world_up_x;
extern Sint16 world_up_y;
extern Uint8 play_token;
extern Sint32 player_number;
extern Sint16 unit_info_wx;
extern Sint16 unit_info_wy;


int copy_surface (SDL_Surface *src, SDL_Rect *srcrect, SDL_Surface *dst, SDL_Rect *dstrect)
{
    Uint32 pixel;
    Uint8 r, g, b;
    Sint16 src_x, src_y, dst_x, dst_y;
    Sint16 srcrect_x, srcrect_y, srcrect_w, srcrect_h;
    Sint16 dstrect_x, dstrect_y;

    if (srcrect == NULL)
    {
        srcrect_x = 0;
        srcrect_y = 0;
        srcrect_w = src->w;
        srcrect_h = src->h;
    }
    else
    {
        srcrect_x = srcrect->x;
        srcrect_y = srcrect->y;
        srcrect_w = srcrect->w;
        srcrect_h = srcrect->h;
    }

    if (dstrect == NULL)
    {
        dstrect_x = 0;
        dstrect_y = 0;
    }
    else
    {
        dstrect_x = dstrect->x;
        dstrect_y = dstrect->y;
    }

    dst_y = dstrect_y;

    for (src_y = srcrect_y; src_y < srcrect_y + srcrect_h; src_y++)
    {
        dst_x = dstrect_x;

        for (src_x = srcrect_x; src_x < srcrect_x + srcrect_w; src_x++)
        {
            if (SDL_LockSurface (src) < 0)
            {
                printf ("copy_surface: can't lock source surface!\n");
                return (-1);
            }

            pixel = getpixel (src, src_x, src_y);
            SDL_UnlockSurface (src);
            SDL_GetRGB (pixel, src->format, &r, &g, &b);

            pixelRGBA (dst, dst_x, dst_y, r, g, b, 255);

            dst_x++;
        }

        dst_y++;
    }
    return (0);
}

void create_screen_copy_surface (void)
{
	if (screen_copy == NULL)
	{
		/* Create a 32-bit surface with the bytes of each pixel in R,G,B,A order,
       as expected by OpenGL for textures */
		
		Uint32 rmask, gmask, bmask, amask;

		/* SDL interprets each pixel as a 32-bit number, so our masks must depend
       on the endianness (byte order) of the machine */
		#if SDL_BYTEORDER == SDL_BIG_ENDIAN
			rmask = 0xff000000;
			gmask = 0x00ff0000;
			bmask = 0x0000ff00;
			amask = 0x00000000;
		#else
			rmask = 0x000000ff;
			gmask = 0x0000ff00;
			bmask = 0x00ff0000;
			amask = 0x00000000;
		#endif

		screen_copy = SDL_CreateRGBSurface (SDL_SWSURFACE, screen_width, screen_height, video_bpp, 0, 0, 0, 0);
		if(screen_copy == NULL) 
		{
			fprintf(stderr, "CreateRGBSurface failed: %s\n", SDL_GetError());
			exit(1);
		}
		
		if (SDL_SetAlpha (screen_copy, SDL_SRCALPHA, 0) < 0)
		{
			fprintf (stderr, "Can't set alpha channel!\n");
		}
	}
}

void copy_screen (void)
{
	create_screen_copy_surface ();
	
    copy_surface (screen, NULL, screen_copy, NULL);
}

void update_screen (void)
{
	LOGD("update screen\n");
	
	// SDL_UpdateRect (screen[screennum].bmap, 0, 0, 0, 0);
	SDL_Flip (screen);
        
	LOGD("screen updated\n");

	copy_screen ();	
}

Uint8 restore_screen ()
{
	copy_surface (screen_copy, NULL, screen, NULL);
	SDL_FreeSurface (screen_copy);
	screen_copy = NULL;
	update_screen ();
	
	return (TRUE);
}

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
	
	if (play_token == TRUE)
	{
		/* draw green circle in HUD, it's players turn */
		filledCircleRGBA (screen, hud_x + 550, hud_y + 20, 10, 24, 184, 31, 255);
	}
		
	update_screen ();
	
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
	
	if (tank->aim_x != -1 && tank->aim_y != -1 && unit[tank->aim_y][tank->aim_x].type != EMPTY)
	{
		draw_user_selected_unit (screen, tank->aim_x, tank->aim_y, zoom);
	}
	
	update_screen ();
	
	return (0);
}

void draw_tank (Sint16 x, Sint16 y, Uint8 color, Sint16 tank_cannon_angle, Sint16 zoom);

void set_player_home (Sint16 player_number, Sint16 zoom)
{
	Sint16 max_x, max_y, hud_bottom;
	
	if (zoom == 1)
	{
		hud_bottom = 4;		/* tile height of HUD */
	}
	else
	{
		hud_bottom = 2;		/* tile height of HUD */
	}
	
	max_x = (screen_width / (TILE_WIDTH * zoom));
	max_y = (screen_height / (TILE_WIDTH * zoom)) - hud_bottom;
	
	switch (player_number)
	{
		case 0:
			/* red = upper left corner */
			world_up_x = 0; world_up_y = 0;
			break;
			
		case 1:
			/* blue = upper right corner */
			world_up_x = WORLD_WIDTH - max_x;
			world_up_y = 0;
			break;
			
		case 2:
			/* yellow = bottom right corner */
			world_up_x = WORLD_WIDTH - max_x;
			world_up_y = WORLD_HEIGHT - max_y;
			break;
			
		case 3:
			/* green = bottom left corner */
			world_up_x = 0;
			world_up_y = WORLD_HEIGHT - max_y;
			break;
	}
}

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
	if (world_up_y > WORLD_HEIGHT - 1) world_up_y = WORLD_HEIGHT - 1;
	
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
	
	update_screen ();	
}

void draw_line_rotated (SDL_Surface *screen, Sint16 x, Sint16 y, Sint16 length, Sint16 angle, Sint16 width, Uint8 r, Uint8 g, Uint8 b)
{
	double aspect_ratio = 1.0, dangle, rads, dr;
	double dradius = length / (aspect_ratio * 2.0);
	int ra, x2, y2;
	
	dangle = angle;
	rads = (dangle + 90.0) * M_PI / 180.0;
			
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

void draw_shot_anim (Sint16 unit_x, Sint16 unit_y, Sint16 target_x, Sint16 target_y, Sint16 zoom)
{
	Sint16 x1 = ((unit_x - world_up_x) * zoom) + (zoom / 2);
	Sint16 y1 = ((unit_y - world_up_y) * zoom) + (zoom / 2);
	
	Sint16 x2 = (target_x - world_up_x) * zoom + (zoom / 2);
	Sint16 y2 = (target_y - world_up_y) * zoom + (zoom / 2);
	
	thickLineRGBA (screen, x1, y1, x2, y2, zoom / 8, 0, 0, 0, 255);
	update_screen ();
	
	SDL_Delay (1500);
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
	
	update_screen ();
}



