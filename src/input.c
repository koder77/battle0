#include "global_defs.h"

extern SDL_Joystick *joystick;

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
		
		if (buttonmask & SDL_BUTTON (SDL_BUTTON_LEFT))
		{
			*button = 1;
		}

		if (buttonmask & SDL_BUTTON (SDL_BUTTON_RIGHT))
		{
			*button = 2;
		}

		if (buttonmask & SDL_BUTTON (SDL_BUTTON_MIDDLE))
		{
			*button = 3;
		}
	}
	
	return (0);		/* no error */
}

Sint16 get_joystick_state (Sint16 *mx, Sint16 *my, Sint16 *button, Sint16 check_button)
{
	SDL_PumpEvents ();
	
	*mx = SDL_JoystickGetAxis (joystick, 0);
	*my = SDL_JoystickGetAxis (joystick, 1);
	
	*button = SDL_JoystickGetButton (joystick, check_button);

	return (0);
}

Sint16 get_key_state (Uint8 *key)
{
	SDL_Event event;
	
	SDL_PollEvent (&event);
	
    switch (event.type)
	{
		case SDL_KEYDOWN:
            *key = event.key.keysym.sym;
			break;
			
		default:
			break;
	}
	
	return (0);
}