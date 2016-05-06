uniform sampler2D sampler;

varying vec2 texCoord;

void main(){
    vec4 color = texture2D(sampler, vec2(texCoord.x, 1-texCoord.y));
    if(color.a <= .1)
        discard;
    gl_FragColor = color;
}
