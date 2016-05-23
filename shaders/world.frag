uniform sampler2D texture;
uniform sampler2D normalTex;

varying vec2 texCoord;
varying vec4 color;
varying vec3 fragCoord;

uniform vec4 ambientLight;
uniform vec4 light[128];
uniform vec4 lightColor[128];
uniform float lightImpact;
uniform int lightSize;

float b = .0005f;
float minLight = .05f;
float radius = sqrt(1.0f / (b * minLight));

void main()
{
	vec4 pixTex = texture2D(texture, vec2(texCoord.x, 1-texCoord.y));
    if (pixTex.a < 0.1f) 
		discard;
 
	vec4 shadeColor = vec4(0.0f, 0.0f, 0.0f, 0.0f);
	for (int i = 0; i < lightSize; i++) {
		vec2 loc = light[i].xy;
		float dist = length(loc - fragCoord.xy);
		float attenuation = clamp(1.0f - dist*dist/(radius*radius), 0.0f, 1.0f);
		attenuation *= attenuation;
		
		shadeColor += vec4(attenuation, attenuation, attenuation, 1.0f) * vec4(lightColor[i]);		
	}
	shadeColor += ambientLight;	

	gl_FragColor = pixTex * color * shadeColor;
}
