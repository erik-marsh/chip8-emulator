#pragma once

#include <cstdint>
#include <random>

#include "Renderer.h"
#include "OlcNoiseMaker.h"
#include "Keys.h"

class Core
{
private:
	static uint8_t fontset[80];

	uint8_t registers[16];
	uint16_t indexRegister;
	uint16_t programCounter;
	int8_t stackPointer; // signed so we can use -1 as an "empty" value

	uint8_t delayTimer;
	uint8_t soundTimer;
	double lastDelayTimerUpdate;
	double lastSoundTimerUpdate;

	uint16_t stack[16];
	uint8_t framebuffer[32 * 8]; // 32 high x 64 wide bitmap
	uint8_t memory[4096];

	std::default_random_engine* generator;
	std::uniform_int_distribution<int>* distribution;

	GLFWwindow* windowContext;
	Renderer* renderer;
	olcNoiseMaker<short>* sound;

	void updateFramebuffer(uint8_t xPos, uint8_t yPos, uint8_t height);
	//bool isPixelDeactivated(uint16_t oldWord, uint16_t newWord, int xOffset);
	bool isPixelDeactivated(uint8_t oldByte, uint8_t newByte);

	double soundCallback(int nChannels, double deltaTime);

	bool isWaitingForInput;

public:
	Core(GLFWwindow* context);
	~Core();

	void loadProgram();
	void opcode();//uint16_t opcode);
	void draw();
	void updateDelayTimer(double currTime);
	void updateSoundTimer(double currTime);
	friend void Fx0AKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

	const bool getIsWaitingForInput() const;
};

// so gross but hey what can you do
inline void Fx0AKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	Core* c = static_cast<Core*>(glfwGetWindowUserPointer(window));

	if (!c->isWaitingForInput) return;

	for (int i = 0; i < 16; i++)
	{
		if (key == KEY_MAP[i] && action == GLFW_PRESS)
		{
			uint16_t opcode = c->memory[c->programCounter - 2] << 8 | c->memory[c->programCounter - 1]; // pc will be one instruction ahead during this
			int regX = (opcode & 0x0F00) >> 8;
			c->registers[regX] = i;
			c->isWaitingForInput = false;
			return;
		}
	}
}