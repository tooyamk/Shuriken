#pragma once

#include "ConstantBuffer.h"
#include "Sampler.h"
#include "TextureView.h"
#include "modules/graphics/ConstantBufferManager.h"

namespace aurora::modules::graphics::win_d3d11 {
	class AE_MODULE_DLL Program : public IProgram {
	public:
		Program(Graphics& graphics);
		virtual ~Program();

		virtual const void* AE_CALL getNative() const override;
		virtual bool AE_CALL create(const ProgramSource& vert, const ProgramSource& frag) override;
		virtual void AE_CALL destroy() override;

		bool AE_CALL use(const VertexBufferFactory* vertexFactory, const ShaderParameterFactory* paramFactory);
		void AE_CALL useEnd();

	protected:
		class MyConstantBufferLayout : public ConstantBufferLayout {
		public:
			MyConstantBufferLayout() : sameId(0) {}

			uint32_t sameId;
		};


		struct InVertexBufferInfo {
			InVertexBufferInfo() :
				name(),
				slot(0) {
			}

			std::string name;
			uint32_t slot;
		};


		struct InLayout {
			InLayout (uint32_t numInElements);
			~InLayout();

			std::vector<uint32_t> formats;
			ID3D11InputLayout* layout;

			bool isEqual(const D3D11_INPUT_ELEMENT_DESC* inElements, uint32_t num) const;
		};


		ID3DBlob* _vertBlob;
		ID3D11VertexShader* _vs;
		ID3D11PixelShader* _ps;

		ID3D11InputLayout* _curInLayout;
		D3D11_INPUT_ELEMENT_DESC* _inElements;
		uint32_t _numInElements;

		std::vector<InVertexBufferInfo> _inVerBufInfos;
		std::vector<InLayout> _inLayouts;


		struct TextureLayout {
			std::string name;
			uint32_t bindPoint;
		};


		struct SamplerLayout {
			std::string name;
			uint32_t bindPoint;
		};


		struct ParameterLayout {
			std::vector<MyConstantBufferLayout> constantBuffers;
			std::vector<TextureLayout> textures;
			std::vector<SamplerLayout> samplers;

			void clear(Graphics& g);
		};


		ParameterLayout _vsParamLayout;
		ParameterLayout _psParamLayout;

		std::vector<ConstantBuffer*> _usingSameConstBuffers;
		std::vector<ShaderParameter*> _tempParams;
		std::vector<const ConstantBufferLayout::Variables*> _tempVars;

		ID3DBlob* AE_CALL _compileShader(const ProgramSource& source, const char* target);
		ID3D11InputLayout* _getOrCreateInputLayout();
		void AE_CALL _parseInLayout(const D3D11_SHADER_DESC& desc, ID3D11ShaderReflection& ref);
		void AE_CALL _parseParameterLayout(const D3D11_SHADER_DESC& desc, ID3D11ShaderReflection& ref, ParameterLayout& dst);
		void AE_CALL _parseConstantVar(ConstantBufferLayout::Variables& var, ID3D11ShaderReflectionType* type);
		void AE_CALL _calcConstantLayoutSameBuffers(std::vector<std::vector<MyConstantBufferLayout>*>& constBufferLayouts);

		ConstantBuffer* _getConstantBuffer(const MyConstantBufferLayout& cbLayout, const ShaderParameterFactory& factory);
		void _constantBufferUpdateAll(ConstantBuffer* cb, const std::vector<ConstantBufferLayout::Variables>& vars);

		template<ProgramStage Stage>
		void AE_CALL _useParameters(const ParameterLayout& layout, const ShaderParameterFactory& factory) {
			auto g = _graphics.get<Graphics>();

			for (auto& info : layout.constantBuffers) {
				auto cb = _getConstantBuffer(info, factory);
				if (cb && g == cb->getGraphics()) {
					if (auto native = (ConstantBuffer*)cb->getNative(); native) {
						if (auto buffer = native->getInternalBuffer(); buffer) g->useConstantBuffers<Stage>(info.bindPoint, 1, &buffer);
					}
				}
			}

			for (auto& info : layout.textures) {
				if (auto p = factory.get(info.name, ShaderParameterType::TEXTURE); p) {
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
				if (auto p = factory.get(info.name, ShaderParameterType::SAMPLER); p) {
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