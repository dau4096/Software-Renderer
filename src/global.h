#pragma once
#include "includes.h"
#include "constants.h"
using namespace std;



namespace structs {

struct Texture {
	glm::vec2 dimentions;
	int channels;
	unsigned char* data;
	int valid;

	Texture() : dimentions(0.0f, 0.0f), channels(0), data(nullptr), valid(0) {}

	Texture(glm::vec2 dimentions, int channels, unsigned char* data)
		: dimentions(dimentions), channels(channels), data(data), valid(1) {}
};


struct TriData {
	int startX, endX;
	unsigned int triIndex;
	float depth;

	TriData() : startX(), endX(), triIndex() {}

	TriData(int sX, int eX, unsigned int tIdx, float d)
		: startX(sX), endX(eX), triIndex(tIdx), depth(d) {}
};

static inline bool compareTriData(structs::TriData* a, structs::TriData* b) {
	return a->depth < b->depth;
}


struct Span {
	glm::ivec2 start;
	size_t length;
	int triIndex;

	inline void _clampSpanValues(int X, int Y, int len) {
		start = glm::ivec2(
			glm::clamp(X, 0, static_cast<int>(display::RENDER_RESOLUTION.x-1)),
			glm::clamp(Y, 0, static_cast<int>(display::RENDER_RESOLUTION.y-1))
		);
		int endX = glm::clamp(X + len, 0, static_cast<int>(display::RENDER_RESOLUTION.x));
		length = static_cast<size_t>(std::max(0, endX - start.x));
	}

	Span() : start(), length(0), triIndex(0) {}

	Span(int X, int Y, int len, int tIdx)
		: triIndex(tIdx) {
			_clampSpanValues(X, Y, len);
		}

	Span(float X, int Y, int len, int tIdx)
		: triIndex(tIdx) {
			_clampSpanValues(int(round(X)), Y, len);
		}

	Span(TriData& thisTri, unsigned int yScan)
		: triIndex(thisTri.triIndex) {
			_clampSpanValues(thisTri.startX, yScan, thisTri.endX - thisTri.startX);
	}
};


struct FrameBuffer {
	std::vector<GLubyte> data;
	size_t width, height, channels;
	GLuint GLTextureID;

	FrameBuffer() : data(), width(0), height(0), channels(0), GLTextureID() {}

	void generateTexture() {
		glGenTextures(1, &GLTextureID);
		glBindTexture(GL_TEXTURE_2D, GLTextureID);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data.data());
		glBindTexture(GL_TEXTURE_2D, 0);	
	}

	FrameBuffer(int width, int height) //Only support RGB. No need for alpha anytime soon.
		: width(width), height(height), channels(3) {
			data = std::vector<GLubyte>(width * height * channels);
			generateTexture();
		}

	FrameBuffer(glm::ivec2 resolution) //Only support RGB. No need for alpha anytime soon.
		: width(static_cast<unsigned int>(resolution.x)), height(static_cast<unsigned int>(resolution.y)), channels(3) {
			data = std::vector<GLubyte>(width * height * channels);
			generateTexture();
		}

	void clear() {
		std::fill(data.begin(), data.end(), 0);
	}

	GLubyte& operator[](size_t index) {
		if ((index < 0) || (index >= width*height)) {
			std::cerr << "Index out of range: " << index << std::endl;
		}
		return data[index];
	}

	GLubyte operator[](size_t index) const {
		if ((index < 0) || (index >= width*height)) {
			std::cerr << "Index out of range: " << index << std::endl;
		}
		return data[index];
	}

	void setPX(size_t X, size_t Y, glm::uvec3 colour) {
		if ((X < 0) || (X >= width) || (Y < 0) || (Y >= height)) {return; /* Outside of valid framebuffer area */}
		size_t startIdx = (X + (Y * width)) * channels;
		data[startIdx + 0] = colour.r;
		data[startIdx + 1] = colour.g;
		data[startIdx + 2] = colour.b;
	}

	glm::uvec3 getPX(size_t X, size_t Y) const {
		if ((X < 0) || (X >= width) || (Y < 0) || (Y >= height)) {return glm::uvec3(0, 0, 0); /* Outside of valid framebuffer area */}
		size_t startIdx = (X + (Y * width)) * channels;
		return glm::uvec3(
			data[startIdx + 0],
			data[startIdx + 1],
			data[startIdx + 2]
		);
	}

	void drawSpan(Span& span, glm::uvec3 colour=glm::uvec3(255, 0, 255)) {
	    if (
	        ((span.start.x + span.length) <= 0) || (span.start.x >= width) ||
	        ((span.start.y < 0) || (span.start.y >= height))
	    ) {
	        return; //Span is not visible onscreen.
	    }

	    int visibleLength = span.length * channels;
	    if (visibleLength <= 0) {return; /* Would attempt to write 0 bytes */}

	    size_t startIdx = (span.start.x + (span.start.y * width)) * channels;

	    //Write the first pixel's RGB
	    data[startIdx + 0] = static_cast<GLubyte>(colour.r);
	    data[startIdx + 1] = static_cast<GLubyte>(colour.g);
	    data[startIdx + 2] = static_cast<GLubyte>(colour.b);

	    size_t size = 3;
	    while (size < static_cast<size_t>(visibleLength)) {
	    	//Exponentially increase how much of the span is filled, O(log(n)) vs O(n) [linear for-loop]
	        size_t copySize = std::min<size_t>(size, visibleLength - size);
	        std::memcpy(data.data() + startIdx + size, data.data() + startIdx, copySize);
	        size += copySize;
	    }
	}

	void updateGLTexture() {
		glBindTexture(GL_TEXTURE_2D, GLTextureID);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data.data());
		glBindTexture(GL_TEXTURE_2D, 0);
	}
};

static inline glm::vec3 reformatVec3(glm::vec3 in) {
	return glm::round(in);
}

static inline glm::vec3 findLowest(glm::vec3 a, glm::vec3 b) {
	if (a.y > b.y) {return reformatVec3(b);}
	return reformatVec3(a);
}
static inline glm::vec3 findHighest(glm::vec3 a, glm::vec3 b) {
	if (a.y < b.y) {return reformatVec3(b);}
	return reformatVec3(a);
}


struct Edge {
	glm::ivec2 start, end;
	float sZ, eZ; //Z Values for ends.
	float dx, currentX, dz, currentZ;
	int triIndex;
	bool isLeftEdge;

	Edge() : start(), end(), dx(), currentX(), triIndex(), isLeftEdge() {}

	Edge(glm::vec3 s, glm::vec3 e, int tIdx, bool isLeft)
		: triIndex(tIdx), sZ(s.z), eZ(e.z),
		  currentZ(sZ), isLeftEdge(isLeft) {
		  	glm::vec3 low = findLowest(s, e);
		  	glm::vec3 high = findHighest(s, e);

		  	start = glm::vec2(low);
		  	end = glm::vec2(high);
		  	currentX = high.x;

			glm::vec3 delta = high - low;
			dx = (abs(delta.y) >= 1) ? (delta.x / delta.y) : 0.0f;
			dz = (abs(delta.y) >= 1) ? (delta.z / delta.y) : 0.0f;
		}

	void calculateYScanValues(unsigned int yScan) {
		float dy = static_cast<float>(yScan - start.y);
		currentX = floor(start.x + (dx * dy) + 0.5f);
		currentZ = sZ + (dz * dy);
	}
};

static inline bool compareEdges(structs::Edge* a, structs::Edge* b) {
	bool bothSameStartX = (a->currentX == b->currentX);
	return (bothSameStartX && (a->isLeftEdge && !b->isLeftEdge)) || (a->currentX < b->currentX);
}


struct Camera {
	glm::vec3 position;
	glm::vec2 angle;
	float nearZ, FOV, farZ;

	Camera()
		: position(display::CAMERA_START_POSITION),
		  angle(display::CAMERA_START_ANGLE),
		  nearZ(display::CAMERA_NEAR_Z),
		  FOV(display::CAMERA_FOV * constants::TO_RAD),
		  farZ(display::CAMERA_FAR_Z) {}
};


struct Model {
	glm::vec3 position, rotation, scale;
	glm::mat4 matrix;
	unsigned int startIndex, endIndex;

	Model() : position(), rotation(), scale(), matrix(), startIndex(), endIndex() {}

	glm::mat4& recalculateMatrix() {
		glm::mat4 translationMat = glm::mat4(
			1.0f, 			0.0f,		 	0.0f,		 	0.0f,
			0.0f, 			1.0f,		 	0.0f,		 	0.0f,
			0.0f, 			0.0f,		 	1.0f,		 	0.0f,
			position.x, 	position.y, 	position.z, 	1.0f
		);

		float sx = sin(rotation.x), cx = cos(rotation.x);
		float sy = sin(rotation.y), cy = cos(rotation.y);
		float sz = sin(rotation.z), cz = cos(rotation.z);
		glm::mat4 rotationMat = glm::mat4(
			cy*cz, 				cy*sz, 				-sy, 	0.0f,
			sx*sy*cz-cx*sz, 	sx*sy*sz+cx*cz, 	sx*cy, 	0.0f,
			cx*sy*cz+sx*sz, 	cx*sy*sz-sx*cz, 	cx*cy,	0.0f,
			0.0f, 				0.0f, 				0.0f, 	1.0f
		);

		glm::mat4 scaleMat = glm::mat4(
			scale.x,	0.0f, 		0.0f,		0.0f, 
			0.0f, 		scale.y,	0.0f, 		0.0f, 
			0.0f, 		0.0f, 		scale.z,	0.0f, 
			0.0f, 		0.0f, 		0.0f, 		1.0f
		);

		matrix = translationMat * rotationMat * scaleMat;
		return matrix;
	}

	Model(
		glm::vec3 pos, glm::vec3 rot, glm::vec3 scl,
		unsigned int sIDX, unsigned int eIDX
	) : position(pos), rotation(rot), scale(scl),
		startIndex(sIDX), endIndex(eIDX) {
			recalculateMatrix();
		}
};


struct ModelMeta { //For use in the model files vector. Only used to load models.
	std::string name;
	glm::vec3 position, rotation, scale;

	ModelMeta() : name(), position(), rotation(), scale() {}

	ModelMeta(std::string fileName, glm::vec3 pos, glm::vec3 rot, glm::vec3 scl)
		: name(fileName), position(pos), rotation(rot), scale(scl) {}
};


}


//Span-rendering specific values
inline structs::FrameBuffer frameBuffer;
inline std::vector<structs::Edge> edges;

inline std::vector<glm::vec3> vertices;
inline std::vector<glm::ivec4> indices;
inline std::vector<glm::vec4> projectedVertices;

//Models
inline std::vector<structs::Model> models;
inline std::vector<structs::ModelMeta> modelFiles;


inline glm::mat4 projMatrix;
inline glm::mat4 viewMatrix, pvMatrix;


inline structs::Camera camera;
inline float verticalFOV;

inline glm::ivec2 currentScreenRes;
inline double cursorXPos, cursorYPos, cursorXPosPrev, cursorYPosPrev;



//Initialize keyMap for input tracking
inline std::vector<int> monitoredKeys = {
	GLFW_KEY_W, GLFW_KEY_S,
	GLFW_KEY_A, GLFW_KEY_D,
	GLFW_KEY_E, GLFW_KEY_Q,
	GLFW_KEY_1, GLFW_KEY_ESCAPE,
	GLFW_KEY_LEFT_SHIFT, GLFW_KEY_LEFT_CONTROL,
	GLFW_KEY_LEFT_ALT
};
inline std::unordered_map<int, bool> keyMap = []() {
	std::unordered_map<int, bool> tmp;
	for (int key : monitoredKeys) {
		tmp[key] = false;
	}
	return tmp;
}();



inline std::vector<glm::uvec3> rngColourList;



namespace GLIndex {

inline GLuint displayShader;
inline GLuint screenspaceVAO;

}