/* environment.frag */
#version 460 core

layout(rgba32f, binding=0) uniform image2D renderedFrame;

uniform mat4 pvmMatrix;
uniform float zNear;
uniform float zFar;

struct Triangle {
	vec3 vertexA, vertexB, vertexC;
	vec2 uvA, uvB, uvC;

	int texID;
	int valid;
	vec2 _padding;
};
layout(std140, binding=0) uniform triUBO {
	Triangle triangles[512];
};

struct TriScreen {
	vec4 sVertexA, sVertexB, sVertexC;
	vec2 edgeA, edgeB, edgeC;
	ivec2 minBB, maxBB;
	float areaABC;
	bool valid;
};


vec2 fragPosition;
ivec2 renderResolution;
vec4 fragColour;

const float INF = 0xFFFFFF;
const float EPSILON = 1e-4f;
const vec4 INVALIDv4 = vec4(INF, INF, INF, INF);




vec4 textureMap(sampler2DArray textureArray, vec2 UV, int textureID) {
	vec3 UV3D = vec3(UV.xy, textureID);
	if (textureID >= 0) {
		return texture(textureArray, UV3D.xyz);
	} else {
		return vec4(abs(UV3D.xyz), 1.0f);
	}
}

#include "miniShader.glsl"


float area(vec2 a, vec2 b, vec2 c) {
	return (c.x-a.x)*(b.y-a.y)-(c.y-a.y)*(b.x-a.x);
}



vec4 project(vec3 vertex) {
	vec3 nVertex;
	vertexShader(vertex, nVertex);

	vec4 vertexV4 = vec4(nVertex.xyz, 1.0f);
	vec4 ndc = pvmMatrix * vertexV4;
	ndc.w = max(1e-5f, ndc.w);
	ndc.xyz /= ndc.w;
	if (ndc.z < -1 || ndc.z > 1) {return INVALIDv4;}
	return vec4(
		(ndc.x + 1.0f) / 2.0f * renderResolution.x,
		(1.0f - ndc.y) / 2.0f * renderResolution.y,
		ndc.z * ndc.w, ndc.w
	);
}



TriScreen createTriScreen(Triangle thisTri) {
	TriScreen triS;
	triS.valid = false;
	triS.sVertexA = project(thisTri.vertexA);
	if (triS.sVertexA == INVALIDv4) {return triS;}
	triS.sVertexB = project(thisTri.vertexB);
	if (triS.sVertexB == INVALIDv4) {return triS;}
	triS.sVertexC = project(thisTri.vertexC);
	if (triS.sVertexC == INVALIDv4) {return triS;}

	triS.edgeA = triS.sVertexB.xy - triS.sVertexA.xy;
	triS.edgeB = triS.sVertexC.xy - triS.sVertexB.xy;
	triS.edgeC = triS.sVertexA.xy - triS.sVertexC.xy;

	triS.minBB = ivec2(min(triS.sVertexA.xy, min(triS.sVertexB.xy, triS.sVertexC.xy)));
	triS.minBB = clamp(triS.minBB, ivec2(0, 0), renderResolution - 1);
	triS.maxBB = ivec2(max(triS.sVertexA.xy, max(triS.sVertexB.xy, triS.sVertexC.xy)));
	triS.maxBB = clamp(triS.maxBB, ivec2(0, 0), renderResolution - 1);

	triS.areaABC = area(triS.sVertexA.xy, triS.sVertexB.xy, triS.sVertexC.xy);

	triS.valid = true;

	return triS;
}



bool inTri(TriScreen triS) {
	vec2 p = fragPosition;

	vec2 a = triS.sVertexA.xy;
	vec2 b = triS.sVertexB.xy;
	vec2 c = triS.sVertexC.xy;

	//Compute edge functions
	float w0 = (p.x - b.x) * (a.y - b.y) - (p.y - b.y) * (a.x - b.x);
	float w1 = (p.x - c.x) * (b.y - c.y) - (p.y - c.y) * (b.x - c.x);
	float w2 = (p.x - a.x) * (c.y - a.y) - (p.y - a.y) * (c.x - a.x);

	//Accept either clockwise or counter-clockwise
	return (w0 >= 0.0 && w1 >= 0.0 && w2 >= 0.0) || (w0 <= 0.0 && w1 <= 0.0 && w2 <= 0.0);
}



vec3 baryCentric(TriScreen triS) {
	float baryA = area(fragPosition.xy, triS.sVertexB.xy, triS.sVertexC.xy) / triS.areaABC;
	float baryB = area(fragPosition.xy, triS.sVertexC.xy, triS.sVertexA.xy) / triS.areaABC;
	float baryC = 1.0f - baryA - baryB;

	return vec3(baryA, baryB, baryC);
}



float baryDepth(vec3 bCW, TriScreen triS) {
	float weightSum = triS.sVertexA.w + triS.sVertexB.w + triS.sVertexC.w;
	return (triS.sVertexA.z * bCW.x) + (triS.sVertexB.z * bCW.y) + (triS.sVertexC.z * bCW.z) / weightSum;	
}

vec2 baryUV(vec3 bCW, Triangle tri, TriScreen triS) {
	vec2 uvA = tri.uvA / triS.sVertexA.w;
	vec2 uvB = tri.uvB / triS.sVertexB.w;
	vec2 uvC = tri.uvC / triS.sVertexC.w;

	float wA = 1.0 / triS.sVertexA.w;
	float wB = 1.0 / triS.sVertexB.w;
	float wC = 1.0 / triS.sVertexC.w;

	vec2 uvOverW = bCW.x * uvA + bCW.y * uvB + bCW.z * uvC;
	float invW = bCW.x * wA + bCW.y * wB + bCW.z * wC;

	return uvOverW / invW;
}



void drawTriangles(inout float minDepth) {
	for (int index=0; index<512; index++) {

		Triangle thisTri = triangles[index];
		if (thisTri.valid <= 0) {break; /* End of valid triangles. */}

		TriScreen triS = createTriScreen(thisTri);
		if (!triS.valid) {continue; /* Invalid screen position. */}

		if (fragPosition.x < triS.minBB.x || fragPosition.x > triS.maxBB.x ||
			fragPosition.y < triS.minBB.y || fragPosition.y > triS.maxBB.y) {
			continue;
		}

		if (inTri(triS)) {
			vec3 bCW = baryCentric(triS);
			float interpDepth = baryDepth(bCW, triS);
			if (interpDepth < minDepth) {
				minDepth = interpDepth;
				vec2 UV = baryUV(bCW, thisTri, triS);
				fragmentShader(fragColour.rgb, thisTri, UV, interpDepth);
			}
		}

	}
}



void main() {
	renderResolution = imageSize(renderedFrame);
	fragPosition = vec2(gl_FragCoord.x, renderResolution.y - gl_FragCoord.y);
	ivec2 framePosition = ivec2(gl_FragCoord.xy);
	float minDepth = INF;
	fragColour = vec4(0.0f, 0.0f, 0.0f, minDepth);


	drawTriangles(minDepth);


	vec4 finalFragColour = vec4(fragColour.rgb, minDepth);
	imageStore(renderedFrame, framePosition, finalFragColour);
}