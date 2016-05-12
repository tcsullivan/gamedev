uniform sampler2D sampler;

varying vec2 texCoord;
varying vec4 color;

void main(){
    vec4 pixelColor = texture2D(sampler, vec2(texCoord.x, texCoord.y));
    gl_FragColor = pixelColor * color;
}
