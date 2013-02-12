#version 150 core

uniform sampler2D Diffuse;
uniform sampler2DShadow Shadow;

in block
{
	vec2 Texcoord;
	vec4 ShadowCoord;
} In;

out vec4 Color;

void main()
{
	vec4 ShadowCoord = In.ShadowCoord;
	ShadowCoord.z -= 0.005;

	vec4 Diffuse = texture(Diffuse, In.Texcoord);

	float Visibility = mix(0.5, 1.0, texture(Shadow, ShadowCoord.xyz));

	Color = Visibility * Diffuse;
}
