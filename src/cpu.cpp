#include <cstring>
#include <fstream>
#include "cpu.h"
#include <SDL.h>

cpu::cpu() {
	unsigned char fontset[80] = {
		0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
		0x20, 0x60, 0x20, 0x20, 0x70, // 1
		0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
		0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
		0x90, 0x90, 0xF0, 0x10, 0x10, // 4
		0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
		0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
		0xF0, 0x10, 0x20, 0x40, 0x40, // 7
		0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
		0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
		0xF0, 0x90, 0xF0, 0x90, 0x90, // A
		0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
		0xF0, 0x80, 0x80, 0x80, 0xF0, // C
		0xE0, 0x90, 0x90, 0x90, 0xE0, // D
		0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
		0xF0, 0x80, 0xF0, 0x80, 0x80  // F
	};

	screen_update_flag = false;
	memset(memory, 0, sizeof(memory));
	opcode = 0;
	memset(V, 0, sizeof(V));
	PC = 0x200; //ROM starts at 0x200
	I = 0;
	memset(stack, 0, sizeof(stack));
	sp = 0;
	delay_timer = 0;
	sound_timer = 0;
	memset(display, 0, sizeof(display));
	memset(keys, 0, sizeof(keys));

	//Load fontset into 0x50–0x9F
	for (int i = 0; i < 80; i++) {
		memory[0x50 + i] = fontset[i];
	}

}

bool cpu::load_rom(std::string file_path) {
	std::ifstream file(file_path, std::ios::in | std::ios::binary);

	if (!file.is_open()) {
		return false;
	}

	char c;

	for (int i = 0x200; file.get(c); i++) {
		memory[i] = c;
	}

	return true;
}

void cpu::one_cycle() {
	//fetch
	opcode = (memory[PC] << 8) | memory[PC + 1];
	SDL_Log("Address: 0x%x opcode: 0x%x", PC, opcode);

	PC += 2;
	
	//decode
	//break opcode into 4 4-bit nibbles
	unsigned char nibble1 = opcode >> 12;

	unsigned char nibble2 = (opcode >> 8) & 0b00001111;
	unsigned char& X = nibble2;

	unsigned char nibble3 = (opcode >> 4) & 0b00001111;
	unsigned char& Y = nibble3;

	unsigned char nibble4 = opcode & 0b00001111;
	unsigned char& N = nibble4;

	//NN = the third and fourth nibbles
	unsigned char NN = (nibble3 << 4) | nibble4;

	//NNN = the seconds, third, and forth nibbles
	unsigned short NNN = (((nibble2 << 4) | nibble3) << 4) | nibble4;

	switch (nibble1) {
	case 0x0:
		switch (nibble4) {
		//0x00E0, clear screen
		case 0x0:
			memset(display, 0, sizeof(display));
			screen_update_flag = true;
			break;

		//0x00EE, return from subroutine
		case 0xE:
			//TODO
			break;
		}
		break;

	//0x1NNN, jump NNN
	case 0x1:
		PC = NNN;
		break;

	case 0x2:
		break;

	case 0x3:
		break;

	case 0x4:
		break;

	case 0x5:
		break;

	//0x6XNN, set VX to NN
	case 0x6:
		V[X] = NN;
		break;

	//0x7XNN, add NN to VX
	case 0x7:
		V[X] += NN;
		break;

	case 0x8:
		break;

	case 0x9:
		break;

	//0xANNN, set I to NNN
	case 0xA:
		I = NNN;
		break;

	case 0xB:
		break;

	case 0xC:
		break;
	
	//0xDXYN, Display
	//Draw N pixels tall sprite from memory[I] at coordinates (VX,VY)
	//If any pixels are turned off VF is set to 1
	case 0xD:
		unsigned int x_coord, y_coord;
		x_coord = V[X] % 64;
		y_coord = V[Y] % 32;

		V[0xF] = 0;

		//single byte of sprite data from memory
		unsigned char sprite_data;

		for (int y_index = 0; y_index < N; y_index++) {
			sprite_data = memory[I + y_index];

			for (int x_index = 7; x_index >= 0; x_index--) {
				int bit = (sprite_data >> x_index) & 1;

				if (bit == 1) {
					if (display[(x_coord - x_index) + ((y_coord + y_index) * 64)] == 1) {
						V[0xF] = 1;
					}
					screen_update_flag = true;
					display[(x_coord - x_index) + ((y_coord + y_index) * 64)] ^= 1;
				}
			}
		}

		break;

	case 0xE:
		break;

	case 0xF:
		break;


	}
}

unsigned char* cpu::get_display() { return display; }