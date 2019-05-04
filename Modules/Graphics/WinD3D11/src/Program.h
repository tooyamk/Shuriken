#pragma once

#include "ConstantBuffer.h"
#include "Sampler.h"
#include "modules/graphics/ConstantBufferManager.h"

namespace aurora::modules::graphics::win_d3d11 {
	class Graphics;


	class AE_MODULE_DLL Program : public IProgram {
	public:
		Program(Graphics& graphics);
		virtual ~Program();

		virtual bool AE_CALL upload(const ProgramSource& vert, const ProgramSource& frag) override;
		virtual bool AE_CALL use() override;
		virtual void AE_CALL draw(const VertexBufferFactory* vertexFactory, const ShaderParameterFactory* paramFactory,
			const IIndexBuffer* indexBuffer, ui32 count = 0xFFFFFFFFui32, ui32 offset = 0) override;

	protected:
		struct MyConstantBufferLayout : public ConstantBufferLayout {
			ui32 sameId;
		};


		struct InVertexBufferInfo {
			std::string name;
			ui32 slot;
		};


		struct InLayout {
			InLayout(ui32 numInElements);
			~InLayout();

			std::vector<ui32> formats;
			ID3D11InputLayout* layout;

			bool isEqual(const D3D11_INPUT_ELEMENT_DESC* inElements, ui32 num) const;
		};


		ID3DBlob* _vertBlob;
		ID3D11VertexShader* _vs;
		ID3D11PixelShader* _ps;

		ID3D11InputLayout* _curInLayout;
		D3D11_INPUT_ELEMENT_DESC* _inElements;
		ui32 _numInElements;

		std::vector<InVertexBufferInfo> _inVerBufInfos;
		std::vector<InLayout> _inLayouts;


		struct TextureLayout {
			std::string name;
			ui32 bindPoint;
		};


		struct SamplerLayout {
			std::string name;
			ui32 bindPoint;
		};


		struct ParameterLayout {
			std::vector<MyConstantBufferLayout> constantBuffers;
			std::vector<TextureLayout> textures;
			std::vector<SamplerLayout> samplers;

			void clear(Graphics& g);
		};


		struct ParameterUsageStatistics {
			ui16 exclusiveCount = 0;
			ui16 autoCount = 0;
			ui16 shareCount = 0;
			ui16 unknownCount = 0;
		};


		ParameterLayout _vsParamLayout;
		ParameterLayout _psParamLayout;

		std::vector<ConstantBuffer*> _usingSameConstBuffers;
		std::vector<ShaderParameter*> _tempParams;
		std::vector<const ConstantBufferLayout::Variables*> _tempVars;

		void AE_CALL _release();
		ID3DBlob* AE_CALL _compileShader(const ProgramSource& source, const i8* target);
		ID3D11InputLayout* _getOrCreateInputLayout();
		void AE_CALL _parseInLayout(const D3D11_SHADER_DESC& desc, ID3D11ShaderReflection& ref);
		void AE_CALL _parseParameterLayout(const D3D11_SHADER_DESC& desc, ID3D11ShaderReflection& ref, ParameterLayout& dst);
		void AE_CALL _parseConstantVar(ConstantBufferLayout::Variables& var, ID3D11ShaderReflectionType* type);
		void AE_CALL _calcConstantLayoutSameBuffers(std::vector<std::vector<MyConstantBufferLayout>*>& constBufferLayouts);

		ConstantBuffer* _getConstantBuffer(const MyConstantBufferLayout& cbLayout, const ShaderParameterFactory& factory);
		void _collectParameters(const ConstantBufferLayout::Variables& var, const ShaderParameterFactory& factory, ParameterUsageStatistics& statistics);
		void _updateConstantBuffer(ConstantBuffer* cb, const ShaderParameter& param, const ConstantBufferLayout::Variables& vars);
		void _constantBufferUpdateAll(ConstantBuffer* cb, const std::vector<ConstantBufferLayout::Variables>& var);

		template<ProgramStage stage>
		void AE_CALL _useParameters(const ParameterLayout& layout, const ShaderParameterFactory& factory) {
			for (auto& info : layout.constantBuffers) {
				auto cb = _getConstantBuffer(info, factory);
				if (cb) cb->use<stage>(info.bindPoint);
			}

			for (auto& info : layout.textures) {
				auto c = factory.get(info.name, ShaderParameterType::TEXTURE);
				if (c) {
					auto data = c->getData();
					if (data && _graphics == ((ITextureViewBase*)data)->getGraphics()) {
						auto view = (ID3D11ShaderResourceView*)((ITextureViewBase*)data)->getNativeView();
						_graphics.get<Graphics>()->useShaderResources<stage>(info.bindPoint, 1, &view);
					}
					//if (data) (BaseTexture*)(((ITexture*)data)->getNative())->use<stage>((Graphics*)_graphics, info.bindPoint);
				}
			}

			for (auto& info : layout.samplers) {
				auto c = factory.get(info.name, ShaderParameterType::SAMPLER);
				if (c) {
					auto data = c->getData();
					if (data && _graphics == ((ISampler*)data)->getGraphics()) ((Sampler*)data)->use<stage>(info.bindPoint);
				}
			}
		}
	};
}