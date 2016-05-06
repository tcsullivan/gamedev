attribute vec3 coord2d;
attribute vec2 tex_coord;

uniform mat4 ortho;

varying vec2 texCoord;

void main(){
    texCoord = tex_coord;
    gl_Position = ortho * vec4(coord2d.xyz, 1.0);
}
