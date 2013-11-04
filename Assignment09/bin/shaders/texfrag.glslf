uniform sampler2D tex;
uniform bool hasTexture;
varying vec2 tex_coords;
varying vec3 color;

void main(void) {

    if(hasTexture) {
        gl_FragColor = texture2D(tex,tex_coords.xy);
    }
    
    else {
        gl_FragColor = vec4(color,1.0f);
    }
}
