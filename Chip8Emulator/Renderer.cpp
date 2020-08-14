#include "Renderer.h"
#include <iostream>

Renderer::Renderer()
{
	//m_drawMap = new unsigned char[32 * 64];
	//m_drawMap = new uint8_t[32 * 8]; // 32 rows of 64 bits (8 * 8 bits)
	//for (int i = 0; i < 32 * 8; i++) m_drawMap[i] = 0x00;

	/*for (int i = 0; i < 32 * 8; i++)
	{
		int row = i / 8;
		int col = i % 8;

		if (row % 2 == 0 && col % 2 == 0) m_drawMap[i] = 0xFF;
		if (row % 2 == 1 && col % 2 == 1) m_drawMap[i] = 0xFF;
	}*/

	//m_drawMap[0] = 0xF0;
	//m_drawMap[8] = 0x90;
	//m_drawMap[16] = 0x90;
	//m_drawMap[24] = 0x90;
	//m_drawMap[32] = 0xF0;

	m_shader = new Shader("shader.vert", "shader.frag");

	float vertices[] = {
		1.0f,  0.0f,  // top right
		1.0f,  1.0f,  // bottom right
		0.0f,  1.0f,  // bottom left
		0.0f,  0.0f   // top left
	};

	unsigned int indices[] = {
		0, 1, 3,
		1, 2, 3
	};

	glGenBuffers(1, &m_VBO);
	glGenBuffers(1, &m_EBO);
	glGenVertexArrays(1, &m_VAO);

	glBindVertexArray(m_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);
	glEnableVertexAttribArray(0);
}

Renderer::~Renderer()
{
	//delete[] m_drawMap;
	delete m_shader;
	glDeleteBuffers(1, &m_VBO);
	glDeleteBuffers(1, &m_EBO);
	glDeleteBuffers(1, &m_VAO);
}

void Renderer::draw(uint8_t* framebuffer, int height, int width)
{
	glm::mat4 model(1.0f);

	for (int i = 0; i < height * width; i++)
	{
		int row = i / width;
		int col = i % width;
		int numBytes = width / 8;

		int colByte = col / numBytes;
		int colOffset = (numBytes - 1) - (col % numBytes);

		uint8_t currByte = framebuffer[(row * 8) + colByte];

		if ((currByte >> colOffset) & 0x01)
		{
			model = glm::translate(model, glm::vec3(col, row, 0.0f));
			m_shader->setMat4("model", model);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
			model = glm::mat4(1.0f); //reset for next transform
		}
	}
}