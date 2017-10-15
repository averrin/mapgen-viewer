uniform sampler2D mask;
uniform sampler2D texture;

void main(void){
  vec2 tex_coord = gl_TexCoord[0].xy;
    vec4 mask_color = texture2D(mask, tex_coord);
    vec4 pixel = texture2D(texture, tex_coord);
    if (mask_color[3] == 0.f) {
      if (pixel[3] > 0.f) {
        pixel[0] = 1.f;
        pixel[1] = 0.f;
        pixel[2] = 0.f;
        pixel[3] = 0.f;
        gl_FragColor = pixel;
      }
    }
    gl_FragColor = pixel;
}
