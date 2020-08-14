#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "Shader.h"
#include "Renderer.h"
#include "Core.h"
#include "Keys.h"

GLFWwindow* initOpenGLEnvironment();
void framebufferSizeCallback(GLFWwindow* window, int width, int height);
void processGlobalInput(GLFWwindow* window);

const int WINDOW_WIDTH = 640;
const int WINDOW_HEIGHT = 320;
const glm::mat4 PROJECTION_MATRIX = glm::ortho(
	0.0f,  64.0f, // left to right
	32.0f, 0.0f, // bottom to top
	-1.0f, 1.0f  // near to far
);

int main()
{
	GLFWwindow* window = initOpenGLEnvironment();
	if (!window) return 1;

	Shader shader("shader.vert", "shader.frag");
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	shader.use();
	shader.setMat4("projection", PROJECTION_MATRIX);
	shader.setFloat("color", 1.0f, 1.0f, 1.0f);

	Core core(window);
	glfwSetWindowUserPointer(window, &core);
	glfwSetKeyCallback(window, Fx0AKeyCallback);

	double targetFramerate = 600.0;
	double targetFrequency = 1.0 / targetFramerate;

	double initTime = glfwGetTime();
	double lastFrameTime = initTime;

	core.loadProgram();

	while (!glfwWindowShouldClose(window))
	{
		double currTime = glfwGetTime();
		double deltaTime = currTime - lastFrameTime;
		//std::cout << deltaTime << std::endl;
		processGlobalInput(window);
		bool blocking = core.getIsWaitingForInput();

		if (deltaTime >= targetFrequency && !blocking)
		{
			core.opcode();
			lastFrameTime = currTime;
		}

		// TODO: fix the off by one error in sound timer? maybe its just startup lag
		core.updateSoundTimer(currTime);
		core.updateDelayTimer(currTime);

		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}

GLFWwindow* initOpenGLEnvironment()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Chip8", NULL, NULL);
	if (!window)
	{
		std::cerr << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return nullptr;
	}

	glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cerr << "Failed to initialize GLAD" << std::endl;
		return nullptr;
	}

	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

	return window;
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void processGlobalInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}