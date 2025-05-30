#define TINYOBJLOADER_IMPLEMENTATION
#include "includes.h"
#include "utils.h"
#include "tiny_obj_loader.h"
#include "C:/Users/User/Documents/code/.cpp/stb_image.h"
using namespace std;
using namespace utils;
using namespace glm;


namespace render {



GLFWwindow* initializeWindow(int width, int height, const char* title) {
	if (!glfwInit()) {
		raise("Failed to initialize GLFW");
		return nullptr;
	}


	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);  // Set OpenGL major version
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);  // Set OpenGL minor version
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // Use Core profile


	GLFWwindow* Window = glfwCreateWindow(width, height, title, NULL, NULL);
	if (!Window) {
		glfwTerminate();
		raise("Failed to create GLFW window");
		return nullptr;
	}
	glfwMakeContextCurrent(Window);

	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK) {
		raise("Failed to initialize GLEW.");
	}

	glfwSetInputMode(Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	return Window;
}



GLuint compileShader(GLenum shaderType, std::string filePath, bool hasInclude=false, std::string includeName="") {
	GLuint shader = glCreateShader(shaderType);
	if (shader == 0) {
		raise("Error: Failed to create shader.");
		return 0;
	}


	std::string source = utils::readFile(filePath);
	if (hasInclude && (shaderType == GL_FRAGMENT_SHADER)) {
		size_t includePos = source.find("#include \"" + includeName + "\"");
		if (includePos != std::string::npos) {
			std::string includedShader = utils::readFile(includeName);
			std::string tag = "#include \"" + includeName + "\"";
			source.replace(includePos, tag.length(), includedShader);
		}
	}
	const char* src = source.c_str();
	glShaderSource(shader, 1, &src, nullptr);

	glCompileShader(shader);
	

	GLint success;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success) {
		char infolog[512];
		glGetShaderInfoLog(shader, 512, nullptr, infolog);
		raise("Error: Shader compilation failed;\n" + string(infolog));
	}

	return shader;
}


GLuint createShaderProgram(std::string name, bool hasVertexSource=true, bool hasInclude=false, std::string includeName="") {
	GLuint vertexShader;
	if (hasVertexSource) {
		vertexShader = compileShader(GL_VERTEX_SHADER, "src\\shaders\\"+ name +".vert");
	} else {
		vertexShader = compileShader(GL_VERTEX_SHADER, "src\\shaders\\generic.vert");
	}
	GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, "src\\shaders\\"+ name +".frag", hasInclude, includeName);

	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);

	GLint success;
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success) {
		char infolog[512];
		glGetProgramInfoLog(shaderProgram, 512, nullptr, infolog);
		raise("Error: Program linking failed;\n" + string(infolog));
	}

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	return shaderProgram;
}



glm::mat4 projectionMatrix(utils::Camera& camera) {
	float aspectRatio = static_cast<float>(display::RENDER_RESOLUTION.x) / static_cast<float>(display::RENDER_RESOLUTION.y);
	return glm::perspective(glm::radians(camera.FOV), aspectRatio, camera.nearZ, camera.farZ);
}

glm::mat4 viewMatrix(utils::Camera& camera) {
	glm::vec3 forward = glm::vec3(
		sin(camera.angle.x)*cos(camera.angle.y),
		cos(camera.angle.x)*cos(camera.angle.y),
		sin(camera.angle.y)
	);

	return glm::lookAt(camera.position, camera.position + forward, glm::vec3(0.0f, 0.0f, 1.0f));
}






GLuint createVertexSSBO(size_t size) {
	GLuint vertexSSBO;
	glGenBuffers(1, &vertexSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, vertexSSBO);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(utils::Vertex) * size, nullptr, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, vertexSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	return vertexSSBO;
}

GLuint createIndexSSBO(size_t size) {
	GLuint indexSSBO;
	glGenBuffers(1, &indexSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, indexSSBO);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(glm::ivec4) * size, nullptr, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, indexSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	return indexSSBO;
}

GLuint createModelSSBO(size_t size) {
	GLuint modelSSBO;
	glGenBuffers(1, &modelSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, modelSSBO);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(utils::ModelGPU) * size, nullptr, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, modelSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	return modelSSBO;
}


void updateVertexSSBO(GLuint vertexSSBO, std::vector<utils::Vertex>* vertices) {
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, vertexSSBO);
	glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(utils::Vertex) * vertices->size(), vertices->data());
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void updateIndexSSBO(GLuint indexSSBO, std::vector<glm::ivec4>* indices) {
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, indexSSBO);
	glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(glm::ivec4) * indices->size(), indices->data());
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void updateModelSSBO(GLuint modelSSBO, std::vector<utils::Model>* models, glm::mat4& pvMatrix) {
	std::vector<utils::ModelGPU> bufferData;
	for (utils::Model model : *models) {
		bufferData.push_back(utils::ModelGPU(&model, pvMatrix));
	}

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, modelSSBO);
	glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(utils::ModelGPU) * bufferData.size(), bufferData.data());
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}



void loadModel(
		const std::string& modelFileName, 
		std::vector<utils::Vertex>* globalVertices, std::vector<glm::ivec4>* globalIndices,
		std::vector<utils::Model>* models,
		int textureID,
		glm::vec3 position=glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3 rotation=glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3 scale=glm::vec3(1.0f, 1.0f, 1.0f)
	) {
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string warn;
	std::string filePath = "models/" + modelFileName;

	bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, filePath.c_str(), nullptr, true);

	if (!warn.empty()) std::cout << "TinyOBJ warning: " << warn << std::endl;
	if (!ret) return;

	int baseVertexIndex = globalVertices->size();
	int baseIndexIndex = globalIndices->size();

	for (const auto& shape : shapes) {
		std::unordered_map<int, int> indexMap;

		for (size_t f = 0; f < shape.mesh.num_face_vertices.size(); f++) {
			int fv = shape.mesh.num_face_vertices[f];

			glm::ivec4 triangle;
			for (size_t v = 0; v < fv; v++) {
				tinyobj::index_t idx = shape.mesh.indices[f * fv + v];

				glm::vec3 pos(
					attrib.vertices[3 * idx.vertex_index + 0],
					attrib.vertices[3 * idx.vertex_index + 1],
					attrib.vertices[3 * idx.vertex_index + 2]
				);

				glm::vec2 uv(0.0f);
				if (idx.texcoord_index >= 0) {
					uv = glm::vec2(
						attrib.texcoords[2 * idx.texcoord_index + 0],
						attrib.texcoords[2 * idx.texcoord_index + 1]
					);
				}

				utils::Vertex vertex(pos, uv);

				globalVertices->push_back(vertex);
				triangle[v] = globalVertices->size() - 1;
			}

			globalIndices->push_back(triangle);
		}

		int start = baseIndexIndex;
		int end = globalIndices->size() - 1;

		models->emplace_back(
			position, rotation, scale, start, end, textureID
		);
	}
}







GLuint createTexture2D(int width, int height, GLint imageFormat=GL_RGBA32F) {
	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);

	glTexStorage2D(GL_TEXTURE_2D, 1, imageFormat, width, height);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glBindTexture(GL_TEXTURE_2D, 0);

	return textureID;
}


GLuint loadTextureFile(std::string fileName) {
	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	int width, height, channels;


	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, display::TEXTURE_RESOLUTION.x, display::TEXTURE_RESOLUTION.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);


	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);


	std::string texturePath = "heightmaps/" + fileName + ".png";
	unsigned char* textureData = stbi_load(texturePath.c_str(), &width, &height, &channels, 4);

	if (!textureData) {
		std::cerr << "Failed to load image " << texturePath << ": " << stbi_failure_reason() << std::endl;
		glBindTexture(GL_TEXTURE_2D, 0);
		glDeleteTextures(1, &textureID);
		return 0; //Indicate failure
	}


	if (width != display::TEXTURE_RESOLUTION.x || height != display::TEXTURE_RESOLUTION.y) {
		std::cerr << "Texture " << fileName << " has incorrect dimensions (" << width << "x" << height << "). Expected "
				  << display::TEXTURE_RESOLUTION.x << "x" << display::TEXTURE_RESOLUTION.y << "." << std::endl;
		stbi_image_free(textureData);
		return 0;
	}


	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, display::TEXTURE_RESOLUTION.x, display::TEXTURE_RESOLUTION.y, GL_RGBA, GL_UNSIGNED_BYTE, textureData);


	stbi_image_free(textureData);



	glBindTexture(GL_TEXTURE_2D, 0);

	return textureID;
}



GLuint createTexture2DArray(std::array<std::string, display::TEXTURE_ARRAY_MAX_LAYERS>& textureNames) {
	GLuint sheetArrayID;
	glGenTextures(1, &sheetArrayID);
	glBindTexture(GL_TEXTURE_2D_ARRAY, sheetArrayID);


	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA8, display::TEXTURE_RESOLUTION.x, display::TEXTURE_RESOLUTION.y, display::TEXTURE_ARRAY_MAX_LAYERS, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);


	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);




	int fallbackTextureWidth, fallbackTextureHeight, fallbackTextureChannels;
	bool usedFallback;

	unsigned char* fallbackTextureData = stbi_load(
		display::FALLBACK_TEXTURE_PATH,
		&fallbackTextureWidth, &fallbackTextureHeight,
		&fallbackTextureChannels, 4
	);

	if (!fallbackTextureData) {
		std::cerr << "Failed to load fallback texture : " << stbi_failure_reason() << std::endl;
		glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
		glDeleteTextures(1, &sheetArrayID);
		return 0;	
	}



	int width, height, channels;
	int layerIndex = 0;
	for (const std::string& textureName : textureNames) {
		if (textureName.empty()) continue;
		usedFallback = false;

		std::string reportedTextureName = textureName;
		std::string texturePath = "textures/" + textureName + ".png";
		unsigned char* textureData = stbi_load(
			texturePath.c_str(),
			&width, &height,
			&channels, 4
		);

		if (!textureData) {
			//Use fallback texture.
			textureData = fallbackTextureData;
			width = fallbackTextureWidth;
			height = fallbackTextureHeight;
			channels = fallbackTextureChannels;
			reportedTextureName = "FALLBACK_TEXTURE";
			usedFallback = true;
		}


		if (width != display::TEXTURE_RESOLUTION.x || height != display::TEXTURE_RESOLUTION.y) {
			std::cerr << "Texture " << reportedTextureName << " has incorrect dimensions (" << width << "x" << height << "). Expected "
					  << display::TEXTURE_RESOLUTION.x << "x" << display::TEXTURE_RESOLUTION.y << "." << std::endl;
			stbi_image_free(textureData);
			continue;
		}


		glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, layerIndex, display::TEXTURE_RESOLUTION.x, display::TEXTURE_RESOLUTION.y, 1, GL_RGBA, GL_UNSIGNED_BYTE, textureData);


		if (!usedFallback) {
			stbi_image_free(textureData);
		}

		layerIndex++;
		if (layerIndex >= display::TEXTURE_ARRAY_MAX_LAYERS) break;
	}

	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

	stbi_image_free(fallbackTextureData);

	return sheetArrayID;
}





GLuint getVAO() {
	const float vertices[] = {
		-1.0f, -1.0f, 0.0f,  0.0f, 0.0f,  //Bottom-left
		 1.0f, -1.0f, 0.0f,  1.0f, 0.0f,  //Bottom-right
		-1.0f,  1.0f, 0.0f,  0.0f, 1.0f,  //Top-left
		 1.0f,  1.0f, 0.0f,  1.0f, 1.0f,  //Top-right
	};

	const int indices[] = {
		0, 1, 2,
		2, 3, 1,
	};

	// Create VAO (Vertex Array Object) to store all vertex state
	GLuint VAO;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	// Create VBO (Vertex Buffer Object) to store vertex data
	GLuint VBO;
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	// Fill the buffer with vertex data (positions + texture coordinates)
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	GLuint EBO;
	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// Define the position attribute (location = 0)
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// Define the texture coordinate attribute (location = 1)
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0); // Unbind VAO

	return VAO;
}



}