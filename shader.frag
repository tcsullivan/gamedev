#version 120

uniform vec2 lightLocation;
uniform vec3 lightColor;
uniform float lightStrength;
uniform float screenHeight;

void main(){
	float distance = length(lightLocation - gl_FragCoord.xy);
	float attenuation = lightStrength / distance;
	vec4 color = vec4(attenuation, attenuation, attenuation, (pow(attenuation, 3)) * vec4(lightColor, 2))+.5;
	
	gl_FragColor = color;
}
