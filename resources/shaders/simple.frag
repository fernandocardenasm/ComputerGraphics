#version 150

in  vec4 pass_Normal;
in  vec4 pass_Light;
in  vec4 pass_Vertex;

//Values taken from: http://www.cs.toronto.edu/~jepson/csc2503/tutorials/phong.pdf

float ka = 0.1;
float kd = 0.6;
float ks = 0.7;

uniform vec3 color_for_planet;

out vec4 out_Color;

void main(void)
{
	//Normalizing  the three-component direction vectors
	vec3 N = normalize(pass_Normal.xyz);
    vec3 L = normalize( (pass_Light - pass_Vertex).xyz );
    vec3 V = normalize( (-pass_Vertex).xyz );
    vec3 R = normalize(-reflect(L, N) );
  	
    //Equation taken from: http://web.cse.ohio-state.edu/~whmin/courses/cse5542-2013-spring/11-illumination.pdf

    //Diffuse light: https://www.youtube.com/watch?v=K2lj2Rzs7hQ
    float diffuse = kd * max(dot(N,L), 0.0);
    diffuse = clamp(diffuse, 0.0, 1.0);

    //n = 8 planets + 1 moon.

    out_Color = vec4(ka * color_for_planet + 
                         diffuse * color_for_planet  + 
                         ks * color_for_planet * max(0.0, (pow(dot(R,V), 9.0)) ),
                         1.0);
}
