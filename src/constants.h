#ifndef CONSTANTS_H
#define CONSTANTS_H

#include "includes.h"
#include <glm/glm.hpp>
namespace constants {
	//Mathematical Constants
	constexpr float PI = 3.141593f;
	constexpr float HALF_PI = PI / 2.0f;
	constexpr float TWO_PI = PI * 2.0f;
	constexpr float EXP = 2.718282f;

	constexpr float TO_RAD = 0.017453f;
	constexpr float TO_DEG = 57.295780f;

	constexpr float EPSILON = 1e-5f;


	//Frequency and time-period.
	constexpr float HZ = 60.0f;
	constexpr float DT = 1.0f / HZ;


	//Invalid returns for vectors and floats.
	constexpr float INVALID = 0xFFFFFF;
	constexpr glm::vec2 INVALIDv2 = glm::vec2(INVALID, INVALID);
	constexpr glm::vec3 INVALIDv3 = glm::vec3(INVALID, INVALID, INVALID);
	constexpr glm::vec4 INVALIDv4 = glm::vec4(INVALID, INVALID, INVALID, INVALID);
}

namespace config {
	constexpr float TURN_SPEED_CURSOR = 0.0025f;
	constexpr float CAMERA_MOVE_SPEED = 0.1f;
	constexpr float CAMERA_MOVE_MULT_SLOW = 0.25f;
	constexpr float CAMERA_MOVE_MULT_FAST = 3.0f;
	constexpr float CAMERA_MOVE_MULT_FASTER = 10.0f;
}

namespace display {
	//Resolutions
	constexpr glm::ivec2 RENDER_RESOLUTION = glm::ivec2(640, 360);
	constexpr glm::ivec2 SCREEN_RESOLUTION = RENDER_RESOLUTION * 2;

	constexpr glm::uvec3 EDGE_COLOUR_L = glm::uvec3(255u, 255u, 127u);
	constexpr glm::uvec3 EDGE_COLOUR_R = glm::uvec3(127u, 255u, 255u);

	//Texture Standardisation
	constexpr glm::ivec2 TEXTURE_RESOLUTION = glm::ivec2(128, 128);

	//Camera Assorted
	constexpr float CAMERA_FOV = 70.0f;
	constexpr float CAMERA_NEAR_Z = 0.01f;
	constexpr float CAMERA_FAR_Z = 100.0f;
	constexpr glm::vec3 CAMERA_START_POSITION = glm::vec3(0.0f,-2.0f, 0.0f);
	constexpr glm::vec2 CAMERA_START_ANGLE = glm::vec2(0.0f, 0.0f);
}


namespace dev {
	//Assorted DEV/DEBUG constants
	constexpr bool SHOW_FREQ = false; //Shows current refresh rate


	//Rendering options.
	constexpr bool DRAW_EDGES = true; //Draws the Edge class instances over the triangles
	constexpr bool DRAW_WIREFRAME = false; //Only draws edges. (Horizontal included)
	constexpr bool SHOW_CORNERS = false; //Draws markers on each corner for visual coordinate reference

	//Very unstable.
	constexpr bool DRAW_BACKFACES = true; //If a triangle has inverted winding order (the back) then it corrects that. Can cause issues as method is not ideal.
}

#endif