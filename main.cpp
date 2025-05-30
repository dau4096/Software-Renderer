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
	GLFW_KEY_LEFT_SHIFT,
	GLFW_KEY_LEFT_ALT
};


std::array<std::string, display::TEXTURE_ARRAY_MAX_LAYERS> textureNames = {
	"a", "b", "c",
	"mus2", "osa", "piloten", "s_t_a_r_e",
	"switch", "tabs=fish", "lamp",
	"brick", "brick2", "brick3"
	"metal", "metal2", "planks",
	"planks2", "quake"
};



std::array<utils::Triangle, constants::MAX_TRIANGLES> triangleData;
glm::mat4 pvmMatrix;


glm::ivec2 currentScreenRes;
unordered_map<int, bool> keyMap = {};


void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);

	currentScreenRes = glm::ivec2(width, height);
}




void tmpFillTris(std::array<utils::Triangle, constants::MAX_TRIANGLES>* triangleData) {
	triangleData->at(0) = Triangle(
		glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec2(-1.0f, 1.0f), 	//vA
		glm::vec3(-1.0f, 0.0f, 1.0f), glm::vec2(-1.0f, 0.0f), 	//vB
		glm::vec3( 0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 0.0f), 	//vC
		0 	//texID
	);
	triangleData->at(1) = Triangle(
		glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec2(-1.0f, 1.0f), 	//vA
		glm::vec3( 0.0f, 0.0f, 0.0f), glm::vec2(0.0f, 1.0f), 	//vB
		glm::vec3( 0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 0.0f), 	//vC
		1 	//texID
	);


	triangleData->at(2) = Triangle(
		glm::vec3( 1.0f, 0.0f, 0.0f), glm::vec2(-1.0f, 1.0f), 	//vA
		glm::vec3( 1.0f, 0.0f, 1.0f), glm::vec2(-1.0f, 0.0f), 	//vB
		glm::vec3( 2.0f,-1.0f, 1.0f), glm::vec2(0.0f, 0.0f), 	//vC
		5 	//texID
	);

	triangleData->at(3) = Triangle(
		glm::vec3( 1.0f, 0.0f, 0.0f), glm::vec2(-1.0f, 1.0f), 	//vA
		glm::vec3( 2.0f,-1.0f, 0.0f), glm::vec2(0.0f, 1.0f), 	//vB
		glm::vec3( 2.0f,-1.0f, 1.0f), glm::vec2(0.0f, 0.0f), 	//vC
		5 	//texID
	);
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

	tmpFillTris(&triangleData);

	GLuint textureArray = render::createTexture2DArray(textureNames);


	GLuint triangleSSBO = render::createTriangleSSBO();
	GLuint renderedFrameID = render::createTexture2D(display::RENDER_RESOLUTION.x, display::RENDER_RESOLUTION.y);

	//Geometry shader
	GLuint geoShader = render::createShaderProgram("geometry", false, true, "miniShader.glsl");

	//Display Shader
	GLuint displayShader = render::createShaderProgram("display");



	glViewport(0, 0, currentScreenRes.x, currentScreenRes.y);
	glDisable(GL_DEPTH_TEST);
	GLuint VAO = render::getVAO();


	utils::GLErrorcheck("Initialisation", true);


	glm::mat4 modelMatrix = glm::mat4(1.0f);
	glm::mat4 projMatrix = render::projectionMatrix(camera);
	glm::mat4 viewMatrix;


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



		render::updateTriangleSSBO(triangleSSBO, &triangleData);

		viewMatrix = render::viewMatrix(camera);
		pvmMatrix = projMatrix * viewMatrix * modelMatrix;




		//Geometry Shader.
		glViewport(0, 0, display::RENDER_RESOLUTION.x, display::RENDER_RESOLUTION.y);
		glUseProgram(geoShader);
		glBindImageTexture(0, renderedFrameID, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
		glBindTextureUnit(0, textureArray);

		GLint MVPMatLocation = glGetUniformLocation(geoShader, "pvmMatrix");
		GLint zNearLocation = glGetUniformLocation(geoShader, "zNear");
		GLint zFarLocation = glGetUniformLocation(geoShader, "zFar");
		glUniformMatrix4fv(MVPMatLocation, 1, GL_FALSE, glm::value_ptr(pvmMatrix));
		glUniform1f(zNearLocation, camera.nearZ);
		glUniform1f(zFarLocation, camera.farZ);

		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
		glBindVertexArray(0);
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
		utils::GLErrorcheck("Geometry Shader", true);



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
