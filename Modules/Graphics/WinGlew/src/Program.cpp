#include "Program.h"
#include "Graphics.h"
#include "IndexBuffer.h"
#include "VertexBuffer.h"

namespace aurora::modules::graphics::win_glew {
	Program::Program(Graphics& graphics) : IProgram(graphics),
		_handle(0) {
	}

	Program::~Program() {
		_release();
	}

	bool Program::upload(const ProgramSource& vert, const ProgramSource& frag) {
		_release();

		_handle = glCreateProgram();
		if (!_handle) return false;

		auto vertexShader = _compileShader(vert, GL_VERTEX_SHADER);
		if (!vertexShader) return false;

		auto fragmentShader = _compileShader(frag, GL_FRAGMENT_SHADER);
		if (!fragmentShader) {
			glDeleteShader(vertexShader);
			return false;
		}

		glAttachShader(_handle, vertexShader);
		glAttachShader(_handle, fragmentShader);
		glLinkProgram(_handle);
		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);

		GLint status;
		glGetProgramiv(_handle, GL_LINK_STATUS, &status);
		if (status == GL_FALSE) {
			_release();
			return false;
		}

		GLchar charBuffer[512];
		GLsizei len;

		GLint atts;
		glGetProgramiv(_handle, GL_ACTIVE_ATTRIBUTES, &atts);
		_inVertexBufferLayouts.resize(atts);
		for (GLint i = 0; i < atts; ++i) {
			auto& info = _inVertexBufferLayouts[i];
			glGetActiveAttrib(_handle, i, sizeof(charBuffer), &len, &info.size, &info.type, charBuffer);
			
			info.name = charBuffer + 7;
			info.location = glGetAttribLocation(_handle, charBuffer);
		}

		GLint uniforms;
		glGetProgramiv(_handle, GL_ACTIVE_UNIFORMS, &uniforms);
		for (GLint i = 0; i < uniforms; ++i) {
			GLint size;
			GLenum type;
			glGetActiveUniform(_handle, i, sizeof(charBuffer), &len, &size, &type, charBuffer);
			
			auto location = glGetUniformLocation(_handle, charBuffer);
			if (location < 0) continue;

			auto& info = _uniformLayouts.emplace_back();
			info.name = charBuffer + 5;// 20;
			info.location = location;
			info.size = size;
			info.type = type;

			switch (info.type) {
			case GL_SAMPLER_1D:
			case GL_SAMPLER_2D:
			case GL_SAMPLER_3D:
			{

			}
			}
		}

		if (_graphics.get()->getDeviceFeatures().supportConstantBuffer) {
			GLint uniformBlocks;
			glGetProgramiv(_handle, GL_ACTIVE_UNIFORM_BLOCKS, &uniformBlocks);
			std::vector<GLint> indices;
			std::vector<GLint> offsets;
			std::vector<GLint> types;
			std::vector<GLint> sizes;
			for (GLint i = 0; i < uniforms; ++i) {
				GLint location;
				glGetActiveUniformBlockiv(_handle, i, GL_UNIFORM_BLOCK_BINDING, &location);

				GLint dataSize;
				glGetActiveUniformBlockiv(_handle, i, GL_UNIFORM_BLOCK_DATA_SIZE, &dataSize);

				//GLint nameLen;
				//glGetActiveUniformBlockiv(_handle, i, GL_UNIFORM_BLOCK_NAME_LENGTH, &nameLen);

				GLint nameLen;
				glGetActiveUniformBlockName(_handle, i, sizeof(charBuffer), &nameLen, charBuffer);
				std::string n = charBuffer;

				GLint numUniforms;
				glGetActiveUniformBlockiv(_handle, i, GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS, &numUniforms);//Block中有多少个Uniform变量
				indices.resize(numUniforms);
				offsets.resize(numUniforms);
				types.resize(numUniforms);
				sizes.resize(numUniforms);

				glGetActiveUniformBlockiv(_handle, i, GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES, indices.data());//获取Block中uniform的索引值

				glGetActiveUniformsiv(_handle, numUniforms, (GLuint*)indices.data(), GL_UNIFORM_OFFSET, offsets.data());
				glGetActiveUniformsiv(_handle, numUniforms, (GLuint*)indices.data(), GL_UNIFORM_TYPE, types.data());
				glGetActiveUniformsiv(_handle, numUniforms, (GLuint*)indices.data(), GL_UNIFORM_SIZE, sizes.data());

				for (GLint j = 0; j < numUniforms; ++j) {
					glGetActiveUniformName(_handle, indices[j], sizeof(charBuffer), &nameLen, charBuffer);
					int a = 1;
				}

				int a = 1;
			}
			int a = 1;
		}
		

		return true;
	}

	bool Program::use() {
		if (_handle) {
			glUseProgram(_handle);
			return true;
		}
		return false;
	}

	void Program::draw(const VertexBufferFactory* vertexFactory, const ShaderParameterFactory* paramFactory, 
		const IIndexBuffer* indexBuffer, ui32 count, ui32 offset) {
		if (_handle && vertexFactory && indexBuffer && _graphics == indexBuffer->getGraphics() && count > 0) {
			for (auto& info : _inVertexBufferLayouts) {
				auto vb = vertexFactory->get(info.name);
				if (vb && _graphics == vb->getGraphics()) ((VertexBuffer*)vb)->use(info.location);
			}

			glEnable(GL_CULL_FACE);
			glCullFace(GL_BACK);
			glDisable(GL_DEPTH_TEST);

			((IndexBuffer*)indexBuffer)->draw(count, offset);
		}
	}

	void Program::_release() {
		if (_handle) {
			glDeleteProgram(_handle);
			_handle = 0;
		}
		_inVertexBufferLayouts.clear();
		_uniformLayouts.clear();
	}

	GLuint Program::_compileShader(const ProgramSource& source, GLenum type) {
		if (!source.isValid()) return 0;

		if (source.language != ProgramLanguage::GLSL) {
			auto g = _graphics.get<Graphics>();
			auto translator = g->getProgramSourceTranslator();
			if (translator) {
				return _compileShader(g->getProgramSourceTranslator()->translate(source, ProgramLanguage::GLSL, g->getStringVersion()), type);
			} else {
				return 0;
			}
		}

		println("%s", source.data.getBytes());

		GLuint shader = glCreateShader(type);
		auto s = source.data.getBytes();
		auto len = (GLint)source.data.getLength();
		glShaderSource(shader, 1, &s, &len);
		glCompileShader(shader);

		auto compileStatus = GL_TRUE;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &compileStatus);

		if (compileStatus == GL_FALSE) {
			GLsizei logLen = 0;
			GLchar log[1024];
			glGetShaderInfoLog(shader, sizeof(log), &logLen, log);
			println("compile shader error(%s) :\n %s", (type == GL_VERTEX_SHADER ? "vert" : "frag"), log);

			glDeleteShader(shader);
			return 0;
		}
		return shader;
	}
}