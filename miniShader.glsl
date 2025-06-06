/* miniShader.glsl */

layout(binding=0) uniform sampler2DArray textureArray;


void vertexShader(in vec3 vertexIn, out vec3 vertexOut) {
	vertexOut = vertexIn;
}


//Simple "shader" to be applied to every valid frag.
//Shader-ception!
void fragmentShader(out vec3 outFragColour, vec2 UV, int textureID, float depth) {
	//outFragColour = vec3(abs(UV.xy), 0.0f);
	outFragColour = textureMap(textureArray, UV.xy, textureID).rgb;
}