attribute vec3 coord2d;
attribute vec2 tex_coord;

uniform vec4 tex_color;
uniform mat4 ortho;

varying vec2 texCoord;
varying vec4 color;

void main(){
	color = tex_color;
    texCoord = tex_coord;
    gl_Position = ortho * vec4(coord2d.xyz, 1.0);
}
