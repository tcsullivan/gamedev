#version 120
uniform sampler2D sampler;

uniform int numLight;
uniform vec2 lightLocation[64];
uniform float fireFlicker[64];
uniform vec3 lightColor;
uniform float amb;

float b = .0005f;
float minLight = .05f;
float radius = sqrt(1.0f / (b * minLight));
//float radius = b*minlight;

float rand(vec2 co){
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

void main(){
	vec4 color = vec4(0.0f, 0.0f, 0.0f, 0.0f);
	for(int i = 0; i < numLight; i++){
		vec2 loc = lightLocation[i];
		float dist = length(loc - gl_FragCoord.xy);
		//float attenuation=1.0/(1.0+0.01*dist+0.00000000001*dist*dist);
		float attenuation = clamp(1.0f - dist*dist/(radius*radius), 0.0f, 1.0f); attenuation *= attenuation;

		color += vec4(attenuation, attenuation, attenuation, 1.0f) * vec4(lightColor, 1.0f) * fireFlicker[i];
	}
	vec2 coords = gl_TexCoord[0].st;
	vec4 tex = texture2D(sampler, coords);

	color += vec4(amb,amb,amb,1.0f+amb);
	gl_FragColor = tex * vec4(color)*tex.a;
}

/*	b values
	.002		10
	.008		50
	.0005		200
	.00008		500
	.00002		1000
	.00005		2000
*/
