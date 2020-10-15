#include "Program.h"
#include "BaseTexture.h"
#include "BaseTextureView.h"
#include "Graphics.h"
#include "ConstantBuffer.h"
#include "IndexBuffer.h"
#include "Sampler.h"
#include "VertexBuffer.h"
#include "aurora/Debug.h"
#include "aurora/GraphicsBuffer.h"
#include "aurora/ShaderDefine.h"
#include "aurora/ShaderParameter.h"

namespace aurora::modules::graphics::gl {
	Program::Program(Graphics& graphics) : IProgram(graphics),
		_handle(0) {
	}

	Program::~Program() {
		destroy();
	}

	const void* Program::getNative() const {
		return this;
	}

	bool Program::create(const ProgramSource& vert, const ProgramSource& frag, const ShaderDefine* defines, size_t numDefines, const IncludeHandler& handler) {
		destroy();

		auto vertexShader = _compileShader(vert, GL_VERTEX_SHADER, ProgramStage::VS, defines, numDefines, handler);
		if (!vertexShader) {
			destroy();
			return false;
		}

		auto fragmentShader = _compileShader(frag, GL_FRAGMENT_SHADER, ProgramStage::PS, defines, numDefines, handler);
		if (!fragmentShader) {
			glDeleteShader(vertexShader);
			destroy();
			return false;
		}

		if (_handle = glCreateProgram(); !_handle) {
			glDeleteShader(vertexShader);
			glDeleteShader(fragmentShader);
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
			GLint len;
			glGetProgramiv(_handle, GL_INFO_LOG_LENGTH, &len);
			std::string info(len, 0);
			glGetProgramInfoLog(_handle, len, &len, info.data());

			_graphics.get<Graphics>()->error("OpenGL link shader error : " + info);

			destroy();
			return false;
		}

		GLchar charBuffer[256];
		GLsizei len;

		GLint atts;
		glGetProgramiv(_handle, GL_ACTIVE_ATTRIBUTES, &atts);
		_info.vertices.resize(atts);
		_inVertexBufferLocations.resize(atts);
		for (GLint i = 0; i < atts; ++i) {
			auto& info = _info.vertices[i];

			GLenum type;
			GLint size;
			glGetActiveAttrib(_handle, i, sizeof(charBuffer), &len, &size, &type, charBuffer);
			switch (type) {
			case GL_INT:
			{
				info.format.type = VertexType::I32;
				info.format.size = VertexSize::ONE;

				break;
			}
			case GL_INT_VEC2:
			{
				info.format.type = VertexType::I32;
				info.format.size = VertexSize::TWO;

				break;
			}
			case GL_INT_VEC3:
			{
				info.format.type = VertexType::I32;
				info.format.size = VertexSize::THREE;

				break;
			}
			case GL_INT_VEC4:
			{
				info.format.type = VertexType::I32;
				info.format.size = VertexSize::FOUR;

				break;
			}
			case GL_FLOAT:
			{
				info.format.type = VertexType::F32;
				info.format.size = VertexSize::ONE;

				break;
			}
			case GL_FLOAT_VEC2:
			{
				info.format.type = VertexType::F32;
				info.format.size = VertexSize::TWO;

				break;
			}
			case GL_FLOAT_VEC3:
			{
				info.format.type = VertexType::F32;
				info.format.size = VertexSize::THREE;

				break;
			}
			case GL_FLOAT_VEC4:
			{
				info.format.type = VertexType::F32;
				info.format.size = VertexSize::FOUR;

				break;
			}
			default:
				break;
			}
			
			info.name = charBuffer;// +7;//in_var_
			_inVertexBufferLocations[i] = glGetAttribLocation(_handle, charBuffer);
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
			//info.name = charBuffer + 5;//type_ ;20;
			info.location = location;
			info.size = size;
			info.type = type;

			static constexpr char TYPE[] = { "type_" };
			if (len > sizeof(TYPE) - 1) {
				if (std::string_view(charBuffer, sizeof(TYPE) - 1) == TYPE) {
					info.names.emplace_back(charBuffer + sizeof(TYPE) - 1);
				} else if (len > sizeof(IProgramSourceTranslator::COMBINED_TEXTURE_SAMPLER) - 1 && std::string_view(charBuffer, sizeof(IProgramSourceTranslator::COMBINED_TEXTURE_SAMPLER) - 1) == IProgramSourceTranslator::COMBINED_TEXTURE_SAMPLER) {
					auto offset = sizeof(IProgramSourceTranslator::COMBINED_TEXTURE_SAMPLER) - 1;
					if (auto pos = String::find(std::string_view(charBuffer + offset, len - offset), 's'); pos == std::string::npos || !pos) {
						info.names.emplace_back(charBuffer);
					} else {
						uint32_t texNameLen = 0;
						if (auto rst = std::from_chars(charBuffer + offset, charBuffer + offset + pos, texNameLen); rst.ec == std::errc()) {
							offset += pos + 1;
							info.names.emplace_back(charBuffer + offset, texNameLen);
							info.names.emplace_back(charBuffer + offset + texNameLen + 1);
						} else {
							info.names.emplace_back(charBuffer);
						}
					}
				} else {
					info.names.emplace_back(charBuffer);
				}
			} else {
				info.names.emplace_back(charBuffer);
			}
		}

		auto g = _graphics.get<Graphics>();
		if (g->getDeviceFeatures().constantBuffer) {
			GLint numUniformBlocks;
			glGetProgramiv(_handle, GL_ACTIVE_UNIFORM_BLOCKS, &numUniformBlocks);
			std::vector<GLint> indices;
			std::vector<GLint> offsets;
			std::vector<GLint> types;
			std::vector<GLint> sizes;
			std::vector<GLint> strides;

			std::vector<std::string_view> varNames;

			_uniformBlockLayouts.resize(numUniformBlocks);
			for (GLint i = 0; i < numUniformBlocks; ++i) {
				auto& layout = _uniformBlockLayouts[i];
				layout.bindPoint = i;

				glUniformBlockBinding(_handle, i, i);
				//glGetActiveUniformBlockiv(_handle, i, GL_UNIFORM_BLOCK_BINDING, (GLint*)&layout.bindPoint);
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
				strides.resize(numUniforms);

				auto indicesData = indices.data();
				glGetActiveUniformBlockiv(_handle, i, GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES, indicesData);

				glGetActiveUniformsiv(_handle, numUniforms, (GLuint*)indicesData, GL_UNIFORM_OFFSET, offsets.data());
				glGetActiveUniformsiv(_handle, numUniforms, (GLuint*)indicesData, GL_UNIFORM_TYPE, types.data());
				glGetActiveUniformsiv(_handle, numUniforms, (GLuint*)indicesData, GL_UNIFORM_SIZE, sizes.data());
				glGetActiveUniformsiv(_handle, numUniforms, (GLuint*)indicesData, GL_UNIFORM_ARRAY_STRIDE, strides.data());

				for (GLint j = 0; j < numUniforms; ++j) {
					glGetActiveUniformName(_handle, indices[j], sizeof(charBuffer), &nameLen, charBuffer);
					
					auto rootOffset = String::find(std::string_view(charBuffer, nameLen), '.') + 1;
					std::string_view child(charBuffer + rootOffset, nameLen - rootOffset);

					ConstantBufferLayout::Variables* foundVar = nullptr;

					auto parentVars = &layout.variables;
					varNames.clear();
					String::split(child, std::string_view("."), Invoker(&decltype(varNames)::emplace_back<const std::string_view&>, &varNames));
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

					foundVar->stride = strides[j];
					if (Math::isPot(strides[j])) foundVar->stride |= 1ui32 << 31;
					foundVar->size = Graphics::getGLTypeSize(types[j]);
					if (sizes[j] > 1) {
						if (auto remainder = foundVar->size % strides[j]; remainder) {
							foundVar->size += (foundVar->size + strides[j] - remainder) * (sizes[j] - 1);
						} else {
							foundVar->size *= sizes[j];
						}
					}
					foundVar->offset = offsets[j];
				}

				layout.calcFeatureValue();

				g->getConstantBufferManager().registerConstantLayout(layout);
			}
		}

		return true;
	}

	const ProgramInfo& Program::getInfo() const {
		return _info;
	}

	bool Program::use(const IVertexBufferGetter* vertexBufferGetter, const IShaderParameterGetter* shaderParamGetter) {
		if (_handle) {
			glUseProgram(_handle);

			for (size_t i = 0, n = _info.vertices.size(); i < n; ++i) {
				auto& info = _info.vertices[i];
				if (auto vb = vertexBufferGetter->get(info.name); vb && _graphics == vb->getGraphics()) {
					if (auto vb1 = (VertexBuffer*)vb->getNative(); vb1) vb1->use(_inVertexBufferLocations[i]);
				}
			}

			uint8_t texIdx = 0;

			if (shaderParamGetter) {
				auto g = _graphics.get<Graphics>();

				for (auto& layout : _uniformLayouts) {
					switch (layout.type) {
					case GL_SAMPLER_1D:
					case GL_SAMPLER_1D_ARRAY:
					case GL_SAMPLER_2D:
					case GL_SAMPLER_2D_MULTISAMPLE:
					case GL_SAMPLER_2D_ARRAY:
					case GL_SAMPLER_2D_MULTISAMPLE_ARRAY:
					case GL_SAMPLER_3D:
					{
						if (auto p0 = shaderParamGetter->get(layout.names[0], ShaderParameterType::TEXTURE); p0) {
							if (auto data = p0->getData(); data && g == ((ITextureView*)data)->getGraphics()) {
								if (auto native = (BaseTextureView*)((ITextureView*)data)->getNative(); native && native->internalFormat != GL_NONE) {
									auto idx = texIdx++;

									GLuint sampler = 0;
									if (layout.names.size() > 1 && !layout.names[1].empty()) {
										if (auto p1 = shaderParamGetter->get(layout.names[1], ShaderParameterType::SAMPLER); p1) {
											if (auto data = p1->getData(); data && g == ((ISampler*)data)->getGraphics()) {
												if (auto native = (Sampler*)((ISampler*)data)->getNative(); native) {
													native->update();
													sampler = native->getInternalSampler();
												}
											}
										}
									}

									glActiveTexture(GL_TEXTURE0 + idx);
									glBindTexture(native->internalFormat, native->handle);
									if (sampler) glBindSampler(idx, sampler);
									glUniform1i(layout.location, idx);
								}
							}
						}

						break;
					}
					default:
						break;
					}
				}

				for (auto& layout : _uniformBlockLayouts) {
					ShaderParameterUsageStatistics statistics;
					layout.collectUsingInfo(*shaderParamGetter, statistics, (std::vector<const ShaderParameter*>&)_tempParams, _tempVars);

					ConstantBuffer* cb = nullptr;
					uint32_t numVars = _tempVars.size();
					if (statistics.unknownCount < numVars) {
						if (statistics.exclusiveCount > 0 && !statistics.shareCount) {
							if (cb = (ConstantBuffer*)g->getConstantBufferManager().getExclusiveConstantBuffer(_tempParams, layout); cb) {
								bool isMapping = false;
								for (uint32_t i = 0; i < numVars; ++i) {
									if (auto param = _tempParams[i]; param && param->getUpdateId() != cb->recordUpdateIds[i]) {
										if (!isMapping) {
											if (cb->map(Usage::MAP_WRITE) == Usage::NONE) break;
											isMapping = true;
										}
										cb->recordUpdateIds[i] = param->getUpdateId();
										ConstantBufferManager::updateConstantBuffer(cb, *param, *_tempVars[i]);
									}
								}

								if (isMapping) cb->unmap();
							}
						} else {
							if (cb = (ConstantBuffer*)g->getConstantBufferManager().popShareConstantBuffer(layout.size); cb) _constantBufferUpdateAll(cb, layout.variables);
						}
					}

					_tempParams.clear();
					_tempVars.clear();

					if (cb) {
						if (auto native = (ConstantBuffer*)cb->getNative(); native) glBindBufferBase(GL_UNIFORM_BUFFER, layout.bindPoint, native->getInternalBuffer());
					}
				}
			}

			return true;
		}
		return false;
	}

	void Program::_constantBufferUpdateAll(ConstantBuffer* cb, const std::vector<ConstantBufferLayout::Variables>& vars) {
		if (cb->map(Usage::MAP_WRITE) != Usage::NONE) {
			for (uint32_t i = 0, n = _tempVars.size(); i < n; ++i) {
				if (auto param = _tempParams[i]; param) ConstantBufferManager::updateConstantBuffer(cb, *param, *_tempVars[i]);
			}

			cb->unmap();
		}
	}

	void Program::destroy() {
		if (_handle) {
			glDeleteProgram(_handle);
			_handle = 0;
		}
		_inVertexBufferLocations.clear();
		_uniformLayouts.clear();
		_info.clear();

		auto& cbm = _graphics.get<Graphics>()->getConstantBufferManager();
		for (auto& layout : _uniformBlockLayouts) cbm.unregisterConstantLayout(layout);
		_uniformBlockLayouts.clear();
	}

	GLuint Program::_compileShader(const ProgramSource& source, GLenum type, ProgramStage stage, const ShaderDefine* defines, size_t numDefines, const IncludeHandler& handler) {
		if (!source.isValid()) return 0;

		if (source.language != ProgramLanguage::GLSL) {
			auto g = _graphics.get<Graphics>();
			if (auto translator = g->getProgramSourceTranslator(); translator) {
				return _compileShader(g->getProgramSourceTranslator()->translate(source, ProgramLanguage::GLSL, g->getStringVersion(), defines, numDefines, [this, stage, handler](const std::string_view& name) {
					if (handler) return handler(*this, stage, name);
					return ByteArray();
				}), type, stage, defines, numDefines, handler);
			} else {
				return 0;
			}
		}

		printdln("------ glsl shader code(", (type == GL_VERTEX_SHADER ? "vert" : "frag"), ") ------\n", 
			std::string_view((char*)source.data.getSource(), source.data.getLength()), "\n------------------------------------");

		GLuint shader = glCreateShader(type);
		auto s = (const char*)source.data.getSource();
		auto len = (GLint)source.data.getLength();
		glShaderSource(shader, 1, &s, &len);
		glCompileShader(shader);

		auto compileStatus = GL_TRUE;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &compileStatus);

		if (compileStatus == GL_FALSE) {
			GLint len;
			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
			std::string info(len, 0);
			glGetShaderInfoLog(shader, len, &len, info.data());
			
			GLsizei logLen = 0;
			GLchar log[1024];
			glGetShaderInfoLog(shader, sizeof(log), &logLen, log);
			std::string msg = "OpenGL compile shader error(";
			msg += type == GL_VERTEX_SHADER ? "vert" : "frag";
			msg += ") : \n";
			_graphics.get<Graphics>()->error(msg + info);

			glDeleteShader(shader);
			return 0;
		}
		return shader;
	}
}