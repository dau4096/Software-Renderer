#include "includes.h"
#include "global.h"
#include "utils.h"
#include "stb_image.h"
using namespace std;
using namespace utils;
using namespace glm;


namespace graphics {

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

	//Create VAO
	GLuint VAO;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	//Create VBO
	GLuint VBO;
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	//Fill the buffer with vertex data (positions + texture coordinates)
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	GLuint EBO;
	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	//Define position attr (location 0)
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	//Define tex-coord attr (location 1)
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	return VAO;
}



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



glm::mat4 getProjectionMatrix(structs::Camera& camera) {
	float aspectRatio = static_cast<float>(display::RENDER_RESOLUTION.x) / static_cast<float>(display::RENDER_RESOLUTION.y);
	return glm::perspective(camera.FOV, aspectRatio, camera.nearZ, camera.farZ);
}


glm::mat4 getViewMatrix(structs::Camera& camera) {
	glm::vec3 forward = glm::vec3(
		sin(camera.angle.x)*cos(camera.angle.y),
		cos(camera.angle.x)*cos(camera.angle.y),
		sin(camera.angle.y)
	);

	return glm::lookAt(camera.position, camera.position + forward, glm::vec3(0.0f, 0.0f, 1.0f));
}


glm::mat4 getModelMatrix(glm::vec3 pos=glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 rot=glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 scale=glm::vec3(1.0f, 1.0f, 1.0f)) {
	glm::mat4 translationMat = glm::mat4(
		1.0f, 	0.0f, 	0.0f, 	0.0f,
		0.0f, 	1.0f, 	0.0f, 	0.0f,
		0.0f, 	0.0f, 	1.0f, 	0.0f,
		pos.x, 	pos.y, 	pos.z, 	1.0f
	);

	float sx = sin(rot.x), cx = cos(rot.x);
	float sy = sin(rot.y), cy = cos(rot.y);
	float sz = sin(rot.z), cz = cos(rot.z);
	glm::mat4 rotationMat = glm::mat4(
		cy*cz, cy*sz, -sy, 0.0f,
		sx*sy*cz-cx*sz, sx*sy*sz+cx*cz, sx*cy, 0.0f,
		cx*sy*cz+sx*sz, cx*sy*sz-sx*cz, cx*cy, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);

	glm::mat4 scaleMat = glm::mat4(
		scale.x,	0.0f, 		0.0f,		0.0f, 
		0.0f, 		scale.y,	0.0f, 		0.0f, 
		0.0f, 		0.0f, 		scale.z,	0.0f, 
		0.0f, 		0.0f, 		0.0f, 		1.0f
	);

	return translationMat * rotationMat * scaleMat;
}



void placeholderPrepareGeometry() {
	vertices.push_back(glm::vec3(-1.0f, 0.0f, 0.0f));
	vertices.push_back(glm::vec3(-1.0f, 0.0f, 1.0f));
	vertices.push_back(glm::vec3( 0.0f, 0.0f, 1.0f));
	vertices.push_back(glm::vec3( 0.0f, 0.0f, 0.0f));
	indices.push_back(glm::ivec4(0,1,2, 0));
	//indices.push_back(glm::ivec4(0,2,3, 1));



	vertices.push_back(glm::vec3( 1.0f, 0.0f, 0.0f));
	vertices.push_back(glm::vec3( 1.0f, 0.0f, 1.0f));
	vertices.push_back(glm::vec3( 2.0f,-1.0f, 1.0f));
	vertices.push_back(glm::vec3( 2.0f,-1.0f, 0.0f));
	indices.push_back(glm::ivec4(4,5,6, 2));
	//indices.push_back(glm::ivec4(4,6,7, 3));



	vertices.push_back(glm::vec3(-1.0f,-1.0f, 0.0f));
	vertices.push_back(glm::vec3(-1.0f,-1.0f, 1.0f));
	vertices.push_back(glm::vec3( 0.0f,-1.0f, 1.0f));
	vertices.push_back(glm::vec3( 0.0f,-1.0f, 0.0f));
	//indices.push_back(glm::ivec4(8, 9,10, 4));
	indices.push_back(glm::ivec4(8,10,11, 5));
}


void prepareGraphics() {
	glEnable(GL_BLEND);

	placeholderPrepareGeometry();
	projMatrix = getProjectionMatrix(camera);
	modelMatrix = getModelMatrix();

	//Print some metrics.
	std::cout << "Vertices: " << vertices.size() << std::endl;
	std::cout << "Triangles: " << indices.size() << std::endl;

	projectedVertices = std::vector<glm::vec4>(vertices.size());
	//3 Edges for every triangle maximum.
	//It may end up being 2 in reality of one edge is perfectly horizontal but such cases are rare and should assume all 3 are not horizontal.
	//Such edges can simply not be added later and so are not processed.
	edges = std::vector<structs::Edge>(indices.size() * 3u);



	frameBuffer = structs::FrameBuffer(display::RENDER_RESOLUTION);
	frameBuffer.updateGLTexture();

	//Display Shader
	GLIndex::displayShader = createShaderProgram("display");

	glViewport(0, 0, currentScreenRes.x, currentScreenRes.y);
	glDisable(GL_DEPTH_TEST);
	GLIndex::screenspaceVAO = getVAO();

	verticalFOV = 2 * atan(tan(camera.FOV / 2.0f) * (display::RENDER_RESOLUTION.x / display::RENDER_RESOLUTION.y));

	utils::GLErrorcheck("Initialisation", true);
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


	std::string texturePath = "textures/" + fileName + ".png";
	unsigned char* textureData = stbi_load(texturePath.c_str(), &width, &height, &channels, 4);

	if (!textureData) {
		std::cerr << "Failed to load image " << texturePath << ": " << stbi_failure_reason() << std::endl;
		glBindTexture(GL_TEXTURE_2D, 0);
		glDeleteTextures(1, &textureID);
		return 0; //Indicate failure
	}


	if (width != display::TEXTURE_RESOLUTION.x || height != display::TEXTURE_RESOLUTION.y) {
		std::cerr << "Texture " << fileName << " has incorrect dimensions (" << width << "x" << height << "). Expected " << display::TEXTURE_RESOLUTION.x << "x" << display::TEXTURE_RESOLUTION.y << "." << std::endl;
		stbi_image_free(textureData);
		return 0;
	}


	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, display::TEXTURE_RESOLUTION.x, display::TEXTURE_RESOLUTION.y, GL_RGBA, GL_UNSIGNED_BYTE, textureData);


	stbi_image_free(textureData);



	glBindTexture(GL_TEXTURE_2D, 0);

	return textureID;
}





}





namespace frame {

glm::vec4 project(glm::vec3 vertex, glm::mat4 pvmMatrix) { //Could be moved to compute shader later, if needed.
	glm::vec4 vertexV4 = glm::vec4(vertex, 1.0f);
	glm::vec4 ndc = pvmMatrix * vertexV4;
	ndc.w = glm::max(1e-3f, ndc.w);
	ndc /= glm::vec4(ndc.w, ndc.w, ndc.w, 1.0f);
	return glm::vec4(
		(ndc.x + 1.0f) / 2.0f * display::RENDER_RESOLUTION.x,
		(1.0f - ndc.y) / 2.0f * display::RENDER_RESOLUTION.y,
		ndc.z * ndc.w, ndc.w
	);
}


void projectVertices() {
	//Project vertices.
	size_t vIndex = 0; //Could be a compute shader later.
	for (glm::vec3 vertex : vertices) {
		glm::vec4 proj = project(vertex, pvmMatrix);
		if (proj == constants::INVALIDv4) {
			projectedVertices[vIndex] = glm::vec4(-1.0f, -1.0f, -1.0f, 1.0f);
		} else {
			projectedVertices[vIndex] = proj;
		}
		vIndex++;
	}
}


void handleEdges(
		std::array<std::vector<structs::Edge*>, display::RENDER_RESOLUTION.y>* edgeAdditions,
		std::array<std::vector<structs::Edge*>, display::RENDER_RESOLUTION.y>* edgeRemovals
	) {
	//Create edges.
	size_t tIndex = 0;
	for (glm::ivec4 compIndex : indices) {
		glm::vec4 A = projectedVertices[compIndex.x];
		glm::vec4 B = projectedVertices[compIndex.y];
		glm::vec4 C = projectedVertices[compIndex.z];


		glm::vec2 AB = glm::vec2(B - A);
		glm::vec2 AC = glm::vec2(C - A);
		glm::vec2 An = glm::vec2(AC.y, -AC.x);
		bool isBackface = dot(An, AB) < 0.0f;
		if (isBackface && !dev::DRAW_BACKFACES) {continue;}


		//Mark edges as screenspace left (starting a triangle) or right (ending a triangle)
		std::array<glm::vec4,3> verts = {A,B,C};
		std::sort(verts.begin(), verts.end(), [](auto& v1, auto& v2) {return v1.y < v2.y;});


		structs::Edge longEdge = structs::Edge(verts[0], verts[2], compIndex.w, true);
		structs::Edge shortEdgeA = structs::Edge(verts[0], verts[1], compIndex.w, false);
		structs::Edge shortEdgeB = structs::Edge(verts[1], verts[2], compIndex.w, false);

		float v1X = std::round(verts[1].x);
		longEdge.calculateYScanValues(static_cast<size_t>(std::round(verts[1].y)));
		if (v1X < longEdge.currentX) {
			shortEdgeA.isLeftEdge = true; 
			shortEdgeB.isLeftEdge = true;
			longEdge.isLeftEdge = false;
		} else {
			shortEdgeA.isLeftEdge = false; 
			shortEdgeB.isLeftEdge = false;
			longEdge.isLeftEdge = true;
		}

		unsigned int startIDX = (tIndex * 3);
		edges[startIDX + 0] = longEdge;
		edges[startIDX + 1] = shortEdgeA;
		edges[startIDX + 2] = shortEdgeB;

		tIndex++;
	}



	//Add edge start/ends to the relevant datasets.
	for (structs::Edge& e : edges) {
		int yMin = glm::clamp(static_cast<int>(std::round(e.start.y)), 0, display::RENDER_RESOLUTION.y-1);
		int yMax = glm::clamp(static_cast<int>(std::round(e.end.y + 1)), 0, display::RENDER_RESOLUTION.y-1);
		if (yMin == yMax) {
			continue; //Ignore horizontal edges.
		}

		//Find start of edge and add to relevant line of the additions vector.
		if ((yMin >= 0) && (yMin < display::RENDER_RESOLUTION.y)) {
			edgeAdditions->at(yMin).emplace_back(&e);
		}

		//Find end of edge and add to relevant line of the additions vector.
		if ((yMax >= 0) && (yMax < display::RENDER_RESOLUTION.y)) {
			edgeRemovals->at(yMax).emplace_back(&e);
		}
	}
}


std::vector<structs::TriData> triangleStack;
structs::TriData* getActiveTriangle() {
    if (!triangleStack.empty()) {
        return &triangleStack[0]; // pointer to real object in stack
    }
    return nullptr;
}

bool manageStack(structs::Edge* thisEdge, structs::Span* thisSpan, unsigned int yScan) {
	structs::TriData* activeTriangle = getActiveTriangle();
	bool hasActiveTriangle = activeTriangle != nullptr;
	if (thisEdge->isLeftEdge) {
		//Edge starts new triangle.
		structs::TriData newTriangle = structs::TriData(
			thisEdge->currentX, display::RENDER_RESOLUTION.x-1u,
			thisEdge->triIndex, thisEdge->currentZ
		);
		if (!hasActiveTriangle) { //No triangles are currently active.
			triangleStack.push_back(newTriangle);
		} else {
			//Must decide whether to occlude or be occluded by active triangle.
			if (newTriangle.depth < activeTriangle->depth) {
				activeTriangle->endX = thisEdge->currentX;
				*thisSpan = structs::Span(
					*activeTriangle, yScan
				);

				//New triangle occludes old. Create span for old and add new to start of stack.
				triangleStack.insert(triangleStack.begin(), newTriangle);
				return true;

			} else {
				//Add triangle to stack, in order of depth.
				bool didAddTriangle = false;
				for (unsigned int index=0u; index<triangleStack.size(); index++) {
					if (triangleStack[index].depth > newTriangle.depth) {
						triangleStack.insert(std::next(triangleStack.begin(), index), newTriangle);
						didAddTriangle = true;
						break;
					}
				}

				if (!didAddTriangle) {
					triangleStack.push_back(newTriangle);
				}
			}
		}
	} else {
		//Edge ends a triangle.
		bool foundTriangle = false;
		for (unsigned int index=0u; index<triangleStack.size(); index++) {
			structs::TriData thisTri = triangleStack[index];
			if (thisTri.triIndex == thisEdge->triIndex) {
				//The same triangle that this edge closes.
				thisTri.endX = thisEdge->currentX;
				if ((index+1) < triangleStack.size()) {
					triangleStack[index+1].startX = thisEdge->currentX;
				}
				*thisSpan = structs::Span(
					thisTri, yScan
				);
				triangleStack.erase(std::next(triangleStack.begin(), index));
				foundTriangle = true;
				break;
			}
		}
		if (foundTriangle) {
			return true;
		}
	}
	return false;
}



void createSpans(
		std::array<std::vector<structs::Edge*>, display::RENDER_RESOLUTION.y>* edgeAdditions,
		std::array<std::vector<structs::Edge*>, display::RENDER_RESOLUTION.y>* edgeRemovals
	) {
	std::vector<structs::Edge*> activeEdgesList; //Active edges, based on the above 2 vectors.
	for (unsigned int yScan=0u; yScan<display::RENDER_RESOLUTION.y; yScan++) {
		//Add new lines that start on this scanline.
		for (structs::Edge* thisEdge : edgeAdditions->at(yScan)) {
			activeEdgesList.push_back(thisEdge);
		}

		//Remove lines that stop on this scanline
		for (structs::Edge* thisEdge : edgeRemovals->at(yScan)) {
			auto it = std::find(activeEdgesList.begin(), activeEdgesList.end(), thisEdge);
			if (it != activeEdgesList.end()) {
				activeEdgesList.erase(it);
			}
		}

		//Handle the lines?
		size_t numActiveEdges = activeEdgesList.size();
		if (numActiveEdges < 1) {continue; /* No active edges for this scanline. */}

		for (structs::Edge* thisEdge : activeEdgesList) {
			thisEdge->calculateYScanValues(yScan);
			if constexpr (dev::DRAW_EDGES || dev::DRAW_WIREFRAME) {
				frameBuffer.setPX(
					thisEdge->currentX, yScan,
					((thisEdge->isLeftEdge) ? display::EDGE_COLOUR_L : display::EDGE_COLOUR_R)
				);
			}
		}
		if constexpr (dev::DRAW_WIREFRAME) {continue;}
		//Sort by left-to-right onscreen.
		std::sort(activeEdgesList.begin(), activeEdgesList.end(), structs::compareEdges);


		std::vector<structs::Span> spanStack;
		triangleStack.clear();
		for (structs::Edge* thisEdge : activeEdgesList) {
			structs::Span thisSpan;
			bool success = manageStack(thisEdge, &thisSpan, yScan);
			if (!success) {continue; /* Tri was not in stack. */}
			spanStack.push_back(thisSpan);
		}


		for (structs::Span& thisSpan : spanStack) {
			frameBuffer.drawSpan(thisSpan, colourList.at(thisSpan.triIndex));
		}

		if constexpr (dev::DRAW_EDGES || dev::DRAW_WIREFRAME) {
			for (structs::Edge* thisEdge : activeEdgesList) {				
				frameBuffer.setPX(
					thisEdge->currentX, yScan,
					((thisEdge->isLeftEdge) ? glm::uvec3(255u, 255u, 127u) : glm::uvec3(127u, 255u, 255u))
				);
			}
		}
	}
}



void draw() {
	viewMatrix = graphics::getViewMatrix(camera);
	pvmMatrix = projMatrix * viewMatrix * modelMatrix;


	if constexpr (dev::SHOW_CORNERS) {
		//Mark corners for screen positional reference.
		frameBuffer.setPX(0,				 	0,						glm::uvec3(255, 255, 255));
		frameBuffer.setPX(frameBuffer.width-1, 	0,						glm::uvec3(255,   0,   0));
		frameBuffer.setPX(0,				 	frameBuffer.height-1,	glm::uvec3(  0, 255,   0));
		frameBuffer.setPX(frameBuffer.width-1, 	frameBuffer.height-1,	glm::uvec3(255, 255,   0));
	}



	//Quake span-style rendering process
	projectVertices();

	std::array<std::vector<structs::Edge*>, display::RENDER_RESOLUTION.y> edgeAdditions;
	std::array<std::vector<structs::Edge*>, display::RENDER_RESOLUTION.y> edgeRemovals;
	handleEdges(&edgeAdditions, &edgeRemovals);
	createSpans(&edgeAdditions, &edgeRemovals);


	//Display Shader and update screen.
	frameBuffer.updateGLTexture();
	glViewport(0, 0, currentScreenRes.x, currentScreenRes.y);
	glUseProgram(GLIndex::displayShader);

	glBindTextureUnit(0, frameBuffer.GLTextureID);
	glBindVertexArray(GLIndex::screenspaceVAO);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
	glBindVertexArray(0);

	utils::GLErrorcheck("Display Shader", true);
}




void updateCamera() {
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
}



}