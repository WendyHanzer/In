uniform sampler2D tex;
uniform bool hasTexture;
varying vec2 tex_coords;
varying vec4 color;

void main(void) {

    if(hasTexture) {
        gl_FragColor = vec4(color.rgb * vec3(texture2D(tex,tex_coords.xy)), 1.0);
    }

    else {
        gl_FragColor = color;
    }
}
