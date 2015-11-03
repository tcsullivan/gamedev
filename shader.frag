#version 120

uniform vec2 lightLocation;
uniform vec3 lightColor;
uniform float lightStrength;
uniform float screenHeight;

uniform vec2 rayStart;
uniform vec2 rayEnd;

uniform sampler2D tex;
uniform vec2 resolution;

void main(){
	float distance = length(lightLocation - gl_FragCoord.xy);
	float attenuation = lightStrength / distance;
	vec4 color = vec4(0, 0, 0, 0.8f - pow(attenuation, 3)) * vec4(lightColor, 1);
	
	gl_FragColor = color;
}
