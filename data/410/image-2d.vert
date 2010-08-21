#version 410 core

precision highp int;

// Declare all the semantics
#define ATTR_POSITION	0
#define ATTR_COLOR		3
#define ATTR_TEXCOORD	4
#define VERT_POSITION	0
#define VERT_COLOR		3
#define VERT_TEXCOORD	4
#define VERT_INSTANCE	7
#define GEOM_TEXCOORD	4
#define GEOM_INSTANCE	7
#define FRAG_COLOR		0

uniform mat4 MVP;

layout(location = ATTR_POSITION) in vec2 Position;
layout(location = ATTR_TEXCOORD) in vec2 Texcoord;
layout(location = VERT_TEXCOORD) out vec2 VertTexcoord;

void main()
{	
	VertTexcoord = Texcoord;
	gl_Position = MVP * vec4(Position, 0.0, 1.0);
}