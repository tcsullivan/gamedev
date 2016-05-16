uniform sampler2D sampler;

varying vec2 texCoord;
varying vec4 color;

void main(){
    vec4 pixTex = texture2D(sampler, vec2(texCoord.x, 1-texCoord.y));
    if(pixTex.a == 0.0)
        discard;
    gl_FragColor = pixTex * color;
}
