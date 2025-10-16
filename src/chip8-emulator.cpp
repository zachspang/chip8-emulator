// chip8-emulator.cpp : Defines the entry point for the application.
//

#include "cpu.h"
#include <iostream>
#include <SDL.h>

const int WINDOW_SCALE_FACTOR = 20;

//SDL audio callback
void audio_callback(void* userdata, Uint8* stream, int len) {
	//stream casted to int16_t for AUDIO_S16LSB format
	int16_t* stream_16bit = (int16_t*)stream;
	
	//How many audio samples per second
	const int audio_sample_rate = 44100;

	//Frequency for the beeping sound
	const int square_wave_freq = 410;

	//Time to complete one one wave period
	const int square_wave_period = audio_sample_rate / square_wave_freq;

	//Volume of beeping sound
	const int volume = 500;

	//Index of current sample, this is static so sampling picks up where it left off if the stream runs out 
	// of bytes in the middle of a cycle
	static int sample_index = 0;

	//Each sample is 16 bits with AUDIO_S16LSB, stream is a Uint8 array. When a value is set for a sample it 
	// is setting two elements of stream so we need to divide len by 2
	for (int i = 0; i < len / 2; i++) {
		//If the sample is at the crest of the wave volume is positive, if it is a trough volume is negative
		if ((sample_index++ / (square_wave_period / 2)) % 2) {
			stream_16bit[i] = volume;
		} else {
			stream_16bit[i] = -volume;
		}
	}
}

int main(int argc, char *argv[])
{ 
	//Initialize SDL Video
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

	//Initialize SDL input
	const Uint8* keyboard_state = SDL_GetKeyboardState(nullptr);

	//Initialize SDL audio
	SDL_AudioSpec desired_spec, obtained_spec;
	desired_spec.freq = 44100;
	desired_spec.format = AUDIO_S16LSB;
	desired_spec.channels = 1;
	desired_spec.samples = 512;
	desired_spec.callback = audio_callback;

	SDL_AudioDeviceID audio_device = SDL_OpenAudioDevice(NULL, 0, &desired_spec, &obtained_spec, 0);

	if (audio_device == 0) {
		SDL_Log("Failed to get audio device %s", SDL_GetError());
	}

	if (desired_spec.format != obtained_spec.format ||
		desired_spec.channels != obtained_spec.channels) {
		SDL_Log("Failed to match desired audio spec");
	}

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

		chip8.keys[0x1] = keyboard_state[SDL_SCANCODE_1];
		chip8.keys[0x2] = keyboard_state[SDL_SCANCODE_2];
		chip8.keys[0x3] = keyboard_state[SDL_SCANCODE_3];
		chip8.keys[0xC] = keyboard_state[SDL_SCANCODE_4];

		chip8.keys[0x4] = keyboard_state[SDL_SCANCODE_Q];
		chip8.keys[0x5] = keyboard_state[SDL_SCANCODE_W];
		chip8.keys[0x6] = keyboard_state[SDL_SCANCODE_E];
		chip8.keys[0xD] = keyboard_state[SDL_SCANCODE_R];

		chip8.keys[0x7] = keyboard_state[SDL_SCANCODE_A];
		chip8.keys[0x8] = keyboard_state[SDL_SCANCODE_S];
		chip8.keys[0x9] = keyboard_state[SDL_SCANCODE_D];
		chip8.keys[0xE] = keyboard_state[SDL_SCANCODE_F];

		chip8.keys[0xA] = keyboard_state[SDL_SCANCODE_Z];
		chip8.keys[0x0] = keyboard_state[SDL_SCANCODE_X];
		chip8.keys[0xB] = keyboard_state[SDL_SCANCODE_C];
		chip8.keys[0xF] = keyboard_state[SDL_SCANCODE_V];

		//Play audio
		if (chip8.is_beeping()) {
			//Play sound
			SDL_PauseAudioDevice(audio_device, 0);
		} else {
			//Stop sound
			SDL_PauseAudioDevice(audio_device, 1);
		}

		//SDL_Delay(16);
	}

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_CloseAudioDevice(audio_device);
	SDL_Quit();

	return 0;
}