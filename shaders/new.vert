attribute vec3 coord2d;
attribute vec2 tex_coord;

uniform mat4 ortho;
uniform vec4 tex_color;

varying vec2 texCoord;
varying vec4 color;

void main(){
    texCoord = tex_coord;
    color = tex_color;
    gl_Position = ortho * vec4(coord2d.xyz, 1.0);
}
