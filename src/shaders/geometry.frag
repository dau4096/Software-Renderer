/* environment.frag */
#version 460 core


layout(rgba32f, binding=0) uniform image2D renderedFrame;

uniform mat4 pvmMatrix;
uniform float zNear;
uniform float zFar;

struct Triangle {
	vec3 vertexA, vertexB, vertexC;
	vec3 colour;
	int valid;
};
layout(std140, binding=0) uniform triUBO {
	Triangle triangles[512];
};

struct TriScreen {
	ivec2 minBB, maxBB;
	vec3 sVertexA, sVertexB, sVertexC;
	vec2 edgeA, edgeB, edgeC;
	bool valid;
};


vec2 fragPosition;
ivec2 renderResolution;
vec4 fragColour;

const float INF = 0xFFFFFF;
const float EPSILON = 1e-4f;
const dvec2 INVALIDdv2 = dvec2(INF, INF);
const vec2 INVALIDv2 = vec2(INF, INF);
const vec3 INVALIDv3 = vec3(INF, INF, INF);


vec3 project(vec3 vertex) {
	vec4 vertexV4 = vec4(vertex.xyz, 1.0f);
	vec4 ndc = pvmMatrix * vertexV4;
	ndc.xyz /= ndc.w;
	if (ndc.z < -1 || ndc.z > 1) {return INVALIDv3;}
	return vec3(
		(ndc.x + 1.0f) / 2.0f * renderResolution.x,
		(1.0f - ndc.y) / 2.0f * renderResolution.y,
		ndc.z
	);
}

TriScreen createTriScreen(Triangle thisTri) {
	TriScreen triS;
	triS.valid = false;
	triS.sVertexA = project(thisTri.vertexA);
	if (triS.sVertexA == INVALIDv3) {return triS;}
	triS.sVertexB = project(thisTri.vertexB);
	if (triS.sVertexB == INVALIDv3) {return triS;}
	triS.sVertexC = project(thisTri.vertexC);
	if (triS.sVertexC == INVALIDv3) {return triS;}

	triS.edgeA = triS.sVertexB.xy - triS.sVertexA.xy;
	triS.edgeB = triS.sVertexC.xy - triS.sVertexB.xy;
	triS.edgeC = triS.sVertexA.xy - triS.sVertexC.xy;

	triS.minBB = ivec2(min(triS.sVertexA.xy, min(triS.sVertexB.xy, triS.sVertexC.xy)));
	triS.maxBB = ivec2(max(triS.sVertexA.xy, max(triS.sVertexB.xy, triS.sVertexC.xy)));

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

void drawTriangles() {
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
			fragColour.rgb = vec3(1, 0, 1);
			//minDepth = interpDepth;
		}
		break;
	}
}



void main() {
	fragPosition = gl_FragCoord.xy;
	renderResolution = imageSize(renderedFrame);
	ivec2 framePosition = ivec2(fragPosition);
	float minDepth = INF;
	fragColour = vec4(0.0f, 0.0f, 0.0f, minDepth);


	drawTriangles();


	vec4 finalFragColour = vec4(fragColour.rgb, minDepth);
	imageStore(renderedFrame, framePosition, finalFragColour);
}