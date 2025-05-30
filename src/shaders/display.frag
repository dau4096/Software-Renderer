/* display.frag */
#version 460 core

in vec2 fragTexCoord;
out vec4 fragColour;

uniform ivec2 screenResolution;
uniform ivec2 renderResolution;
uniform sampler2D renderedFrame;


void main() {
	vec2 UV;
	if (renderResolution.x < screenResolution.x){
		UV.x = gl_FragCoord.x / float(screenResolution.x);
	} else {
		UV.x = gl_FragCoord.x / float(renderResolution.x);
	}
	if (renderResolution.y < screenResolution.y){
		UV.y = gl_FragCoord.y / float(screenResolution.y);
	} else {
		UV.y = gl_FragCoord.y / float(renderResolution.y);
	}
	fragColour = vec4(texture(renderedFrame, UV).rgb, 1.0f);
}