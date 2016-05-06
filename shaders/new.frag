uniform sampler2D sampler;

varying vec2 texCoord;
varying float join;

void main(){
    vec4 color = texture2D(sampler, vec2(texCoord.x, texCoord.y));
    gl_FragColor = color;
}
