#include "Program.h"
#include "Graphics.h"
#include "IndexBuffer.h"
#include "VertexBuffer.h"
#include "base/String.h"
#include "modules/graphics/VertexBufferFactory.h"

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
			
			info.name = charBuffer + 7;//in_var_
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
			info.name = charBuffer + 5;//type_ ;20;
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

		auto g = _graphics.get<Graphics>();
		if (g->getDeviceFeatures().supportConstantBuffer) {
			GLint uniformBlocks;
			glGetProgramiv(_handle, GL_ACTIVE_UNIFORM_BLOCKS, &uniformBlocks);
			std::vector<GLint> indices;
			std::vector<GLint> offsets;
			std::vector<GLint> types;
			std::vector<GLint> sizes;

			std::vector<std::string_view> varNames;
			std::unordered_map<std::string_view, ConstantBufferLayout::Variables*> vars;
			for (GLint i = 0; i < uniformBlocks; ++i) {
				auto& layout = _uniformBlockLayouts.emplace_back();

				glGetActiveUniformBlockiv(_handle, i, GL_UNIFORM_BLOCK_BINDING, (GLint*)&layout.bindPoint);
				glGetActiveUniformBlockiv(_handle, i, GL_UNIFORM_BLOCK_DATA_SIZE, (GLint*)&layout.size);

				//GLint nameLen;
				//glGetActiveUniformBlockiv(_handle, i, GL_UNIFORM_BLOCK_NAME_LENGTH, &nameLen);

				GLint nameLen;
				glGetActiveUniformBlockName(_handle, i, sizeof(charBuffer), &nameLen, charBuffer);
				layout.name = charBuffer;

				GLint numUniforms;
				glGetActiveUniformBlockiv(_handle, i, GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS, &numUniforms);
				indices.resize(numUniforms);
				offsets.resize(numUniforms);
				types.resize(numUniforms);
				sizes.resize(numUniforms);

				auto indicesData = indices.data();
				glGetActiveUniformBlockiv(_handle, i, GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES, indicesData);

				glGetActiveUniformsiv(_handle, numUniforms, (GLuint*)indicesData, GL_UNIFORM_OFFSET, offsets.data());
				glGetActiveUniformsiv(_handle, numUniforms, (GLuint*)indicesData, GL_UNIFORM_TYPE, types.data());
				glGetActiveUniformsiv(_handle, numUniforms, (GLuint*)indicesData, GL_UNIFORM_SIZE, sizes.data());

				vars.clear();
				for (GLint j = 0; j < numUniforms; ++j) {
					glGetActiveUniformName(_handle, indices[j], sizeof(charBuffer), &nameLen, charBuffer);
					
					auto rootOffset = String::findFirst(charBuffer, nameLen, '.') + 1;
					std::string_view child(charBuffer + rootOffset, nameLen - rootOffset);

					ConstantBufferLayout::Variables* foundVar = nullptr;
					auto itr = vars.find(child);
					if (itr == vars.end()) {
						auto parentVars = &layout.variables;
						ui32 fullNameLen = 0;
						varNames.clear();
						String::split(child, std::string_view("."), varNames);
						for (auto& sv : varNames) {
							fullNameLen += fullNameLen ? sv.size() + 1 : sv.size();
							std::string_view fullName(child.data(), fullNameLen);

							auto itr2 = vars.find(fullName);
							if (itr2 == vars.end()) {
								auto& var = parentVars->emplace_back();
								var.name = sv;
								parentVars = &var.structMembers;
								foundVar = &var;
								vars.emplace(fullName, foundVar);
							} else {
								foundVar = itr2->second;
								parentVars = &foundVar->structMembers;
							}
						}
					} else {
						foundVar = itr->second;
					}

					foundVar->size = Graphics::getGLTypeSize(types[j]) * sizes[j];
					foundVar->offset = offsets[j];
				}

				layout.calcFeatureCode();

				g->getConstantBufferManager().registerConstantLayout(layout);
			}
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
		_uniformBlockLayouts.clear();
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

		println(source.data.getBytes());

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
			println("compile shader error(", (type == GL_VERTEX_SHADER ? "vert" : "frag"), ") : \n", log);

			glDeleteShader(shader);
			return 0;
		}
		return shader;
	}
}