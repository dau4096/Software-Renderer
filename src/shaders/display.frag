/* display.frag */
#version 460 core

in vec2 fragUV;
out vec4 fragColour;

layout(binding = 0) uniform sampler2D renderedFrame;

void main() {
	fragColour = vec4(texture(renderedFrame, fragUV).rgb, 1.0f);
}