attribute vec3 v_position;
attribute vec2 v_texCoord;
attribute vec3 v_color;
attribute vec3 v_normal;
varying vec2 tex_coords;
varying vec4 color;
uniform mat4 mvpMatrix;
uniform bool hasTexture;
uniform vec4 ambient, diffuse, specular;
uniform vec4 lightPosition;
uniform float shininess;

void main(void) {
    vec3 pos = (mvpMatrix * vec4(v_position,1.0)).xyz;

    vec3 L = normalize(lightPosition.xyz - pos);
    vec3 E = normalize(-pos);
    vec3 H = normalize(L + E);
    vec3 N = normalize(mvpMatrix * vec4(v_normal, 0.0)).xyz;

    float kd = max(dot(L,N), 0.0);
    vec4 diffuseValue = kd * diffuse;

    float ks = pow(max(dot(N,H), 0.0), shininess);
    vec4 specularValue = ks * specular;

    if(dot(L,N) < 0.0) {
        specularValue = vec4(1.0,1.0,1.0,0.0);
    }

	tex_coords = v_texCoord;

	//vec3 ignore = v_color;

    color = (ambient + diffuseValue + specularValue) + vec4(v_color, 1.0);

    gl_Position = mvpMatrix * vec4(v_position, 1.0);
}
