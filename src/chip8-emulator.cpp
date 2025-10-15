// chip8-emulator.cpp : Defines the entry point for the application.
//

#include "cpu.h"
#include <iostream>
#include <SDL.h>

const int WINDOW_SCALE_FACTOR = 20;

int main(int argc, char *argv[])
{ 
	//Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
		SDL_LogError(1, "Unable to initialize SDL: %s", SDL_GetError());
		return 1;
	}

	SDL_Window* window = SDL_CreateWindow(
		"CHIP8 Emulator",
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		64 * WINDOW_SCALE_FACTOR,
		32 * WINDOW_SCALE_FACTOR,
		0
	);

	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 1);

	//Initialize cpu
	cpu chip8;
	if (!chip8.load_rom("roms\\3-corax+.ch8")) {
		SDL_LogError(1, "Unable to open ROM");
		return 1;
	}
	
	while(true) {
		chip8.one_cycle();

		if (chip8.screen_update_flag) {
			SDL_RenderClear(renderer);

			SDL_Rect rect = SDL_Rect();
			rect.w = WINDOW_SCALE_FACTOR;
			rect.h = WINDOW_SCALE_FACTOR;

			unsigned char* display = chip8.get_display();

			//Draw a rectangle to represent each pixel
			for (int i = 0; i < 2048; i++) {
				//Turn i into 2d coords
				rect.x = (i % 64) * WINDOW_SCALE_FACTOR;
				rect.y = (i / 64) * WINDOW_SCALE_FACTOR;

				if (display[i] == 1) {
					SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
				}
				else {
					SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
				}
				SDL_RenderFillRect(renderer, &rect);
			}

			SDL_RenderPresent(renderer);

			chip8.screen_update_flag = false;
		}
		
		//store key states in memory

		//SDL_Delay(16);

	}

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}
