attribute vec3 v_position;
attribute vec2 v_texCoord;

varying vec2 tex_coords;
uniform mat4 mvpMatrix;

void main(void) {
	gl_Position = mvpMatrix * vec4(v_position, 1.0);
	//gl_TexCoord[0].st = v_texCoord;
	tex_coords = v_texCoord;
}
