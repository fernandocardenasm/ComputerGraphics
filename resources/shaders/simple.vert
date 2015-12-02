#version 150
#extension GL_ARB_explicit_attrib_location : require
// vertex attributes of VAO
layout(location=0) in vec3 in_Position;
layout(location=1) in vec3 in_Normal;
layout(location=2) in vec2 in_Texcoord;

//Matrix Uniforms as specified with glUniformMatrix4fv
uniform mat4 ModelMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ProjectionMatrix;
uniform mat4 NormalMatrix;

out vec4 pass_Normal;
out vec4 pass_Vertex;
out vec4 pass_Light;

out vec2 pass_Texcoord;

vec4 light = vec4(0.0,0.0,0.0,1.0);

void main(void)
{


	gl_Position = (ProjectionMatrix  * ViewMatrix * ModelMatrix) * vec4(in_Position, 1.0f);
	pass_Normal = normalize(NormalMatrix * vec4(in_Normal, 0.0f));
	pass_Vertex =  ViewMatrix * ModelMatrix * vec4(in_Position, 1.0f);
	pass_Light =  ViewMatrix * light ;

	//pass_Texcoord = vec2(in_Texcoord.x, 1.0 - in_Texcoord.y);
	pass_Texcoord = in_Texcoord;

}
