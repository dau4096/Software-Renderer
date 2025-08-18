#ifndef UTILS_H
#define UTILS_H

#include "includes.h"
#include "constants.h"
#include <vector>
#include <stdexcept>
#include <C:/Users/User/Documents/code/.cpp/glm/glm.hpp>
#include "C:/Users/User/Documents/code/.cpp/stb_image_write.h"

using namespace std;



//Utility functions
namespace utils {
	static inline void printVec(glm::vec2 vector) {std::cout << "<" << vector.x << ", " << vector.y << ">" << std::endl;};
	static inline void printVec(glm::vec3 vector) {std::cout << "<" << vector.x << ", " << vector.y << ", " << vector.z << ">" << std::endl;};
	static inline void printVec(glm::vec4 vector) {std::cout << "<" << vector.x << ", " << vector.y << ", " << vector.z << ", " << vector.w << ">" << std::endl;};
	static inline void raise(std::string err) {
		std::cerr << err << std::endl;
		std::string end;
		std::cin >> end;
	};
	static inline void pause() {
		string pause;
		std::cin >> pause;
	};
	void GLErrorcheck(std::string location = "", bool shouldPause = false);

    std::string readFile(const std::string& filePath);


	static inline bool isVec2NaN(glm::vec2 v) {return (std::isnan(v.x) || std::isnan(v.y));}
	static inline bool isVec3NaN(glm::vec3 v) {return (std::isnan(v.x) || std::isnan(v.y) || std::isnan(v.z));}



	static inline std::string strToLower(const std::string& input) {
		std::string result = input;
		std::transform(result.begin(), result.end(), result.begin(), [](unsigned char c){return std::tolower(c);});
		return result;
	}

	static inline std::string strToUpper(const std::string& input) {
		std::string result = input;
		std::transform(result.begin(), result.end(), result.begin(), [](unsigned char c){return std::toupper(c);});
		return result;
	}


	float determinant(glm::vec2 vecA, glm::vec2 vecB);
	float angleClamp(float value); //Degrees


	int RNGc(); //Client
	int RNGw(); //World
	void clearRNG(); //Reset both
	

	struct Texture {
		glm::vec2 dimentions;
		int channels;
		unsigned char* data;
		int valid;

		Texture() : dimentions(0.0f, 0.0f), channels(0), data(nullptr), valid(0) {}

		Texture(glm::vec2 dimentions, int channels, unsigned char* data)
			: dimentions(dimentions), channels(channels), data(data), valid(1) {}
	};


	struct Span {
		glm::ivec2 start;
		size_t length;
		size_t triIndex;

		inline void _clampSpanValues(int X, int Y, int len) {
			start = glm::ivec2(X, Y);
			length = size_t(glm::clamp(len, 0, display::RENDER_RESOLUTION.x));
		}

		Span() : start(), length(0), triIndex(0) {}

		Span(int X, int Y, int len, size_t tIdx)
			: triIndex(tIdx) {
				_clampSpanValues(X, Y, len);
			}

		Span(float X, int Y, int len, size_t tIdx)
			: triIndex(tIdx) {
				_clampSpanValues(int(round(X)), Y, len);
			}
	};


	struct FrameBuffer {
		std::vector<GLubyte> data;
		unsigned int width, height, channels;
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
			data = std::vector<GLubyte>(width * height * channels);
		}

		void writeToPNG(std::string name) {
			stbi_write_png(
				name.c_str(),
				width, height,
				channels, data.data(), width*channels
			);
		}

		GLubyte& operator[](size_t index) {
			if ((index < 0) || (index >= width*height)) {
				raise("Index out of range: " + index);
			}
			return data[index];
		}

		GLubyte operator[](size_t index) const {
			if ((index < 0) || (index >= width*height)) {
				raise("Index out of range: " + index);
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

		void drawSpan(Span* span, glm::uvec3 colour=glm::uvec3(255, 0, 255)) {
			for (size_t xVal=0; xVal<span->length; xVal++) {
				int xPos = span->start.x+xVal;
				if (xPos < 0) {continue;}
				if (xPos >= width) {continue;}
				setPX(xPos, span->start.y, colour);
			}
		}

		void drawSpan(Span* span, std::array<glm::uvec3, 7>* colourList) {
			for (size_t xVal=0; xVal<span->length; xVal++) {
				size_t xPos = span->start.x+xVal;
				if (xPos < 0) {continue;}
				if (xPos >= width) {continue;}
				setPX(xPos, span->start.y, colourList->at(span->triIndex));
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
		size_t triIndex;

		Edge() : start(), end(), dx(), currentX(), triIndex() {}

		Edge(glm::vec3 s, glm::vec3 e, size_t tIdx)
			: triIndex(tIdx), sZ(s.z), eZ(e.z),
			  currentZ(sZ) {

			  	glm::vec3 low = findLowest(s, e);
			  	glm::vec3 high = findHighest(s, e);

			  	start = glm::vec2(low);
			  	end = glm::vec2(high);
			  	currentX = high.x;

				glm::vec3 delta = high - low;
				dx = (abs(delta.y) >= 1) ? (delta.x / delta.y) : 0.0f;
				dz = (abs(delta.y) >= 1) ? (delta.z / delta.y) : 0.0f;
			}

		void calculateYScanValues(size_t yScan) {
			float dy = float(yScan - start.y);
			currentX = floor(start.x + (dx * dy) + 0.5f);
			currentZ = sZ + (dz * dy);
		}
	};


	struct Camera {
		glm::vec3 position;
		glm::vec2 angle;
		float nearZ, FOV, farZ;

		Camera()
			: position(display::CAMERA_START_POSITION),
			  angle(display::CAMERA_START_ANGLE),
			  nearZ(display::CAMERA_NEAR_Z),
			  FOV(display::CAMERA_FOV),
			  farZ(display::CAMERA_FAR_Z) {}
	};
}

#endif