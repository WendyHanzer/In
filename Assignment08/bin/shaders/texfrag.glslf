uniform sampler2D tex;
varying vec2 tex_coords;

void main(void) {
	vec4 texval = texture2D(tex, tex_coords.xy);

	gl_FragColor = texval;
}
