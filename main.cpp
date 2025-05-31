#define STB_IMAGE_IMPLEMENTATION
#include "C:/Users/User/Documents/code/.cpp/stb_image.h"
#include "src/includes.h"
#include "src/render.h"
#include "src/utils.h"
using namespace std;
using namespace utils;
using namespace glm;



// Keyboard presses to monitor.
std::array<int, 16> monitoredKeys = { //16 should cover necessary keys.
	GLFW_KEY_W, GLFW_KEY_S,
	GLFW_KEY_A, GLFW_KEY_D,
	GLFW_KEY_E, GLFW_KEY_Q,
	GLFW_KEY_1, GLFW_KEY_ESCAPE,
	GLFW_KEY_LEFT_SHIFT, GLFW_KEY_LEFT_CONTROL,
	GLFW_KEY_LEFT_ALT
};


std::array<std::string, display::TEXTURE_ARRAY_MAX_LAYERS> textureNames = {
	"a", "b", "c",
	"mus2", "osa", "piloten", "s_t_a_r_e",
	"switch", "tabs=fish", "lamp",
	"brick", "brick2", "brick3",
	"metal", "metal2", "planks",
	"planks2", "quake"
};



std::vector<utils::Vertex> vertices;
std::vector<glm::ivec4> indices;
std::vector<utils::Model> models;


glm::ivec2 currentScreenRes;
unordered_map<int, bool> keyMap = {};


void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);

	currentScreenRes = glm::ivec2(width, height);
}


int main() {
	try { //Catch exceptions
	double cursorXPos, cursorYPos, cursorXPosPrev, cursorYPosPrev;
	currentScreenRes = display::SCREEN_RESOLUTION;


	GLFWwindow* Window = render::initializeWindow(currentScreenRes.x, currentScreenRes.y, "Software-Renderer/main");
	glfwSetFramebufferSizeCallback(Window, framebuffer_size_callback);
	glfwGetCursorPos(Window, &cursorXPos, &cursorYPos);
	glEnable(GL_BLEND);

	cursorXPosPrev = cursorXPos;
	cursorYPosPrev = cursorYPos;
	utils::GLErrorcheck("Window Creation", true);


	Camera camera;


	//Load models.
	render::loadModel(
		"LoPoly_FoodTray.obj",
		&vertices, &indices, &models,
		-1,
		glm::vec3(0, 2, 0), glm::vec3(constants::PI/2, 0, 0), glm::vec3(2,2,2)
	);
	render::loadModel(
		"LoPoly_SteelCable.obj",
		&vertices, &indices, &models,
		-1,
		glm::vec3(-2, 0, 0)
	);
	/*
	render::loadModel(
		"wheel_test.obj",
		&vertices, &indices, &models,
		-1,
		glm::vec3(2, 0, 0)
	);
	*/


	//Print metrics.
	std::cout << "Vertices: " << vertices.size() << std::endl;
	std::cout << "Triangles: " << indices.size() << std::endl;
	std::cout << "Models: " << models.size() << std::endl;



	GLuint textureArray = render::createTexture2DArray(textureNames);
	GLuint vertexSSBO = render::createVertexSSBO(vertices.size());
	GLuint indexSSBO = render::createIndexSSBO(indices.size());
	GLuint modelSSBO = render::createModelSSBO(models.size());
	GLuint renderedFrameID = render::createTexture2D(display::RENDER_RESOLUTION.x, display::RENDER_RESOLUTION.y);

	//Geometry shader
	GLuint geoShader = render::createShaderProgram("geometry", false, true, "miniShader.glsl");

	//Display Shader
	GLuint displayShader = render::createShaderProgram("display");



	glViewport(0, 0, currentScreenRes.x, currentScreenRes.y);
	glDisable(GL_DEPTH_TEST);
	GLuint VAO = render::getVAO();


	utils::GLErrorcheck("Initialisation", true);


	glm::mat4 projMatrix = render::projectionMatrix(camera);
	glm::mat4 viewMatrix, pvMatrix;


	//Initialize keyMap for input tracking
	for (int key : monitoredKeys) {
		keyMap[key] = false;
	}
	float verticalFOV = constants::TO_DEG * 2 * atan(tan(radians(camera.FOV / 2.0f)) * (display::RENDER_RESOLUTION.x / display::RENDER_RESOLUTION.y));

	int tick = 0;
	while (!glfwWindowShouldClose(Window)) {
		double frameStart = glfwGetTime();
		glfwPollEvents();

		// Get inputs for this frame
		for (int key : monitoredKeys) {
			int keyState = glfwGetKey(Window, key);
			if (keyState == GLFW_PRESS) {
				keyMap[key] = true;

			} else if (keyState == GLFW_RELEASE) {
				keyMap[key] = false;
			}
		}


		if (keyMap[GLFW_KEY_ESCAPE]) {
			break; //Quit
		}

		if (keyMap[GLFW_KEY_1]) {
			glfwSetInputMode(Window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);			
		} else {
			glfwSetInputMode(Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			glfwGetCursorPos(Window, &cursorXPos, &cursorYPos);
		}

		float camSpeed = config::CAMERA_MOVE_SPEED;
		if (keyMap[GLFW_KEY_LEFT_CONTROL]) {camSpeed *= config::CAMERA_MOVE_MULT_SLOW;}
		if (keyMap[GLFW_KEY_LEFT_SHIFT]) {camSpeed *= config::CAMERA_MOVE_MULT_FAST;}
		if (keyMap[GLFW_KEY_LEFT_ALT]) {camSpeed *= config::CAMERA_MOVE_MULT_FASTER;}


		if (keyMap[GLFW_KEY_W]) {
			camera.position.x += camSpeed * sin(camera.angle.x);
			camera.position.y += camSpeed * cos(camera.angle.x);
		}
		if (keyMap[GLFW_KEY_S]) {
			camera.position.x -= camSpeed * sin(camera.angle.x);
			camera.position.y -= camSpeed * cos(camera.angle.x);
		}
		if (keyMap[GLFW_KEY_D]) {
			camera.position.x += camSpeed * sin(camera.angle.x + constants::HALF_PI);
			camera.position.y += camSpeed * cos(camera.angle.x + constants::HALF_PI);
		}
		if (keyMap[GLFW_KEY_A]) {
			camera.position.x -= camSpeed * sin(camera.angle.x + constants::HALF_PI);
			camera.position.y -= camSpeed * cos(camera.angle.x + constants::HALF_PI);
		}
		if (keyMap[GLFW_KEY_E]) {
			camera.position.z += camSpeed;
		}
		if (keyMap[GLFW_KEY_Q]) {
			camera.position.z -= camSpeed;
		}



		double cursorXDelta = cursorXPos - cursorXPosPrev;
		double cursorYDelta = cursorYPos - cursorYPosPrev;
		camera.angle.x += cursorXDelta * (config::TURN_SPEED_CURSOR);
		camera.angle.y -= cursorYDelta * (config::TURN_SPEED_CURSOR);
		camera.angle.x = fmod(camera.angle.x + constants::TWO_PI, constants::TWO_PI);
		camera.angle.y = glm::clamp(camera.angle.y, 0.01f-constants::HALF_PI, constants::HALF_PI-0.01f);


		viewMatrix = render::viewMatrix(camera);
		pvMatrix = projMatrix * viewMatrix;

		render::updateVertexSSBO(vertexSSBO, &vertices);
		render::updateIndexSSBO(indexSSBO, &indices);
		render::updateModelSSBO(modelSSBO, &models, pvMatrix);





		//Geometry Shader.
		glViewport(0, 0, display::RENDER_RESOLUTION.x, display::RENDER_RESOLUTION.y);
		glUseProgram(geoShader);
		glBindImageTexture(0, renderedFrameID, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
		glBindTextureUnit(0, textureArray);

		GLint nModelsLocation = glGetUniformLocation(geoShader, "numModels");
		GLint zNearLocation = glGetUniformLocation(geoShader, "zNear");
		GLint zFarLocation = glGetUniformLocation(geoShader, "zFar");
		glUniform1i(nModelsLocation, models.size());
		glUniform1f(zNearLocation, camera.nearZ);
		glUniform1f(zFarLocation, camera.farZ);

		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
		glBindVertexArray(0);
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
		utils::GLErrorcheck("Geometry Shader", false);



		//Display Shader and update screen.
		glViewport(0, 0, currentScreenRes.x, currentScreenRes.y);
		glUseProgram(displayShader);
		glBindTextureUnit(0, renderedFrameID);

		GLuint screenResLoc = glGetUniformLocation(displayShader, "screenResolution");
		GLuint texResLoc = glGetUniformLocation(displayShader, "renderResolution");
		glUniform2i(screenResLoc, currentScreenRes.x, currentScreenRes.y);
		glUniform2i(texResLoc, display::RENDER_RESOLUTION.x, display::RENDER_RESOLUTION.y);

		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
		glBindVertexArray(0);
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

		glfwSwapBuffers(Window);
		utils::GLErrorcheck("Display Shader", true);



		while (glfwGetTime() - frameStart < constants::DT) {}
		if (dev::SHOW_FREQ > 0) {
			double totalTime = (glfwGetTime() - frameStart);
			std::cout << floor(1/totalTime) << std::endl;
		}


		cursorXPosPrev = cursorXPos;
		cursorYPosPrev = cursorYPos;
		tick++;
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
