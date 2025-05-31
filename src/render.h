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
	

	GLuint createVertexSSBO(size_t size);
	GLuint createIndexSSBO(size_t size);
	GLuint createModelSSBO(size_t size);
	void updateVertexSSBO(GLuint vertexSSBO, std::vector<utils::Vertex>* vertices);
	void updateIndexSSBO(GLuint indexSSBO, std::vector<glm::ivec4>* indices);
	void updateModelSSBO(GLuint modelSSBO, std::vector<utils::Model>* models, glm::mat4& pvMatrix);
	void loadModel(
		const std::string& modelFileName, 
		std::vector<utils::Vertex>* globalVertices, std::vector<glm::ivec4>* globalIndices,
		std::vector<utils::Model>* models,
		int textureID,
		glm::vec3 position=glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3 rotation=glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3 scale=glm::vec3(1.0f, 1.0f, 1.0f)
	);


	GLuint createTexture2D(int width, int height, GLint imageFormat=GL_RGBA32F);
	GLuint loadTextureFile(std::string fileName);
	GLuint createTexture2DArray(std::array<std::string, display::TEXTURE_ARRAY_MAX_LAYERS>& textureNames);


	GLuint getVAO();
}

#endif
