#version 150

in  vec4 pass_Normal;

uniform vec3 color_for_planet;

out vec4 out_Color;

void main(void)
{
    out_Color = vec4(abs(pass_Normal.xyz), 1.0f);

    out_Color = vec4(color_for_planet,1.0);
}
