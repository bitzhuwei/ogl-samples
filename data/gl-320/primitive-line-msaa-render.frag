#version 150 core

in block
{
	vec2 Texcoord;
} In;

out vec4 Color;

void main()
{
	Color = vec4(1.0, 0.5, 0.0, 1.0);
}
