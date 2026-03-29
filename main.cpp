#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image.h>
#include "src/includes.h"
#include "src/global.h"
#include "src/graphics.h"
#include "src/utils.h"
using namespace std;
using namespace utils;
using namespace glm;




GLFWwindow* Window;
void GLFWFrameBufferSizeCallback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);

	currentScreenRes = glm::ivec2(width, height);
	verticalFOV = 2 * atan(tan(camera.FOV / 2.0f) * (display::RENDER_RESOLUTION.x / display::RENDER_RESOLUTION.y));
}


void handleInputs() {
	//Get inputs for this frame
	glfwPollEvents();
	for (int key : monitoredKeys) {
		int keyState = glfwGetKey(Window, key);
		if (keyState == GLFW_PRESS) {
			keyMap[key] = true;

		} else if (keyState == GLFW_RELEASE) {
			keyMap[key] = false;
		}
	}

	if (keyMap[GLFW_KEY_1]) {
		glfwSetInputMode(Window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);			
	} else {
		glfwSetInputMode(Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		glfwGetCursorPos(Window, &cursorXPos, &cursorYPos);
	}
}




int main() {
	try { //Catch exceptions
	currentScreenRes = display::SCREEN_RESOLUTION;

	Window = graphics::initializeWindow(currentScreenRes.x, currentScreenRes.y, "Software-Renderer/Quake-Span-Renderer");
	glfwSetFramebufferSizeCallback(Window, GLFWFrameBufferSizeCallback);
	glfwGetCursorPos(Window, &cursorXPos, &cursorYPos);



	//Models setup;
	modelFiles.push_back(structs::ModelMeta(
		"cube", glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(1.57f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f)
	));



	cursorXPosPrev = cursorXPos;
	cursorYPosPrev = cursorYPos;
	graphics::prepareGraphics();
	utils::GLErrorcheck("Window Creation", true);


	int tick = 0;
	while (!glfwWindowShouldClose(Window)) {
		double frameStart = glfwGetTime();
		handleInputs();
		if (keyMap[GLFW_KEY_ESCAPE]) {
			break; //Quit
		}


		//Camera controls and span rendering.
		frame::updateCamera();
		frame::draw();


		glfwSwapBuffers(Window);
		double frametime = glfwGetTime() - frameStart;
		while (glfwGetTime() - frameStart < constants::DT) {}
		if (dev::SHOW_FREQ) {
			double totalTime = (glfwGetTime() - frameStart);
			std::cout << "Current frequency: " << std::floor(1.0f/totalTime) << " Theoretical frequency: " << std::floor(1.0f/frametime) << std::endl;
		}


		cursorXPosPrev = cursorXPos;
		cursorYPosPrev = cursorYPos;
		tick++;

		frameBuffer.clear();
	}

	glfwDestroyWindow(Window);
	glfwTerminate();
	return 0;


	//Catch exceptions.
	} catch (const std::exception& e) {
		std::cerr << "An exception was thrown: " << e.what() << std::endl;
		pause();
		return -1;
	} catch (...) {
		std::cerr << "An unspecified exception was thrown." << std::endl;
		pause();
		return -1;
	}
}
