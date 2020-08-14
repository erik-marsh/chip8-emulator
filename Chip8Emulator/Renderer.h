#pragma once

#include <cstdint>

#include "Shader.h"

class Renderer
{
private:
	Shader * m_shader;
	unsigned int m_VBO;
	unsigned int m_EBO;
	unsigned int m_VAO;

public:
	Renderer();
	~Renderer();
	
	void draw(uint8_t* framebuffer, int height, int width);
};

