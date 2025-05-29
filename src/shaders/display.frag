/* display.frag */
#version 460 core

in vec2 fragTexCoord;
out vec4 fragColour;

uniform ivec2 screenResolution;
uniform sampler2D renderedFrame;


void main() {
	vec2 UV = gl_FragCoord.xy / vec2(screenResolution);
	fragColour = vec4(texture(renderedFrame, UV).rgb, 1.0f);
}