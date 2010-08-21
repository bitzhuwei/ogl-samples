//**********************************
// OpenGL Layered rendering
// 19/08/2010 - 20/08/2010
//**********************************
// Christophe Riccio
// g.truc.creation@gmail.com
//**********************************
// G-Truc Creation
// www.g-truc.net
//**********************************

#include <glf/glf.hpp>

namespace
{
	std::string const SAMPLE_NAME = "OpenGL Layered rendering";
	std::string const VERT_SHADER_SOURCE1(glf::DATA_DIRECTORY + "400/layer.vert");
	std::string const GEOM_SHADER_SOURCE1(glf::DATA_DIRECTORY + "400/layer.geom");
	std::string const FRAG_SHADER_SOURCE1(glf::DATA_DIRECTORY + "400/layer.frag");
	std::string const VERT_SHADER_SOURCE2(glf::DATA_DIRECTORY + "400/rtt-array.vert");
	std::string const FRAG_SHADER_SOURCE2(glf::DATA_DIRECTORY + "400/rtt-array.frag");
	glm::ivec2 const FRAMEBUFFER_SIZE(320, 240);
	int const SAMPLE_SIZE_WIDTH = 640;
	int const SAMPLE_SIZE_HEIGHT = 480;
	int const SAMPLE_MAJOR_VERSION = 4;
	int const SAMPLE_MINOR_VERSION = 0;

	glf::window Window(glm::ivec2(SAMPLE_SIZE_WIDTH, SAMPLE_SIZE_HEIGHT));

	GLsizei const VertexCount = 4;
	GLsizeiptr const VertexSize = VertexCount * sizeof(glf::vertex_v2fv2f);
	glf::vertex_v2fv2f const VertexData[VertexCount] =
	{
		glf::vertex_v2fv2f(glm::vec2(-1.0f,-1.0f), glm::vec2(0.0f, 0.0f)),
		glf::vertex_v2fv2f(glm::vec2( 1.0f,-1.0f), glm::vec2(1.0f, 0.0f)),
		glf::vertex_v2fv2f(glm::vec2( 1.0f, 1.0f), glm::vec2(1.0f, 1.0f)),
		glf::vertex_v2fv2f(glm::vec2(-1.0f, 1.0f), glm::vec2(0.0f, 1.0f))
	};

	GLsizei const ElementCount = 6;
	GLsizeiptr const ElementSize = ElementCount * sizeof(GLushort);
	GLushort const ElementData[ElementCount] =
	{
		0, 1, 2, 
		2, 3, 0
	};

	enum buffer_type
	{
		BUFFER_VERTEX,
		BUFFER_ELEMENT,
		BUFFER_MAX
	};

	enum program 
	{
		LAYERING,
		IMAGE_2D,
		PROGRAM_MAX
	};

	GLuint FramebufferName = 0;
	GLuint VertexArrayName[PROGRAM_MAX];

	GLuint ProgramName[PROGRAM_MAX];
	GLuint UniformMVP[PROGRAM_MAX];
	GLuint UniformDiffuse = 0;
	GLuint UniformLayer = 0;

	GLuint BufferName[BUFFER_MAX];
	GLuint TextureColorbufferName = 0;

	glm::ivec4 Viewport[4];

}//namespace

bool initProgram()
{
	bool Validated = true;

	if(Validated)
	{
		GLuint VertShaderName = glf::createShader(GL_VERTEX_SHADER, VERT_SHADER_SOURCE1);
		GLuint GeomShaderName = glf::createShader(GL_GEOMETRY_SHADER, GEOM_SHADER_SOURCE1);
		GLuint FragShaderName = glf::createShader(GL_FRAGMENT_SHADER, FRAG_SHADER_SOURCE1);

		ProgramName[LAYERING] = glCreateProgram();
		glAttachShader(ProgramName[LAYERING], VertShaderName);
		glAttachShader(ProgramName[LAYERING], GeomShaderName);
		glAttachShader(ProgramName[LAYERING], FragShaderName);
		glDeleteShader(VertShaderName);
		glDeleteShader(GeomShaderName);
		glDeleteShader(FragShaderName);
		glLinkProgram(ProgramName[LAYERING]);
		Validated = glf::checkProgram(ProgramName[LAYERING]);
	}

	if(Validated)
	{
		GLuint VertShaderName = glf::createShader(GL_VERTEX_SHADER, VERT_SHADER_SOURCE2);
		GLuint FragShaderName = glf::createShader(GL_FRAGMENT_SHADER, FRAG_SHADER_SOURCE2);

		ProgramName[IMAGE_2D] = glCreateProgram();
		glAttachShader(ProgramName[IMAGE_2D], VertShaderName);
		glAttachShader(ProgramName[IMAGE_2D], FragShaderName);
		glDeleteShader(VertShaderName);
		glDeleteShader(FragShaderName);
		glLinkProgram(ProgramName[IMAGE_2D]);
		Validated = glf::checkProgram(ProgramName[IMAGE_2D]);
	}

	if(Validated)
	{
		for(std::size_t i = 0; i < PROGRAM_MAX; ++i)
			UniformMVP[i] = glGetUniformLocation(ProgramName[i], "MVP");

		UniformDiffuse = glGetUniformLocation(ProgramName[IMAGE_2D], "Diffuse");
		UniformLayer = glGetUniformLocation(ProgramName[IMAGE_2D], "Layer");
	}

	return glf::checkError("initProgram");
}

bool initVertexBuffer()
{
	glGenBuffers(BUFFER_MAX, BufferName);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, BufferName[BUFFER_ELEMENT]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, ElementSize, ElementData, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    glBindBuffer(GL_ARRAY_BUFFER, BufferName[BUFFER_VERTEX]);
    glBufferData(GL_ARRAY_BUFFER, VertexSize, VertexData, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	return glf::checkError("initArrayBuffer");
}

bool initTexture()
{
	glGenTextures(1, &TextureColorbufferName);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D_ARRAY, TextureColorbufferName);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_LEVEL, 1000);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_SWIZZLE_R, GL_RED);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_SWIZZLE_G, GL_GREEN);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_SWIZZLE_B, GL_BLUE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_SWIZZLE_A, GL_ALPHA);

	glTexImage3D(
		GL_TEXTURE_2D_ARRAY, 
		0, 
		GL_RGB, 
		GLsizei(FRAMEBUFFER_SIZE.x), 
		GLsizei(FRAMEBUFFER_SIZE.y), 
		GLsizei(4), //depth
		0,  
		GL_RGB, 
		GL_UNSIGNED_BYTE, 
		NULL);

	return glf::checkError("initTexture");
}

bool initFramebuffer()
{
	glGenFramebuffers(1, &FramebufferName);
	glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);
	glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, TextureColorbufferName, 0, 0);
	glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, TextureColorbufferName, 0, 1);
	glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, TextureColorbufferName, 0, 2);
	glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, TextureColorbufferName, 0, 3);

	GLenum DrawBuffers[4]= {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3};
	glDrawBuffers(4, DrawBuffers);

	if(glf::checkFramebuffer(FramebufferName))
		return false;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	return true;
}

bool initVertexArray()
{
	glGenVertexArrays(PROGRAM_MAX, VertexArrayName);

    glBindVertexArray(VertexArrayName[IMAGE_2D]);
		glBindBuffer(GL_ARRAY_BUFFER, BufferName[BUFFER_VERTEX]);
		glVertexAttribPointer(glf::semantic::attr::POSITION, 2, GL_FLOAT, GL_FALSE, sizeof(glf::vertex_v2fv2f), GLF_BUFFER_OFFSET(0));
		glVertexAttribPointer(glf::semantic::attr::TEXCOORD, 2, GL_FLOAT, GL_FALSE, sizeof(glf::vertex_v2fv2f), GLF_BUFFER_OFFSET(sizeof(glm::vec2)));
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glEnableVertexAttribArray(glf::semantic::attr::POSITION);
		glEnableVertexAttribArray(glf::semantic::attr::TEXCOORD);
	glBindVertexArray(0);

    glBindVertexArray(VertexArrayName[LAYERING]);
		glBindBuffer(GL_ARRAY_BUFFER, BufferName[BUFFER_VERTEX]);
		glVertexAttribPointer(glf::semantic::attr::POSITION, 2, GL_FLOAT, GL_FALSE, sizeof(glf::vertex_v2fv2f), GLF_BUFFER_OFFSET(0));
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glEnableVertexAttribArray(glf::semantic::attr::POSITION);
	glBindVertexArray(0);

	return glf::checkError("initVertexArray");
}

bool begin()
{
	int Border = 2;
	Viewport[0] = glm::ivec4(Border, Border, FRAMEBUFFER_SIZE - 2 * Border);
	Viewport[1] = glm::ivec4((Window.Size.x >> 1) + Border, 1, FRAMEBUFFER_SIZE - 2 * Border);
	Viewport[2] = glm::ivec4((Window.Size.x >> 1) + Border, (Window.Size.y >> 1) + Border, FRAMEBUFFER_SIZE - 2 * Border);
	Viewport[3] = glm::ivec4(Border, (Window.Size.y >> 1) + Border, FRAMEBUFFER_SIZE - 2 * Border);

	GLint MajorVersion = 0;
	GLint MinorVersion = 0;
	glGetIntegerv(GL_MAJOR_VERSION, &MajorVersion);
	glGetIntegerv(GL_MINOR_VERSION, &MinorVersion);
	bool Validated = glf::version(MajorVersion, MinorVersion) >= glf::version(SAMPLE_MAJOR_VERSION, SAMPLE_MINOR_VERSION);

	if(Validated)
		Validated = initProgram();
	if(Validated)
		Validated = initVertexBuffer();
	if(Validated)
		Validated = initVertexArray();
	if(Validated)
		Validated = initTexture();
	if(Validated)
		Validated = initFramebuffer();

	return Validated && glf::checkError("begin");
}

bool end()
{
	glDeleteVertexArrays(PROGRAM_MAX, VertexArrayName);
	glDeleteBuffers(BUFFER_MAX, BufferName);
	glDeleteTextures(1, &TextureColorbufferName);
	glDeleteFramebuffers(1, &FramebufferName);
	glDeleteProgram(ProgramName[IMAGE_2D]);
	glDeleteProgram(ProgramName[LAYERING]);

	return glf::checkError("end");
}

void display()
{
	glm::mat4 Projection = glm::ortho(-1.0f, 1.0f, 1.0f,-1.0f, 1.0f, -1.0f);
	glm::mat4 View = glm::mat4(1.0f);
	glm::mat4 Model = glm::mat4(1.0f);
	glm::mat4 MVP = Projection * View * Model;

	// Pass 1
	{
		glf::checkError("display A");

		glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);
		glViewport(0, 0, FRAMEBUFFER_SIZE.x, FRAMEBUFFER_SIZE.y);
		glf::checkError("display 0");

		glUseProgram(ProgramName[LAYERING]);
		glUniformMatrix4fv(UniformMVP[LAYERING], 1, GL_FALSE, &MVP[0][0]);
		glf::checkError("display 1");

		glBindVertexArray(VertexArrayName[LAYERING]);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, BufferName[BUFFER_ELEMENT]);
		glf::checkError("display 2");

		glDrawElementsInstancedBaseVertex(GL_TRIANGLES, ElementCount, GL_UNSIGNED_SHORT, NULL, 1, 0);
		glf::checkError("display 3");
	}

	// Pass 2
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glf::checkError("display 4");

		glUseProgram(ProgramName[IMAGE_2D]);
		glUniformMatrix4fv(UniformMVP[IMAGE_2D], 1, GL_FALSE, &MVP[0][0]);
		glUniform1i(UniformDiffuse, 0);
		glf::checkError("display 5");

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D_ARRAY, TextureColorbufferName);
		glf::checkError("display 6");

		glBindVertexArray(VertexArrayName[IMAGE_2D]);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, BufferName[BUFFER_ELEMENT]);
		glf::checkError("display 7");

		for(std::size_t i = 0; i < 4; ++i)
		{
			glUniform1i(UniformLayer, i);
			glViewport(Viewport[i].x, Viewport[i].y, Viewport[i].z, Viewport[i].w);
			glf::checkError("display 8");

			glDrawElementsInstancedBaseVertex(GL_TRIANGLES, ElementCount, GL_UNSIGNED_SHORT, NULL, 1, 0);
			glf::checkError("display 9");
		}
	}

	glf::checkError("display");
	glf::swapBuffers();
}

int main(int argc, char* argv[])
{
	if(glf::run(
		argc, argv,
		glm::ivec2(SAMPLE_SIZE_WIDTH, SAMPLE_SIZE_HEIGHT), 
		SAMPLE_MAJOR_VERSION, 
		SAMPLE_MINOR_VERSION))
		return 0;
	return 1;
}
