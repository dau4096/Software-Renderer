#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
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



utils::FrameBuffer frameBuffer;
std::vector<glm::vec3> vertices;
std::vector<glm::ivec4> indices; //3 vertex indices and a texture index (later)
std::vector<glm::vec4> projectedVertices;
std::vector<utils::Edge> edges;


glm::ivec2 currentScreenRes;
unordered_map<int, bool> keyMap = {};


std::array<glm::uvec3, 7> colourList = {
	glm::uvec3(255, 0, 0),
	glm::uvec3(0, 255, 0),
	glm::uvec3(0, 0, 255),
	glm::uvec3(255, 255, 0),
	glm::uvec3(0, 255, 255),
	glm::uvec3(255, 0, 255),
	glm::uvec3(255, 255, 255),
};


void prepGeometry() {
	vertices.push_back(glm::vec3(-1.0f, 0.0f, 0.0f));
	vertices.push_back(glm::vec3(-1.0f, 0.0f, 1.0f));
	vertices.push_back(glm::vec3( 0.0f, 0.0f, 1.0f));
	vertices.push_back(glm::vec3( 0.0f, 0.0f, 0.0f));
	indices.push_back(glm::ivec4(0,1,2, 0));
	indices.push_back(glm::ivec4(0,2,3, 1));



	vertices.push_back(glm::vec3( 1.0f, 0.0f, 0.0f));
	vertices.push_back(glm::vec3( 1.0f, 0.0f, 1.0f));
	vertices.push_back(glm::vec3( 2.0f,-1.0f, 1.0f));
	vertices.push_back(glm::vec3( 2.0f,-1.0f, 0.0f));
	indices.push_back(glm::ivec4(4,5,6, 2));
	indices.push_back(glm::ivec4(4,6,7, 3));
}


void GLFWFrameBufferSizeCallback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);

	currentScreenRes = glm::ivec2(width, height);
}


bool compareEdges(utils::Edge* a, utils::Edge* b) {
	return a->currentX < b->currentX;
}


int main() {
	try { //Catch exceptions
	double cursorXPos, cursorYPos, cursorXPosPrev, cursorYPosPrev;
	currentScreenRes = display::SCREEN_RESOLUTION;


	GLFWwindow* Window = render::initializeWindow(currentScreenRes.x, currentScreenRes.y, "Software-Renderer/Quake-Span-Renderer");
	glfwSetFramebufferSizeCallback(Window, GLFWFrameBufferSizeCallback);
	glfwGetCursorPos(Window, &cursorXPos, &cursorYPos);
	glEnable(GL_BLEND);

	cursorXPosPrev = cursorXPos;
	cursorYPosPrev = cursorYPos;
	utils::GLErrorcheck("Window Creation", true);


	Camera camera;


	prepGeometry();


	//Print metrics.
	std::cout << "Vertices: " << vertices.size() << std::endl;
	std::cout << "Triangles: " << indices.size() << std::endl;

	projectedVertices = std::vector<glm::vec4>(vertices.size());
	edges = std::vector<utils::Edge>(indices.size() * 3); //3 Edges for every triangle.



	frameBuffer = FrameBuffer(display::RENDER_RESOLUTION);
	frameBuffer.updateGLTexture();



	//Display Shader
	GLuint displayShader = render::createShaderProgram("display");



	glViewport(0, 0, currentScreenRes.x, currentScreenRes.y);
	glDisable(GL_DEPTH_TEST);
	GLuint VAO = render::getVAO();


	utils::GLErrorcheck("Initialisation", true);


	glm::mat4 projMatrix = render::projectionMatrix(camera);
	glm::mat4 modelMatrix = render::modelMatrix();
	glm::mat4 viewMatrix, pvmMatrix;



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
		camera.angle.y += cursorYDelta * (config::TURN_SPEED_CURSOR);
		camera.angle.x = fmod(camera.angle.x + constants::TWO_PI, constants::TWO_PI);
		camera.angle.y = glm::clamp(camera.angle.y, 0.01f-constants::HALF_PI, constants::HALF_PI-0.01f);


		viewMatrix = render::viewMatrix(camera);
		pvmMatrix = projMatrix * viewMatrix * modelMatrix;



		//Quake-style rendering loops
		//Project vertices.
		size_t vIndex = 0; //Could be a compute shader later.
		for (glm::vec3 vertex : vertices) {
			glm::vec4 proj = render::project(vertex, pvmMatrix);
			if (proj == constants::INVALIDv4) {
				projectedVertices[vIndex] = glm::vec4(-1.0f, -1.0f, -1.0f, 1.0f);
			} else {
				projectedVertices[vIndex] = proj;
			}
			vIndex++;
		}


		//Create edges.
		size_t tIndex = 0;
		for (glm::ivec4 compIndex : indices) {
			glm::vec4 A = projectedVertices[compIndex.x];
			glm::vec4 B = projectedVertices[compIndex.y];
			glm::vec4 C = projectedVertices[compIndex.z];

			glm::vec2 AB = glm::vec2(B - A);
			glm::vec2 AC = glm::vec2(C - A);
			glm::vec2 An = glm::vec2(AC.y, -AC.x);
			if (dot(An, AB) < 0.0f) {
				//Invalid winding order, must be viewed from back.
				//Blank edges.
				edges[(tIndex * 3) + 0] = utils::Edge();
				edges[(tIndex * 3) + 1] = utils::Edge();
				edges[(tIndex * 3) + 2] = utils::Edge();
			} else {
				/*
				A triangle has 3 edges. Add to the edges buffer.
				  Λ
				1/ \2
				/___\
				  3
				*/
				edges[(tIndex * 3) + 0] = utils::Edge(glm::vec3(A), glm::vec3(B), compIndex.w);
				edges[(tIndex * 3) + 1] = utils::Edge(glm::vec3(B), glm::vec3(C), compIndex.w);
				edges[(tIndex * 3) + 2] = utils::Edge(glm::vec3(C), glm::vec3(A), compIndex.w);
			}
			tIndex++;
		}

		//Sort edges.
		std::array<std::vector<utils::Edge*>, display::RENDER_RESOLUTION.y> edgeAdditions;
		std::array<std::vector<utils::Edge*>, display::RENDER_RESOLUTION.y> edgeRemovals;
		std::vector<utils::Edge*> activeEdgesList; //Active edges, based on the above 2 vectors.
		for (utils::Edge& e : edges) {
			int yMin = static_cast<int>(std::floor(e.start.y));
			int yMax = glm::min(static_cast<int>(std::floor(e.end.y)), display::RENDER_RESOLUTION.y);
			if (yMin == yMax) {continue; /* Ignore horizontal edges. */}

			//Find start of edge and add to relevant line of the additions vector.
			if (yMin >= 0 && yMin < display::RENDER_RESOLUTION.y) {
				edgeAdditions[yMin].emplace_back(&e);
			}

			//Find end of edge and add to relevant line of the additions vector.
			if (yMax >= 0) {
				edgeRemovals[yMax].emplace_back(&e);
			}
		}

		for (size_t yScan=0; yScan<display::RENDER_RESOLUTION.y; yScan++) {
			//Add new lines that start on this scanline.
			for (utils::Edge* e : edgeAdditions[yScan]) {
				activeEdgesList.push_back(e);
			}

			//Remove lines that stop on this scanline
			for (utils::Edge* e : edgeRemovals[yScan]) {
				auto it = std::find(activeEdgesList.begin(), activeEdgesList.end(), e);
				if (it != activeEdgesList.end()) {
					activeEdgesList.erase(it);
				}
			}

			//Handle the lines?
			size_t numActiveEdges = activeEdgesList.size();
			if (numActiveEdges < 1) {continue; /* No active edges for this scanline. */}
			std::vector<utils::Span> spans;

			for (utils::Edge* edge : activeEdgesList) {
				edge->calculateXPosition(yScan);
				if (dev::SHOW_EDGES) {frameBuffer.setPX(edge->currentX, yScan, glm::uvec3(255, 0, 255));}
			}
			//Sort by left-to-right onscreen.
			std::sort(activeEdgesList.begin(), activeEdgesList.end(), compareEdges);

			//Create spans based on each edge in the scanline.
			utils::Edge* prevEdge = nullptr;
			for (utils::Edge* thisEdge : activeEdgesList) {
				if (prevEdge == nullptr) {
					prevEdge = thisEdge;
					continue;
				}

				if (prevEdge->triIndex == thisEdge->triIndex) {
					//Closes off previous triangle.
					spans.push_back(utils::Span(
						prevEdge->currentX, yScan,
						size_t(floor(thisEdge->currentX - prevEdge->currentX)),
						thisEdge->triIndex
					));
					prevEdge = nullptr;
					continue; //Renders void from here.
				}

				//Another triangle's edge has come next instead.
				//Do some Z testing with ->currentZ values to get thisEdgeCloser value.
				bool thisEdgeCloser = ( //Not even close to correct. But works for testing.
					prevEdge->currentZ > thisEdge->currentZ
				);

				if (thisEdgeCloser) {
					spans.push_back(utils::Span(
						prevEdge->currentX, yScan,
						size_t(floor(thisEdge->currentX - prevEdge->currentX)),
						thisEdge->triIndex
					));
					prevEdge = thisEdge;
					continue;
				}
			}


			if (spans.size() < 1) {continue; /* No spans to draw. */}
			size_t sIdxTMP = 0;
			for (utils::Span span : spans) {
				glm::uvec3 colour = (sIdxTMP > 0) ? glm::uvec3(255, 0, 0) : glm::uvec3(0, 255, 0);
				frameBuffer.drawSpan(&span, &colourList);
				sIdxTMP++;
			}
		}

		frameBuffer.updateGLTexture();




		//Display Shader and update screen.
		glViewport(0, 0, currentScreenRes.x, currentScreenRes.y);
		glUseProgram(displayShader);
		glBindTextureUnit(0, frameBuffer.GLTextureID);

		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
		glBindVertexArray(0);
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

		glfwSwapBuffers(Window);
		utils::GLErrorcheck("Display Shader", true);



		while (glfwGetTime() - frameStart < constants::DT) {}
		if (dev::SHOW_FREQ) {
			double totalTime = (glfwGetTime() - frameStart);
			std::cout << floor(1/totalTime) << std::endl;
		}


		cursorXPosPrev = cursorXPos;
		cursorYPosPrev = cursorYPos;
		tick++;

		//frameBuffer.writeToPNG("testing.png");
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
