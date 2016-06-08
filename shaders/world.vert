attribute vec3 coord2d;
attribute vec2 tex_coord;

uniform vec4 tex_color;
uniform mat4 ortho;
uniform mat4 transform;

varying vec2 texCoord;
varying vec4 color;
varying vec4 fragCoord;

void main(){
	color = tex_color;
    texCoord = tex_coord;
    fragCoord = vec4(coord2d.xyz, 1.0);
	gl_Position = ortho * transform * fragCoord;
}
