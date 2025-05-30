#ifndef CONSTANTS_H
#define CONSTANTS_H

#include "includes.h"
#include <C:/Users/User/Documents/code/.cpp/glm/glm.hpp>
namespace constants {
	//Mathematical Constants
	constexpr float PI = 3.14159265358979f;
	constexpr float HALF_PI = PI / 2.0f;
	constexpr float TWO_PI = PI * 2.0f;
	constexpr float EXP = 2.71828182845905f;

	constexpr float TO_RAD = 0.01745329251994f;
	constexpr float TO_DEG = 57.2957795130824f;


	//Physics/Rendering Frequency/dt
	constexpr int HZ = 45;
	constexpr double DT = 1.0d/HZ;

	constexpr int MAX_TRIANGLES = 512;


	//Invalid returns for vectors and floats.
	constexpr float INVALID = 1e30f;
	constexpr glm::vec2 INVALIDv2 = glm::vec2(INVALID, INVALID);
	constexpr glm::vec3 INVALIDv3 = glm::vec3(INVALID, INVALID, INVALID);
	constexpr glm::vec4 INVALIDv4 = glm::vec4(INVALID, INVALID, INVALID, INVALID);
}

namespace config {
	constexpr float TURN_SPEED_CURSOR = 0.0025f;
	constexpr float CAMERA_MOVE_SPEED = 0.1f;
	constexpr float CAMERA_MOVE_MULT_FAST = 3.0f;
	constexpr float CAMERA_MOVE_MULT_FASTER = 10.0f;
}

namespace display {
	//Resolutions
	constexpr glm::ivec2 SCREEN_RESOLUTION = glm::ivec2(640, 480);
	constexpr glm::ivec2 RENDER_RESOLUTION = glm::ivec2(960, 540);

	//Texture Standardisation
	constexpr glm::ivec2 TEXTURE_RESOLUTION = glm::ivec2(128, 128);
	constexpr int TEXTURE_ARRAY_MAX_LAYERS = 64;
	constexpr const char* FALLBACK_TEXTURE_PATH = "textures/fallback-general.png";


	//Camera Assorted
	constexpr float CAMERA_FOV = 70.0f;
	constexpr float CAMERA_NEAR_Z = 0.01f;
	constexpr float CAMERA_FAR_Z = 100.0f;
	constexpr glm::vec3 CAMERA_START_POSITION = glm::vec3(0.0f,-2.0f, 0.0f);
	constexpr glm::vec2 CAMERA_START_ANGLE = glm::vec2(0.0f, 0.0f);
}


namespace dev {
	//Assorted DEV/DEBUG constants
	constexpr int SHOW_FREQ = 0;
}

#endif