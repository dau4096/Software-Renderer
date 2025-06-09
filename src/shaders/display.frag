/* display.frag */
#version 460 core

in vec2 fragUV;
out vec4 fragColour;

layout(binding = 0) uniform sampler2D renderedFrame;

void main() {
	//vec2 UV = gl_FragCoord.xy / vec2(max(screenResolution, renderResolution));
	//fragColour = vec4(fragUV.rg, 0.0f, 1.0f);
	fragColour = vec4(texture(renderedFrame, fragUV).rgb, 1.0f);
}