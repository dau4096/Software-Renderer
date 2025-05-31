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


	struct Vertex {
		alignas(16) glm::vec3 position;
		alignas(16) glm::vec4 uvCol;


		Vertex() : position(), uvCol() {}

		Vertex(glm::vec3 position, glm::vec2 UV)
			: position(position), //Without vertex col VV
			  uvCol(glm::vec4(UV.x, UV.y, 0.0f, 0.0f)) {}

		Vertex(glm::vec3 position, glm::vec3 vertexColour)
			: position(position), //With vertex colour, value is 1             VV
			  uvCol(glm::vec4(vertexColour.r, vertexColour.g, vertexColour.b, 1.0f)) {}
	};


	struct Model {
		glm::vec3 position, rotation, scale;
		int startIndex, endIndex, textureID;

		Model() : position(), rotation(), scale(), startIndex(), endIndex(), textureID() {}

		Model(
				glm::vec3 position, glm::vec3 rotation, glm::vec3 scale,
				int startIndex, int endIndex, int textureID
			) : position(position), rotation(rotation), scale(scale),
				startIndex(min(startIndex, endIndex)), endIndex(max(startIndex, endIndex)), textureID(textureID) {}
	};

	static glm::mat4 modelMatrix(glm::vec3 pos, glm::vec3 rot, glm::vec3 scale) {
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

	struct ModelGPU {
		alignas(16) glm::mat4 pvmMatrix;
		alignas(8) glm::ivec2 indexLimits;
		alignas(4) int textureID;
		alignas(4) int _padding;

		ModelGPU() : pvmMatrix(), indexLimits(), textureID(), _padding() {}

		ModelGPU(Model* model, glm::mat4& pvMatrix)
			: pvmMatrix(pvMatrix * modelMatrix(model->position, model->rotation, model->scale)),
			  indexLimits(model->startIndex, model->endIndex), textureID(model->textureID) {}
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