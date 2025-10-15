#include <cstring>
#include <fstream>
#include "cpu.h"
#include <SDL.h>
#include <random>
#include <chrono>

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
	last_timer_update_timestamp = std::chrono::steady_clock::now();;
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
	//update timers
	auto now = std::chrono::steady_clock::now();
	std::chrono::duration<double> elapsed_time = now - last_timer_update_timestamp;

	if (elapsed_time.count() * 1000 > 16.666) {
		if (delay_timer > 0) delay_timer--;
		if (sound_timer > 0) {
			sound_timer--;
			//TODO: play beep sound
		}
		last_timer_update_timestamp = std::chrono::steady_clock::now();
		//SDL_Log("Time Since Last Timer Update: %fms", elapsed_time.count() * 1000);
	}

	//fetch
	opcode = (memory[PC] << 8) | memory[PC + 1];
	//SDL_Log("Address: 0x%x opcode: 0x%x", PC, opcode);

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
			PC = stack[sp];
			sp--;
			break;

		}

		break;

	//0x1NNN, jump NNN
	case 0x1:
		PC = NNN;
		break;

	//0x2NNN, call subroutine
	case 0x2:
		sp++;
		stack[sp] = PC;
		PC = NNN;
		break;

	//0x3XNN, if V[X] == NN skip next instruction
	case 0x3:
		if (V[X] == NN) PC += 2; 
		break;

	//0x4XNN, if V[X] != NN skip next instruction
	case 0x4:
		if (V[X] != NN) PC += 2;
		break;

	//0x5XY0, if V[X] == V[Y] skip next instruction
	case 0x5:
		if (V[X] == V[Y]) PC += 2;
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
		switch (nibble4) {
		//0x8XY0, set VX = VY
		case 0x0:
			V[X] = V[Y];
			break;
		
		//0x8XY1, set VX = VX OR VY
		case 0x1:
			V[X] = V[X] | V[Y];
			break;

		//0x8XY2, set VX = VX AND VY
		case 0x2:
			V[X] = V[X] & V[Y];
			break;

		//0x8XY3, set VX = VX XOR VY
		case 0x3:
			V[X] = V[X] ^ V[Y];
			break;

		//0x8XY4, set VX = VX + VY, set VF = carry
		case 0x4:
		{
			unsigned short sum = V[X] + V[Y];
			V[0xF] = sum > 255;
			V[X] = sum & 0b11111111;
			break;
		}
		//0x8XY5, set VX = VX - VY, set VF = NOT borrow
		case 0x5:
			V[0xF] = V[X] > V[Y];
			V[X] = V[X] - V[Y];
			break;

		//!!!Some interpreters move VY into VX first, this might be an issue in some games
		//0x8XY6, set VX = VX SHIFT RIGHT 1, set VF to old least significant bit of VX
		case 0x6:
			V[0xF] = V[X] & 1;
			V[X] = V[X] >> 1;
			break;
			
		//0x8XY7, set VX = VY - VX, set VF = NOT borrow
		case 0x7:
			V[0xF] = V[Y] > V[X];
			V[X] = V[Y] - V[X];
			break;

		//0x8XYE, set VX = VX SHIFT LEFT 1, set VF to old most significant bit of VX
		case 0xE:
			unsigned short bit_shifted = V[X] << 1;
			V[0xF] = bit_shifted >> 8;
			V[X] = bit_shifted & 0b11111111;
			break;
		}

		break;
	
	//0x9XY0, if V[X] != V[Y] skip next instruction
	case 0x9:
		if (V[X] != V[Y]) PC += 2;
		break;

	//0xANNN, set I to NNN
	case 0xA:
		I = NNN;
		break;

	//0xBNNN, set PC to NNN + V0
	case 0xB:
		PC = NNN + V[0];
		break;

	//0xCXNN, VX = RANDOM(0-255) & NN
	case 0xC:
	{
		std::default_random_engine generator;
		std::uniform_int_distribution<int> distribution(0, 255);
		unsigned char random_num = distribution(generator);
		V[X] = random_num & NN;
		break;
	}
	//0xDXYN, Display
	//Draw N pixels tall sprite from memory[I] at coordinates (VX,VY)
	//If any pixels are turned off VF is set to 1
	case 0xD:
	{
		unsigned int x_coord, y_coord;
		x_coord = V[X] % 64;
		y_coord = V[Y] % 32;

		V[0xF] = 0;

		//single byte of sprite data from memory
		unsigned char sprite_data;

		for (int y_index = 0; y_index < N; y_index++) {
			sprite_data = memory[I + y_index];

			for (int x_index = 0; x_index < 8; x_index++) {
				//get the bit from sprite data corresponding to the current pixel
				int bit = (sprite_data >> (7 - x_index)) & 1;
				int display_index = (x_coord + x_index) + ((y_coord + y_index) * 64);

				if (display_index > 2047) break;

				if (bit == 1) {	
					if (display[display_index] == 1) {
						V[0xF] = 1;
					}
					screen_update_flag = true;
					display[display_index] ^= 1;
				}
			}
		}

		break;
	}
	case 0xE:
		switch (nibble3) {
		//0xEX9E, if the key with value VX is pressed skip the next instruction
		case 0x9:
			if (keys[V[X]]) PC += 2;
			break;

		//0xEXA1, if the key with value VX is not pressed skip the next instruction
		case 0xA:
			if (!keys[V[X]]) PC += 2;
			break;
		}
		break;

	case 0xF:
		switch (NN) {
		//0xFX07, set VX to the value of the delay timer
		case 0x07:
			V[X] = delay_timer;
			break;

		//0xFX0A, Wait for a key press then store the key value in VX
		case 0x0A:
			PC -= 2;
			//TODO: Finish this
			break;

		//0xFX15, set the delay timer to VX
		case 0x15:
			delay_timer = V[X];
			break;
		
		//0xFX18, set the sound timer to VX
		case 0x18:
			sound_timer = V[X];
			break;

		//0xFX1E, add VX to I
		case 0x1E:
			I += V[X];
			break;

		//0xFX29, set I to the font address for a hexidecimal character in VX
		case 0x29:
			I = 0x50 + (V[X] * 5);
			break;

		//0xFX33, store a BCD representation of VX in memory location I, I+1, and I+2
		case 0x33:
			memory[I] = V[X] / 100;
			memory[I + 1] = (V[X] / 10) % 10;
			memory[I + 2] = V[X] % 10;
			break;

		//0xFX55, store V0-VX in memory starting at I
		case 0x55:
			for (int index = 0; index <= X; index++) {
				memory[I + index] = V[index];
			}
			break;

		//0xFX65, read V0-VX from memory starting at I
		case 0x65:
			for (int index = 0; index <= X; index++) {
				V[index] = memory[I + index];
			}
			break;
		}
		break;
	}
}

unsigned char* cpu::get_display() { return display; }