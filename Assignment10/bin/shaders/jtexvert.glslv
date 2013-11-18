attribute vec3 v_position;
attribute vec2 v_texCoord;
attribute vec3 v_normal;

varying vec2 tex_coords;
varying vec3 fN;
varying vec3 fE;
varying vec3 fL;
varying vec4 lightPosition;
uniform float shininess;

uniform mat4 mvpMatrix;
uniform bool hasTexture;


void main(void) {
	vec4 lightPosition = vec4(5,2,5,0);
	tex_coords = v_texCoord;
	
	fN = v_normal;
	fE = v_position.xyz;
	fL = lightPosition.xyz;
	
	if(lightPosition.w != 0.0) fL = lightPosition.xyz - v_position.xyz;
	
	gl_Position = mvpMatrix * vec4(v_position, 1.0);
}
