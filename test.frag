uniform vec2 lightLocation;
uniform vec3 lightColor;
uniform float screenHeight;

float radius = 4.9;
float minLight = .01;
float a = .01;
float b = 1.0 / (radius*radius * minLight);

void main() {
	float distance = length(lightLocation - gl_FragCoord.xy);
	//float attenuation = 1.0 / (1.0 + a*distance + b*distance*distance);
	float attenuation = clamp(1.0 - distance*distance/(radius*radius), 0.0, 1.0); attenuation *= attenuation;
	vec4 color = vec4(attenuation, attenuation, attenuation, attenuation) * vec4(lightColor, 1);

	gl_FragColor = color;
}