uniform sampler2D texture;
uniform sampler2D normalTex;

varying vec2 texCoord;
varying vec4 color;

uniform vec4 ambientLight;
uniform vec4 light[128];
uniform vec4 lightColor[128];
uniform float lightImpact;
uniform int lightSize;

void main()
{

	vec4 pixTex = texture2D(texture, vec2(texCoord.x, 1-texCoord.y));
    if (pixTex.a < 0.1) 
		discard;
    
	if (lightSize > 0) {

	}
	
	gl_FragColor = pixTex * color * pixTex.a;
}
