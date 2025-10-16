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

	const Uint8* keyboardState = SDL_GetKeyboardState(nullptr);

	//Initialize cpu
	cpu chip8;
	if (!chip8.load_rom(argv[1])) {
		SDL_LogError(1, "Unable to open ROM at: %s", argv[1]);
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
		
		//store key states
		/*
			Keypad                   Keyboard
			+-+-+-+-+                +-+-+-+-+
			|1|2|3|C|                |1|2|3|4|
			+-+-+-+-+                +-+-+-+-+
			|4|5|6|D|                |Q|W|E|R|
			+-+-+-+-+       =>       +-+-+-+-+
			|7|8|9|E|                |A|S|D|F|
			+-+-+-+-+                +-+-+-+-+
			|A|0|B|F|                |Z|X|C|V|
			+-+-+-+-+                +-+-+-+-+

		*/
		SDL_PumpEvents();
		
		std::copy(chip8.keys, chip8.keys + 16, chip8.prev_keys);

		chip8.keys[0x1] = keyboardState[SDL_SCANCODE_1];
		chip8.keys[0x2] = keyboardState[SDL_SCANCODE_2];
		chip8.keys[0x3] = keyboardState[SDL_SCANCODE_3];
		chip8.keys[0xC] = keyboardState[SDL_SCANCODE_4];

		chip8.keys[0x4] = keyboardState[SDL_SCANCODE_Q];
		chip8.keys[0x5] = keyboardState[SDL_SCANCODE_W];
		chip8.keys[0x6] = keyboardState[SDL_SCANCODE_E];
		chip8.keys[0xD] = keyboardState[SDL_SCANCODE_R];

		chip8.keys[0x7] = keyboardState[SDL_SCANCODE_A];
		chip8.keys[0x8] = keyboardState[SDL_SCANCODE_S];
		chip8.keys[0x9] = keyboardState[SDL_SCANCODE_D];
		chip8.keys[0xE] = keyboardState[SDL_SCANCODE_F];

		chip8.keys[0xA] = keyboardState[SDL_SCANCODE_Z];
		chip8.keys[0x0] = keyboardState[SDL_SCANCODE_X];
		chip8.keys[0xB] = keyboardState[SDL_SCANCODE_C];
		chip8.keys[0xF] = keyboardState[SDL_SCANCODE_V];

		//SDL_Delay(16);
	}

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}
