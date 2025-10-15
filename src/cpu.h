#include <string>
#include <chrono>

class cpu {
public:
	bool screen_update_flag;

	//Create a new cpu object and initialize memory and registers
	cpu();

	//Load ROM into memory. Returns false if it fails
	bool load_rom(std::string);

	//Emulate one cpu cycle
	void one_cycle();

	unsigned char* get_display();
	
private:
	//4kB Memory
	unsigned char memory[4096];

	//2 byte current opcode
	unsigned short opcode;

	//16 one byte general purpose registers, V0 - VF
	unsigned char V[16];

	//2 byte Program Counter, points to current instruction in memory
	unsigned short PC;

	//2 byte Index register points to a location in memory
	unsigned short I;

	//Stack for return addresses
	unsigned short stack[12];

	//Stack pointer
	unsigned short sp;

	//Timestamp of last timer update
	std::chrono::time_point<std::chrono::steady_clock> last_timer_update_timestamp;

	//Two timer registers that decrement 60 times a second until hitting 0. sound_timer beeps when not 0
	unsigned char delay_timer;
	unsigned char sound_timer;

	//64 * 32 grid of pixels, each is 1 or 0 because the only colors are black and white
	unsigned char display[2048];

	//States of the 16 keys on the keypad
	unsigned char keys[16];

};