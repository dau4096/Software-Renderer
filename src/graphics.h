#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "includes.h"
#include "global.h"

namespace graphics {

	GLFWwindow* initializeWindow(int width, int height, const char* title);
	GLuint createShaderProgram(std::string name, bool hasVertexSource=true, bool hasInclude=false, std::string includeName="");
	void prepareGraphics();


	glm::mat4 projectionMatrix(structs::Camera& camera);
	glm::mat4 viewMatrix(structs::Camera& camera);
	glm::mat4 modelMatrix(glm::vec3 pos=glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 rot=glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 scale=glm::vec3(1.0f, 1.0f, 1.0f));
	glm::vec4 project(glm::vec3 vertex, glm::mat4 pvmMatrix);


	GLuint loadTextureFile(std::string fileName);

}


namespace frame {

	void draw();
	void updateCamera();

}

#endif
