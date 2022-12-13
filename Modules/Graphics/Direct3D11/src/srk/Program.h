#pragma once

#include "ConstantBuffer.h"
#include "Sampler.h"
#include "TextureView.h"
#include "srk/ShaderParameter.h"
#include "srk/modules/graphics/ConstantBufferManager.h"

namespace srk::modules::graphics::d3d11 {
	class SRK_MODULE_DLL Program : public IProgram {
	public:
		Program(Graphics& graphics);
		virtual ~Program();

		virtual const void* SRK_CALL getNative() const override;
		virtual bool SRK_CALL create(const ProgramSource& vert, const ProgramSource& frag, const ShaderDefine* defines, size_t numDefines, const IncludeHandler& includeHandler, const InputHandler& inputHandler) override;
		virtual const ProgramInfo& getInfo() const override;
		virtual void SRK_CALL destroy() override;

		bool SRK_CALL use(const IVertexAttributeGetter* vertexAttributeGetter, const IShaderParameterGetter* shaderParamGetter);
		void SRK_CALL useEnd();

	protected:
		class MyIncludeHandler : public ID3DInclude {
		public:
			MyIncludeHandler(const IProgram& program, ProgramStage stage, const IncludeHandler& handler);

			HRESULT Open(D3D_INCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID* ppData, UINT* pBytes);
			HRESULT Close(LPCVOID pData);

		private:
			ProgramStage _stage;
			ByteArray _data;
			const IProgram& _program;
			const IncludeHandler& _handler;
		};


		class MyConstantBufferLayout : public ConstantBufferLayout {
		public:
			MyConstantBufferLayout() : sameId(0) {}

			uint32_t sameId;
		};


		struct InputLayout {
			InputLayout (uint32_t numInElements);
			~InputLayout();

			std::vector<uint32_t> formats;
			ID3D11InputLayout* layout;

			bool equal(const D3D11_INPUT_ELEMENT_DESC* inputElements, uint32_t num) const;
		};


		ID3DBlob* _vertBlob;
		ID3D11VertexShader* _vs;
		ID3D11PixelShader* _ps;

		ID3D11InputLayout* _curInLayout;
		D3D11_INPUT_ELEMENT_DESC* _inputElements;
		uint32_t _numInElements;

		std::vector<uint32_t> _inVerBufSlots;
		std::vector<InputLayout> _inLayouts;


		struct TextureLayout {
			std::string name;
			uint32_t bindPoint = 0;
		};


		struct SamplerLayout {
			std::string name;
			uint32_t bindPoint = 0;
		};


		struct ParameterLayout {
			std::vector<MyConstantBufferLayout> constantBuffers;
			std::vector<TextureLayout> textures;
			std::vector<SamplerLayout> samplers;

			void clear(Graphics& g);
		};


		ProgramInfo _info;

		ParameterLayout _vsParamLayout;
		ParameterLayout _psParamLayout;

		std::vector<ConstantBuffer*> _usingSameConstBuffers;
		std::vector<ShaderParameter*> _tempParams;
		std::vector<const ConstantBufferLayout::Variables*> _tempVars;

		ID3DBlob* SRK_CALL _compileShader(const ProgramSource& source, ProgramStage stage, const std::string_view& target, const D3D_SHADER_MACRO* defines, const IncludeHandler& handler);
		ID3D11InputLayout* _getOrCreateInputLayout();
		void SRK_CALL _parseInputLayout(const D3D11_SHADER_DESC& desc, ID3D11ShaderReflection& ref, const InputHandler& handler);
		void SRK_CALL _parseParameterLayout(const D3D11_SHADER_DESC& desc, ID3D11ShaderReflection& ref, ParameterLayout& dst);
		void SRK_CALL _parseConstantVar(ConstantBufferLayout::Variables& var, ID3D11ShaderReflectionType* type);
		void SRK_CALL _calcConstantLayoutSameBuffers(std::vector<std::vector<MyConstantBufferLayout>*>& constBufferLayouts);

		ConstantBuffer* _getConstantBuffer(const MyConstantBufferLayout& cbLayout, const IShaderParameterGetter& paramGetter);
		void _constantBufferUpdateAll(ConstantBuffer* cb, const std::vector<ConstantBufferLayout::Variables>& vars);

		template<ProgramStage Stage>
		void SRK_CALL _useParameters(const ParameterLayout& layout, const IShaderParameterGetter& paramGetter) {
			auto g = _graphics.get<Graphics>();

			for (auto& info : layout.constantBuffers) {
				auto cb = _getConstantBuffer(info, paramGetter);
				if (cb && g == cb->getGraphics()) {
					if (auto native = (ConstantBuffer*)cb->getNative(); native) {
						if (auto buffer = native->getInternalBuffer(); buffer) g->useConstantBuffers<Stage>(info.bindPoint, 1, &buffer);
					}
				}
			}

			for (auto& info : layout.textures) {
				if (auto p = paramGetter.get(info.name, ShaderParameterType::TEXTURE); p) {
					if (auto data = p->getData(); data && g == ((ITextureView*)data)->getGraphics()) {
						if (auto native = (TextureView*)((ITextureView*)data)->getNative(); native) {
							auto iv = native->getInternalView();
							g->useShaderResources<Stage>(info.bindPoint, 1, &iv);
						}
					}
					//if (data) (BaseTexture*)(((ITexture*)data)->getNative())->use<stage>((Graphics*)_graphics, info.bindPoint);
				}
			}

			for (auto& info : layout.samplers) {
				if (auto p = paramGetter.get(info.name, ShaderParameterType::SAMPLER); p) {
					if (auto data = p->getData(); data && g == ((ISampler*)data)->getGraphics()) {
						if (auto native = (Sampler*)((ISampler*)data)->getNative(); native) {
							native->update();
							auto is = native->getInternalSampler();
							g->useSamplers<Stage>(info.bindPoint, 1, &is);
						}
					}
				}
			}
		}
	};
}