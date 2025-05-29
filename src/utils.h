#ifndef UTILS_H
#define UTILS_H

#include "includes.h"
#include "constants.h"
#include <vector>
#include <stdexcept>
#include <C:/Users/User/Documents/code/.cpp/glm/glm.hpp>

using namespace std;



//Utility functions
namespace utils {
	static inline void printVec2(glm::vec2 vector) {std::cout << "<" << vector.x << ", " << vector.y << ">" << std::endl;};
	static inline void printVec3(glm::vec3 vector) {std::cout << "<" << vector.x << ", " << vector.y << ", " << vector.z << ">" << std::endl;};
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


	struct Triangle {
		alignas(16) glm::vec3 vertexA, vertexB, vertexC;
		alignas(16) glm::vec3 colour;
		alignas(4) int valid;
		alignas(4) int _padding[3];

		Triangle() : vertexA(), vertexB(), vertexC(), colour(), valid(0), _padding() {}

		Triangle(glm::vec3 vA, glm::vec3 vB, glm::vec3 vC, glm::vec3 colour)
			: vertexA(vA), vertexB(vB), vertexC(vC), colour(colour), valid(1), _padding() {}
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