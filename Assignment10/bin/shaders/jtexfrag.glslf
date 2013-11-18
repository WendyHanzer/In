varying vec3 fN;
varying vec3 fL;
varying vec3 fE;
varying vec2 tex_coords;

uniform sampler2D tex;
uniform bool hasTexture;
uniform vec4 ambient, diffuse, specular;
uniform mat4 mvpMatrix;
uniform float shininess;

void main(void) {
	//float intensity = 1.5;
	//shininess = 2.0;
	vec3 N = normalize(fN);
	vec3 E = normalize(fE);
	vec3 L = normalize(fL);
	vec3 H = normalize(L + E);
	vec4 ambientVal = ambient;// * intensity;
	
	float Kd = max(dot(L,N), 0.0);
	vec4 diffuseVal = Kd * diffuse;// * intensity;
	
	float Ks = pow(max(dot(N,H),0.0),shininess);
	vec4 specularVal = Ks * specular;// * intensity;

	
	if(dot(L,N) < 0.0) specularVal = vec4(0.0,0.0,0.0,1.0);
	
	if(hasTexture) {
		gl_FragColor =  (texture2D(tex,tex_coords.xy) + vec4(0.05, 0.05, 0.05, 0)) * (ambient + diffuse + specular);
	}
	else {
		gl_FragColor = ambientVal + diffuseVal + specularVal;
	}
	//if(Kd == 0) gl_FragColor = vec4(1.0,1.0,0.0,1.0);
	gl_FragColor.a = 1.0;
}



