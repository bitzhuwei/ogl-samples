///////////////////////////////////////////////////////////////////////////////////
/// OpenGL Samples Pack (ogl-samples.g-truc.net)
///
/// Copyright (c) 2004 - 2014 G-Truc Creation (www.g-truc.net)
/// Permission is hereby granted, free of charge, to any person obtaining a copy
/// of this software and associated documentation files (the "Software"), to deal
/// in the Software without restriction, including without limitation the rights
/// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
/// copies of the Software, and to permit persons to whom the Software is
/// furnished to do so, subject to the following conditions:
/// 
/// The above copyright notice and this permission notice shall be included in
/// all copies or substantial portions of the Software.
/// 
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
/// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
/// THE SOFTWARE.
///////////////////////////////////////////////////////////////////////////////////

#include "test.hpp"

namespace
{
	char const * VERT_SHADER_SOURCE("gl-500/texture-bindless-nv.vert");
	char const * FRAG_SHADER_SOURCE("gl-500/texture-bindless-nv.frag");
	char const * TEXTURE_DIFFUSE("kueken7_rgba8_srgb.dds");

	GLsizei const VertexCount(4);
	GLsizeiptr const VertexSize = VertexCount * sizeof(glf::vertex_v2fv2f);
	glf::vertex_v2fv2f const VertexData[VertexCount] =
	{
		glf::vertex_v2fv2f(glm::vec2(-1.0f,-1.0f), glm::vec2(0.0f, 1.0f)),
		glf::vertex_v2fv2f(glm::vec2( 1.0f,-1.0f), glm::vec2(1.0f, 1.0f)),
		glf::vertex_v2fv2f(glm::vec2( 1.0f, 1.0f), glm::vec2(1.0f, 0.0f)),
		glf::vertex_v2fv2f(glm::vec2(-1.0f, 1.0f), glm::vec2(0.0f, 0.0f))
	};

	GLsizei const ElementCount(6);
	GLsizeiptr const ElementSize = ElementCount * sizeof(GLushort);
	GLushort const ElementData[ElementCount] =
	{
		0, 1, 2, 
		2, 3, 0
	};

	namespace buffer
	{
		enum type
		{
			VERTEX,
			ELEMENT,
			TRANSFORM,
			MAX
		};
	}//namespace buffer
}//namespace

class instance : public test
{
public:
	instance(int argc, char* argv[]) :
		test(argc, argv, "gl-500-texture-bindless-nv", test::CORE, 4, 5),
		PipelineName(0),
		ProgramName(0),
		VertexArrayName(0),
		TextureName(0),
		TextureHandle(0)
	{}

private:
	std::array<GLuint, buffer::MAX> BufferName;
	GLuint PipelineName;
	GLuint ProgramName;
	GLuint VertexArrayName;
	GLuint TextureName;
	GLuint64 TextureHandle;

	bool initProgram()
	{
		bool Validated(true);
	
		if(Validated)
		{
			compiler Compiler;
			GLuint VertShaderName = Compiler.create(GL_VERTEX_SHADER, getDataDirectory() + VERT_SHADER_SOURCE, "--version 450 --profile core");
			GLuint FragShaderName = Compiler.create(GL_FRAGMENT_SHADER, getDataDirectory() + FRAG_SHADER_SOURCE, "--version 450 --profile core");
			Validated = Validated && Compiler.check();

			ProgramName = glCreateProgram();
			glProgramParameteri(ProgramName, GL_PROGRAM_SEPARABLE, GL_TRUE);
			glAttachShader(ProgramName, VertShaderName);
			glAttachShader(ProgramName, FragShaderName);
			glLinkProgram(ProgramName);
			Validated = Validated && Compiler.checkProgram(ProgramName);
		}

		if(Validated)
		{
			glCreateProgramPipelines(1, &PipelineName);
			glUseProgramStages(PipelineName, GL_VERTEX_SHADER_BIT | GL_FRAGMENT_SHADER_BIT, ProgramName);
		}

		return Validated;
	}

	bool initBuffer()
	{
		GLint UniformBufferOffset(0);
		glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &UniformBufferOffset);
		GLint UniformBlockSize = glm::max(GLint(sizeof(glm::mat4)), UniformBufferOffset);

		glCreateBuffers(buffer::MAX, &BufferName[0]);

		glNamedBufferStorage(BufferName[buffer::ELEMENT], ElementSize, ElementData, 0);
		glNamedBufferStorage(BufferName[buffer::VERTEX], VertexSize, VertexData, 0);
		glNamedBufferStorage(BufferName[buffer::TRANSFORM], UniformBlockSize, nullptr, GL_MAP_WRITE_BIT);

		return true;
	}

	bool initTexture()
	{
		bool Validated(true);

		gli::gl GL;
		gli::texture2D Texture(gli::load_dds((getDataDirectory() + TEXTURE_DIFFUSE).c_str()));
		assert(!Texture.empty());
		gli::gl::format const Format = GL.translate(Texture.format());

		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, &TextureName);
		glTextureParameteri(TextureName, GL_TEXTURE_SWIZZLE_R, GL_RED);
		glTextureParameteri(TextureName, GL_TEXTURE_SWIZZLE_G, GL_GREEN);
		glTextureParameteri(TextureName, GL_TEXTURE_SWIZZLE_B, GL_BLUE);
		glTextureParameteri(TextureName, GL_TEXTURE_SWIZZLE_A, GL_ALPHA);
		glTextureParameteri(TextureName, GL_TEXTURE_BASE_LEVEL, 0);
		glTextureParameteri(TextureName, GL_TEXTURE_MAX_LEVEL, GLint(Texture.levels() - 1));
		glTextureParameteri(TextureName, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTextureParameteri(TextureName, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTextureStorage3D(TextureName, static_cast<GLint>(Texture.levels()), Format.Internal,
			static_cast<GLsizei>(Texture.dimensions().x), static_cast<GLsizei>(Texture.dimensions().y), 1);

		for(gli::texture2D::size_type Level(0); Level < Texture.levels(); ++Level)
		{
			glTextureSubImage3D(TextureName, static_cast<GLint>(Level),
				0, 0, 0,
				static_cast<GLsizei>(Texture[Level].dimensions().x), static_cast<GLsizei>(Texture[Level].dimensions().y), 1,
				Format.External, Format.Type,
				Texture[Level].data());
		}

		glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

		// Query the texture handle and make the texture resident
		TextureHandle = glGetTextureHandleNV(TextureName);
		glMakeTextureHandleResidentNV(TextureHandle);

		return Validated;
	}

	bool initVertexArray()
	{
		glCreateVertexArrays(1, &VertexArrayName);

		glVertexArrayAttribBinding(VertexArrayName, semantic::attr::POSITION, 0);
		glVertexArrayAttribFormat(VertexArrayName, semantic::attr::POSITION, 2, GL_FLOAT, GL_FALSE, 0);
		glEnableVertexArrayAttrib(VertexArrayName, semantic::attr::POSITION);

		glVertexArrayAttribBinding(VertexArrayName, semantic::attr::TEXCOORD, 0);
		glVertexArrayAttribFormat(VertexArrayName, semantic::attr::TEXCOORD, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2));
		glEnableVertexArrayAttrib(VertexArrayName, semantic::attr::TEXCOORD);

		glVertexArrayElementBuffer(VertexArrayName, BufferName[buffer::ELEMENT]);
		glVertexArrayVertexBuffer(VertexArrayName, 0, BufferName[buffer::VERTEX], 0, sizeof(glf::vertex_v2fv2f));

		return true;
	}

	bool begin()
	{
		bool Validated(true);
		Validated = Validated && this->checkExtension("GL_NV_bindless_texture");

		if(Validated)
			Validated = initProgram();
		if(Validated)
			Validated = initBuffer();
		if(Validated)
			Validated = initVertexArray();
		if(Validated)
			Validated = initTexture();

		return Validated;
	}

	bool end()
	{
		glDeleteProgramPipelines(1, &PipelineName);
		glDeleteProgram(ProgramName);
		glDeleteBuffers(buffer::MAX, &BufferName[0]);
		glDeleteTextures(1, &TextureName);
		glDeleteVertexArrays(1, &VertexArrayName);

		return true;
	}

	bool render()
	{
		glm::vec2 WindowSize(this->getWindowSize());

		// Update of the uniform buffer
		{
			glm::mat4* Pointer = static_cast<glm::mat4*>(glMapNamedBufferRange(BufferName[buffer::TRANSFORM],
				0, sizeof(glm::mat4), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT));

			glm::mat4 Projection = glm::perspectiveFov(glm::pi<float>() * 0.25f, WindowSize.x, WindowSize.y, 0.1f, 100.0f);
			glm::mat4 Model = glm::mat4(1.0f);
		
			*Pointer = Projection * this->view() * Model;

			// Make sure the uniform buffer is uploaded
			glUnmapNamedBuffer(BufferName[buffer::TRANSFORM]);
		}

		glViewportIndexedf(0, 0, 0, WindowSize.x, WindowSize.y);
		glClearBufferfv(GL_COLOR, 0, &glm::vec4(1.0f, 0.5f, 0.0f, 1.0f)[0]);

		GLint LocationDiffuse = 0;
		glProgramUniformHandleui64NV(ProgramName, LocationDiffuse, TextureHandle);

		glBindProgramPipeline(PipelineName);
		glBindVertexArray(VertexArrayName);
		glBindBufferBase(GL_UNIFORM_BUFFER, semantic::uniform::TRANSFORM0, BufferName[buffer::TRANSFORM]);

		glDrawElementsInstancedBaseVertexBaseInstance(GL_TRIANGLES, ElementCount, GL_UNSIGNED_SHORT, 0, 1, 0, 0);

		return true;
	}
};

int main(int argc, char* argv[])
{
	int Error(0);

	instance Test(argc, argv);
	Error += Test();

	return Error;
}

