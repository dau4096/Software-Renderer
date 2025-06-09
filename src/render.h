#ifndef RENDER_H
#define RENDER_H

#include "includes.h"
#include "utils.h"
#include <array>

namespace render {
	GLFWwindow* initializeWindow(int width, int height, const char* title);
	GLuint createShaderProgram(std::string name, bool hasVertexSource=true, bool hasInclude=false, std::string includeName="");


	glm::mat4 projectionMatrix(utils::Camera& camera);
	glm::mat4 viewMatrix(utils::Camera& camera);
	glm::mat4 modelMatrix(glm::vec3 pos=glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 rot=glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 scale=glm::vec3(1.0f, 1.0f, 1.0f));
	glm::vec4 project(glm::vec3 vertex, glm::mat4 pvmMatrix);


	GLuint createTexture2D(int width, int height, GLint imageFormat=GL_RGBA32F);
	GLuint loadTextureFile(std::string fileName);
	GLuint createTexture2DArray(std::array<std::string, display::TEXTURE_ARRAY_MAX_LAYERS>& textureNames);


	GLuint getVAO();
}

#endif
