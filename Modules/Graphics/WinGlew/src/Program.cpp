#include "Program.h"
#include "Graphics.h"
#include "ConstantBuffer.h"
#include "IndexBuffer.h"
#include "VertexBuffer.h"
#include "base/String.h"
#include "modules/graphics/VertexBufferFactory.h"
#include "modules/graphics/ShaderParameterFactory.h"

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
		}

		auto g = _graphics.get<Graphics>();
		if (g->getDeviceFeatures().supportConstantBuffer) {
			GLint numUniformBlocks;
			glGetProgramiv(_handle, GL_ACTIVE_UNIFORM_BLOCKS, &numUniformBlocks);
			std::vector<GLint> indices;
			std::vector<GLint> offsets;
			std::vector<GLint> types;
			std::vector<GLint> sizes;

			std::vector<std::string_view> varNames;

			_uniformBlockLayouts.resize(numUniformBlocks);
			for (GLint i = 0; i < numUniformBlocks; ++i) {
				auto& layout = _uniformBlockLayouts[i];

				glGetActiveUniformBlockiv(_handle, i, GL_UNIFORM_BLOCK_BINDING, (GLint*)&layout.bindPoint);
				glGetActiveUniformBlockiv(_handle, i, GL_UNIFORM_BLOCK_DATA_SIZE, (GLint*)&layout.size);

				//GLint nameLen;
				//glGetActiveUniformBlockiv(_handle, i, GL_UNIFORM_BLOCK_NAME_LENGTH, &nameLen);

				GLint nameLen;
				glGetActiveUniformBlockName(_handle, i, sizeof(charBuffer), &nameLen, charBuffer);
				layout.name = charBuffer + 5;//type_

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

				for (GLint j = 0; j < numUniforms; ++j) {
					glGetActiveUniformName(_handle, indices[j], sizeof(charBuffer), &nameLen, charBuffer);
					
					auto rootOffset = String::findFirst(charBuffer, nameLen, '.') + 1;
					std::string_view child(charBuffer + rootOffset, nameLen - rootOffset);

					ConstantBufferLayout::Variables* foundVar = nullptr;

					auto parentVars = &layout.variables;
					varNames.clear();
					String::split(child, std::string_view("."), varNames);
					for (auto& sv : varNames) {
						ConstantBufferLayout::Variables* foundMemVar = nullptr;
						for (auto& memVar : *parentVars) {
							if (memVar.name == sv) {
								foundMemVar = &memVar;
								break;
							}
						}

						if (foundMemVar) {
							foundVar = foundMemVar;
							parentVars = &foundMemVar->structMembers;
						} else {
							auto& var = parentVars->emplace_back();
							if (auto svLen = sv.size(); svLen > 3 && sv[svLen - 3] == '[' && sv[svLen - 2] == '0' && sv[svLen - 1] == ']') {
								var.name = sv.substr(0, svLen - 3);
							} else {
								var.name = sv;
							}
							parentVars = &var.structMembers;
							foundVar = &var;
						}
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
			auto g = _graphics.get<Graphics>();

			for (auto& info : _inVertexBufferLayouts) {
				if (auto vb = vertexFactory->get(info.name); vb && _graphics == vb->getGraphics()) ((VertexBuffer*)vb)->use(info.location);
			}

			if (paramFactory) {
				for (auto& layout : _uniformBlockLayouts) {
					ShaderParameterUsageStatistics statistics;
					layout.collectUsingInfo(*paramFactory, statistics, (std::vector<const ShaderParameter*>&)_tempParams, _tempVars);

					ConstantBuffer* cb = nullptr;
					ui32 numVars = _tempVars.size();
					if (statistics.unknownCount < numVars) {
						if (statistics.exclusiveCount > 0 && !statistics.shareCount) {
							cb = (ConstantBuffer*)g->getConstantBufferManager().getExclusiveConstantBuffer(_tempParams, layout);

							if (cb) {
								bool isMaping = false;
								for (ui32 i = 0; i < numVars; ++i) {
									if (auto param = _tempParams[i]; param && param->getUpdateId() != cb->recordUpdateIds[i]) {
										if (!isMaping) {
											if (cb->map(Usage::CPU_WRITE) == Usage::NONE) break;
											isMaping = true;
										}
										cb->recordUpdateIds[i] = param->getUpdateId();
										_updateConstantBuffer(cb, *param, *_tempVars[i]);
									}
								}

								if (isMaping) cb->unmap();
							}
						} else {
							cb = (ConstantBuffer*)_graphics.get<Graphics>()->getConstantBufferManager().popShareConstantBuffer(layout.size);
							if (cb) _constantBufferUpdateAll(cb, layout.variables);
						}
					}

					_tempParams.clear();
					_tempVars.clear();

					glBindBufferBase(GL_UNIFORM_BUFFER, layout.bindPoint, cb->getInternalBuffer());
				}
			}

			glEnable(GL_CULL_FACE);
			glCullFace(GL_BACK);
			glDisable(GL_DEPTH_TEST);

			((IndexBuffer*)indexBuffer)->draw(count, offset);

			g->getConstantBufferManager().resetUsedShareConstantBuffers();
		}
	}

	void Program::_updateConstantBuffer(ConstantBuffer* cb, const ShaderParameter& param, const ConstantBufferLayout::Variables& var) {
		ui32 size = param.getSize();
		if (!size) return;

		ui16 pes = param.getPerElementSize();
		if (pes < size) {
			/*
			auto remainder = pes & 0b1111;
			if (remainder) {
				auto offset = pes + 16 - remainder;
				auto max = std::min<ui32>(size, var.size);
				ui32 cur = 0, fillSize = 0;
				auto data = (const i8*)param.getData();
				do {
					cb->write(var.offset + fillSize, data + cur, pes);
					cur += pes;
					fillSize += offset;
				} while (cur < max && fillSize < var.size);
			} else {
				cb->write(var.offset, param.getData(), std::min<ui32>(size, var.size));
			}
			*/
		} else {
			cb->write(var.offset, param.getData(), std::min<ui32>(size, var.size));
		}
	}

	void Program::_constantBufferUpdateAll(ConstantBuffer* cb, const std::vector<ConstantBufferLayout::Variables>& vars) {
		if (cb->map(Usage::CPU_WRITE) != Usage::NONE) {
			for (ui32 i = 0, n = _tempVars.size(); i < n; ++i) {
				auto param = _tempParams[i];
				if (param) _updateConstantBuffer(cb, *param, *_tempVars[i]);
			}

			cb->unmap();
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
			if (auto translator = g->getProgramSourceTranslator(); translator) {
				return _compileShader(g->getProgramSourceTranslator()->translate(source, ProgramLanguage::GLSL, g->getStringVersion()), type);
			} else {
				return 0;
			}
		}

		println("------ glsl shader code(", (type == GL_VERTEX_SHADER ? "vert" : "frag"), ") ------\n", 
			std::string(source.data.getBytes(), source.data.getLength()), "\n------------------------------------");

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