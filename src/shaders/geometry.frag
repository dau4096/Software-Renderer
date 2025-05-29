/* environment.frag */
#version 460 core


layout(rgba32f, binding=0) uniform image2D renderedFrame;
layout(binding=0) uniform sampler2DArray textureArray;

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



//Simple "shader" to be applied to every valid frag.
//Shader-ception!
void fragShader(out vec3 _fragColour, Triangle tri, vec2 UV, float depth) {
	//return vec3(UV.xy, 0.0f);
	_fragColour = texture(textureArray, vec3(UV.xy, tri.texID)).rgb;
}



float area(vec2 a, vec2 b, vec2 c) {
	return (c.x-a.x)*(b.y-a.y)-(c.y-a.y)*(b.x-a.x);
}



vec4 project(vec3 vertex) {
	vec4 vertexV4 = vec4(vertex.xyz, 1.0f);
	vec4 ndc = pvmMatrix * vertexV4;
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
	triS.maxBB = ivec2(max(triS.sVertexA.xy, max(triS.sVertexB.xy, triS.sVertexC.xy)));

	triS.areaABC = area(triS.sVertexA.xy, triS.sVertexB.xy, triS.sVertexC.xy);

	triS.valid = true;

	return triS;
}



vec3 baryCentric(TriScreen triS) {
	float baryA = area(fragPosition.xy, triS.sVertexB.xy, triS.sVertexC.xy) / triS.areaABC;
	float baryB = area(fragPosition.xy, triS.sVertexC.xy, triS.sVertexA.xy) / triS.areaABC;
	float baryC = 1.0f - baryA - baryB;

	return vec3(baryA, baryB, baryC);
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



float baryDepth(vec3 bCW, TriScreen triS) {
	float weightSum = triS.sVertexA.w + triS.sVertexB.w + triS.sVertexC.w;
	return (triS.sVertexA.z * bCW.x) + (triS.sVertexB.z * bCW.y) + (triS.sVertexC.z * bCW.z) / weightSum;	
}

vec2 baryUV(vec3 bCW, Triangle tri) {
    return bCW.x * tri.uvA + bCW.y * tri.uvB + bCW.z * tri.uvC;
}



void drawTriangles(inout float minDepth) {
	for (int index=0; index<512; index++) {

		Triangle thisTri = triangles[index];
		if (thisTri.valid <= 0) {break; /* End of valid triangles. */}

		TriScreen triS = createTriScreen(thisTri);
		if (!triS.valid) {continue; /* Invalid screen position. */}

		if ((triS.maxBB.x < 0.0f) || (triS.maxBB.y < 0.0f) || (triS.minBB.x > renderResolution.x) || (triS.minBB.y > renderResolution.y)) {
			continue;
		}
		if (fragPosition.x < triS.minBB.x || fragPosition.x > triS.maxBB.x ||
			fragPosition.y < triS.minBB.y || fragPosition.y > triS.maxBB.y) {
			continue;
		}

		if (inTri(triS)) {
			vec3 bCW = baryCentric(triS);
			float interpDepth = baryDepth(bCW, triS);
			if (interpDepth < minDepth) {
				minDepth = interpDepth;
				vec2 UV = baryUV(bCW, thisTri);
				fragShader(fragColour.rgb, thisTri, UV, interpDepth);
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