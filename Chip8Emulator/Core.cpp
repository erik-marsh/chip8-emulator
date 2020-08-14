#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "Keys.h"
#include "Core.h"

double MakeNoise(int nChannel, double dTime)
{
	double dOutput = sin(440.0 * 2.0 * 3.14159 * dTime);
	return dOutput * 0.5; // Master Volume
}

uint8_t Core::fontset[80] = {
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

void Core::updateFramebuffer(uint8_t xPos, uint8_t yPos, uint8_t height)
{
	int xByte = xPos / 8;
	int xOffset = xPos % 8;
	int yByte = yPos * 8;
	bool checkForPixelDeactivation = true;
	registers[0xF] = 0x00; // clear collision bit

	// 0 | 00000000 11111111
	// 1 | 10000000 01111111
	// 2 | 11000000 00111111
	// 3 | 11100000 00011111
	// 4 | 11110000 00001111
	// 5 | 11111000 00000111
	// 6 | 11111100 00000011
	// 7 | 11111110 00000001

	uint16_t masks[8] = {
		0b0000000011111111,
		0b1000000001111111,
		0b1100000000111111,
		0b1110000000011111,
		0b1111000000001111,
		0b1111100000000111,
		0b1111110000000011,
		0b1111111000000001
	};

	for (uint8_t i = 0; i < height; i++)
	{
		int framebufferIndex = yByte + xByte + (i * 8);
		uint16_t oldFramebufferWord = (framebuffer[framebufferIndex] << 8) | framebuffer[framebufferIndex + 1];
		uint8_t oldByte = oldFramebufferWord >> (8 - xOffset);
		uint8_t newByte = memory[indexRegister + i];

		if (checkForPixelDeactivation && isPixelDeactivated(oldByte, newByte))
		{
			registers[0x0F] = 0x01;
			checkForPixelDeactivation = false;
		}

		newByte = oldByte ^ newByte;

		uint16_t newFramebufferWord = 0x0000;
		newFramebufferWord = newByte << (8 - xOffset);
		
		newFramebufferWord = newFramebufferWord | (oldFramebufferWord & masks[xOffset]);
		framebuffer[framebufferIndex] = newFramebufferWord >> 8;
		framebuffer[framebufferIndex + 1] = newFramebufferWord;
	}
}

bool Core::isPixelDeactivated(uint8_t oldByte, uint8_t newByte)
{
	uint8_t oldBit = 0x00;
	uint8_t newBit = 0x00;
	for (int i = 0; i < 8; i++)
	{
		oldBit = (oldByte >> i) & 0x01;
		newBit = (newByte >> i) & 0x01;
		if (oldBit == 0x01 && newBit == 0x01) return true;
	}
	return false;
}

double Core::soundCallback(int channels, double deltaTime)
{
	double targetFrequency = 0.0;
	if (soundTimer > 0) targetFrequency = 440.0;
	double dOutput = sin(targetFrequency * 2.0 * 3.14159 * deltaTime);
	return dOutput * 0.25;
}

Core::Core(GLFWwindow* context)
	: indexRegister(0x0000), programCounter(0x0200), stackPointer(-1), delayTimer(0x00), soundTimer(0x00), windowContext(context), isWaitingForInput(false)
{
	for (int i = 0; i < 16; i++)
	{
		registers[i] = 0x00;
	}

	for (int i = 0; i < 16; i++)
	{
		stack[i] = 0x0000;
	}

	for (int i = 0; i < 256; i++)
	{
		framebuffer[i] = 0x00;
	}

	for (int i = 0; i < 4096; i++)
	{
		memory[i] = 0x00;
	}

	generator = new std::default_random_engine(std::random_device{}());
	distribution = new std::uniform_int_distribution<int>(0x00, 0xFF);

	renderer = new Renderer();

	std::vector<wstring> devices = olcNoiseMaker<short>::Enumerate();
	sound = new olcNoiseMaker<short>(devices[0], 44100, 1, 8, 512);
	sound->SetUserFunction(std::bind(&Core::soundCallback, this, std::placeholders::_1, std::placeholders::_2));

	// load fontset into memory
	for (int i = 0; i < 80; i++)
	{
		memory[i] = fontset[i];
	}

	draw(); // initial draw just in case
}

Core::~Core()
{
	delete generator;
	delete distribution;
	delete sound;
}

void Core::loadProgram()
{
	std::ifstream file(".\\c8games\\BLITZ", std::ios::binary);
	if (!file.is_open()) 
	{
		std::cout << "Failed to open ROM." << std::endl;
		abort();
	}
	int ptr = 0;
	char buffer[4096 - 0x0200] = { 0 };
	while (!file.eof())
	{
		file.read(buffer + ptr, 1);
		ptr++;
	}

	for (int i = 0x0200; i < 4096; i++)
	{
		memory[i] = static_cast<uint8_t>(buffer[i - 0x0200]);
	}
}

void Core::opcode()
{
	uint16_t opcode = memory[programCounter] << 8 | memory[programCounter + 1];
	int regX = (opcode & 0x0F00) >> 8; // opcodes which specify V_x will always specify it in the low nibble of the high byte
	int regY = (opcode & 0x00F0) >> 4; // similarly, V_y will always be in the high nibble of the low byte
	uint16_t result = 0x00;

	switch (opcode & 0xF000)
	{
	// we ignore the 0nnn opcode
	case 0x0000:
		if (opcode == 0x00E0) // CLS
		{
			for (int i = 0; i < 32 * 8; i++)
			{
				framebuffer[i] = 0x00;
			}
			draw();
		}
		if (opcode == 0x00EE) // RET
		{
			// Returns from subroutine
			programCounter = stack[stackPointer];
			stackPointer--;
		}
		break;
	case 0x1000: // 0x1nnn - JP nnn
		// jump to address nnn
		programCounter = opcode & 0x0FFF;
		// do not update pc after this instruction
		// you can do it by subtracting 2 and letting it add it back, which is kinda messy but works
		programCounter -= 2;
		break;
	case 0x2000: // 0x2nnn - CALL nnn
		// call subroutine at address nnn
		// TODO: check for stack under/overflow
		stackPointer++;
		stack[stackPointer] = programCounter; //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		programCounter = opcode & 0x0FFF;
		programCounter -= 2; // similar to 0x1nnn !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! or is it???????????????????
		break;
	case 0x3000: // 0x3xkk - SE V_x, kk
		// skips next inst if V_x == kk
		if (registers[regX] == (opcode & 0x00FF))
		{
			programCounter += 2;
		}
		break;
	case 0x4000: // 0x4xkk - SNE V_x, kk
		// skips next inst if V_x != kk
		if (!(registers[regX] == (opcode & 0x00FF)))
		{
			programCounter += 2;
		}
		break;
	case 0x5000: // 0x5xy0 - SE V_x, V_y
		// TODO: notify user if low nibble != 0x0
		// Skip next inst if V_x == V_y
		if (registers[regX] == registers[regY])
		{
			programCounter += 2;
		}
		break;
	case 0x6000: // 0x6xkk - LD V_x, kk
		regX = (opcode & 0x0F00) >> 8;
		registers[regX] = opcode & 0x00FF;
		break;
	case 0x7000: // 0x7xkk - ADD V_x, kk
		registers[regX] += opcode & 0x00FF;
		break;
	case 0x8000: // bitwise operations + etc
		switch (opcode & 0x000F)
		{
		case 0x0000: // 0x8xy0 - LD V_x, V_y
			registers[regX] = registers[regY];
			break;
		case 0x0001: // 0x8xy1 - OR V_x, V_y
			registers[regX] = registers[regX] | registers[regY];
			break;
		case 0x0002: // 0x8xy2 - AND V_x, V_y
			registers[regX] = registers[regX] & registers[regY];
			break;
		case 0x0003: // 0x8xy3 - XOR V_x, V_y
			registers[regX] = registers[regX] ^ registers[regY];
			break;
		case 0x0004: // 0x8xy4 - ADD V_x, V_y
			result = registers[regX] + registers[regY];
			if (result > 0x00FF)
			{
				registers[0x0F] = 0x01; // carry flag
			}
			else
			{
				registers[0x0F] = 0x00;
			}
			registers[regX] = static_cast<uint8_t>(result);
			break;
		case 0x0005: // 0x8xy5 - SUB V_x, V_y
			if (registers[regX] > registers[regY])
			{
				registers[0x0F] = 0x01; // set V_F to the inverse of borrow
			}
			else
			{
				registers[0x0F] = 0x00;
			}
			registers[regX] = registers[regX] - registers[regY];
			break;
		case 0x0006: // 0x8xy6 - SHR V_x
			if ((registers[regX] & 0x01) == 0x01)
			{
				registers[0x0F] = 0x01;
			}
			else
			{
				registers[0x0F] = 0x00;
			}
			registers[regX] = registers[regX] >> 1;
			break;
		case 0x0007: // 0x8xy7 - SUBN V_x, V_y
			// subtract V_x FROM V_y (weird syntax)
			if (registers[regY] > registers[regX])
			{
				registers[0x0F] = 0x01; // set V_F to the inverse of borrow
			}
			else
			{
				registers[0x0F] = 0x00;
			}
			registers[regX] = registers[regY] - registers[regX];
			break;
		case 0x000E: // 0x8yE - SHL Vx
			if ((registers[regX] & 0x80) == 0x80)
			{
				registers[0x0F] = 0x01;
			}
			else
			{
				registers[0x0F] = 0x00;
			}
			registers[regX] = registers[regX] << 1;
			break;
		}
		break;
	case 0x9000: // 0x9xy0 - SNE V_x, V_y
		// TODO: notify user if low nibble != 0x0
		if (!(registers[regX] == registers[regY]))
		{
			programCounter += 2;
		}
		break;
	case 0xA000: // 0xAnnn - LD I, nnn
		indexRegister = opcode & 0x0FFF;
		break;
	case 0xB000: // 0xBnnn - JP V_0, nnn
		// jump to address nnn + V_0
		programCounter = (opcode & 0x0FFF) + registers[0];
		programCounter -= 2; // you know the drill
		break;
	case 0xC000: // 0xCxkk - RND V_x, kk
		// generate a random int from 0 to 255, then AND it with kk. store in V_x
		result = static_cast<uint8_t>(distribution->operator()(*generator)); // ugly
		registers[regX] = result & (opcode & 0x00FF);
		break;
	case 0xD000: // 0xDxyn - DRW V_x, V_y, n
		// draw an n-byte sprite at memory location I at (V_x, V_y)
		// height (!!) of sprite is indicated by n
		updateFramebuffer(registers[regX], registers[regY], opcode & 0x000F);
		draw();
		break;
	case 0xE000:
		if ((opcode & 0x00FF) == 0x009E) // 0xEx9E - SKP V_x
		{
			// if keypress == Vx, skip next instruction
			if (glfwGetKey(windowContext, KEY_MAP[registers[regX]]) == GLFW_PRESS)
			{
				programCounter += 2;
			}
		}
		if ((opcode & 0x00FF) == 0x00A1) // 0xE0A1 - SKNP V_x
		{
			// if keypress != Vx, skip next instruction
			if (glfwGetKey(windowContext, KEY_MAP[registers[regX]]) == GLFW_RELEASE)
			{
				programCounter += 2;
			}
		}
		break;
	case 0xF000:
		switch (opcode & 0x00FF)
		{
		case 0x0007: // 0xFx07 - LD V_x, DelayTimer
			registers[regX] = delayTimer;
			break;
		case 0x000A: // 0xFx0A - LD V_x, Keypress
			// blocks + waits for keypress
			// there's a much more accurate/faithful way to do this (https://retrocomputing.stackexchange.com/questions/358/how-are-held-down-keys-handled-in-chip-8)
			// but i don't feel like implementing it right now
			isWaitingForInput = true;
			break;
		case 0x0015: // 0xFx15 - LD DelayTimer, V_x
			delayTimer = registers[regX];
			break;
		case 0x0018: // 0xFx18 - LD SoundTimer, V_x
			soundTimer = registers[regX];
			break;
		case 0x001E: // 0xFx1E - ADD I, V_x
			indexRegister += registers[regX];
			break;
		case 0x0029: // 0xFx29 - LD F, V_x
			// loads the location of the sprite for the number specified by V_x into I
			// since every sprite is 5 bytes long, and fonts start at memory location 0x0000, we can do this
			indexRegister = registers[regX] * 5;
			break;
		case 0x0033: // 0xFx33 - LD B, V_x
			memory[indexRegister] = registers[regX] / 100;			 // 100s digit
			memory[indexRegister + 1] = (registers[regX] / 10) % 10; //  10s digit
			memory[indexRegister + 2] = registers[regX] % 10;		 //   1s digit
			break;
		case 0x0055: // 0xFx55 - LD [I], V_x
			// stores registers V_0 to V_x into memory sequentially, starting at the address in [I]
			// TODO: check if this is inclusive or exclusive
			for (uint8_t i = 0x00; i < (regX + 1); i++)
			{
				memory[indexRegister + i] = registers[i];
			}
			break;
		case 0x0065: // 0xFx65 - LD V_x, [I];
			// reads into regsiters V_0 to V_x the memory values starting at address [I
			// TODO: check if this is inclusive or exclusive
			for (uint8_t i = 0x00; i < (regX + 1); i++)
			{
				registers[i] = memory[indexRegister + i];
			}
			break;
		}
		break;
	}

	programCounter += 2;
}

void Core::draw()
{
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	renderer->draw(framebuffer, 32, 64);
	glfwSwapBuffers(windowContext);
}

void Core::updateDelayTimer(double currTime)
{
	if (delayTimer > 0)
	{
		double deltaTime = currTime - lastDelayTimerUpdate;
		if (deltaTime >= (1.0 / 60.0))
		{
			delayTimer--;
			lastDelayTimerUpdate = currTime;
		}
	}
}

void Core::updateSoundTimer(double currTime)
{
	if (soundTimer > 0)
	{
		double deltaTime = currTime - lastSoundTimerUpdate;

		if (deltaTime >= (1.0 / 60.0))
		{
			soundTimer--;
			lastSoundTimerUpdate = currTime;
		}
	}
}

const bool Core::getIsWaitingForInput() const
{
	return isWaitingForInput;
}
