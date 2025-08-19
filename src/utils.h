#ifndef UTILS_H
#define UTILS_H

#include "includes.h"
#include "constants.h"
#include "global.h"
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
}

#endif