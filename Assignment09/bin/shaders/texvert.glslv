attribute vec3 v_position;
attribute vec2 v_texCoord;
attribute vec3 v_color;
varying vec2 tex_coords;
varying vec3 color;
uniform mat4 mvpMatrix;
uniform bool hasTexture;

void main(void) {
	gl_Position = mvpMatrix * vec4(v_position, 1.0);
	//gl_TexCoord[0].st = v_texCoord;
	tex_coords = v_texCoord;
	
    color = v_color;
}
