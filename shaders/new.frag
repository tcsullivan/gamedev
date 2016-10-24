uniform sampler2D sampler;

varying vec2 texCoord;
varying vec4 color;

void main(){
    vec4 pixelColor = texture2D(sampler, vec2(texCoord.x, texCoord.y));
    //TODO
	if (pixelColor.w != 1.0f)
		discard;
	gl_FragColor = pixelColor * color;
}
