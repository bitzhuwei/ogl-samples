#version 150 core

uniform transform
{
	mat4 MVP;
	mat4 DepthMVP;
	mat4 DepthBiasMVP;
} Transform;

in vec3 Position;
in vec2 Texcoord;

out block
{
	vec2 Texcoord;
	vec4 ShadowCoord;
} Out;

void main()
{
	gl_Position = Transform.MVP * vec4(Position, 1.0);
	Out.ShadowCoord = Transform.DepthBiasMVP * vec4(Position, 1.0);
	Out.Texcoord = Texcoord;
}
