#version 120
uniform sampler2D sampler;

uniform int numLight;
uniform vec2 lightLocation[255];
uniform vec3 lightColor;
uniform float amb;
// uniform float lightStrength;
//uniform float screenHeight;
void main(){
	vec4 color = vec4(0.0f,0.0f,0.0f,0.0f);
	for(int i = 0; i < numLight; i++){
		vec2 loc = lightLocation[i];
		//if(loc.x == 0.0f) continue;
		float dist = length(loc - gl_FragCoord.xy);
		float attenuation=1.0/(1.0+0.01*dist+0.00000000001*dist*dist);

		//vec4 color = vec4(1.0f,1.0f,1.0f,1.0f);
		color += vec4(attenuation, attenuation, attenuation, 1.0f) * vec4(lightColor, 1.0f);
		//color = color + vec4((vec3(lightColor.r + amb, lightColor.g + amb, lightColor.b + amb)*0.25f),1.0f);
	}
	vec2 coords = gl_TexCoord[0].st;
	vec4 tex = texture2D(sampler, coords);

	color += vec4(amb,amb,amb,1.0f+amb);
	gl_FragColor = tex * vec4(color)*tex.a;
}
