#pragma once

#include "Base.h"

namespace aurora::modules::graphics::win_d3d11 {
	class Graphics;

	class AE_MODULE_DLL Program : public IProgram {
	public:
		Program(Graphics& graphics);
		virtual ~Program();

		virtual bool AE_CALL upload(const ProgramSource& vert, const ProgramSource& frag) override;
		virtual bool AE_CALL use() override;
		virtual void AE_CALL useVertexBuffers(const VertexBufferFactory& factory) override;
		virtual void AE_CALL draw(const IIndexBuffer& indexBuffer, ui32 count = 0xFFFFFFFFui32, ui32 offset = 0) override;

	protected:
		struct InVertexBufferInfo {
			std::string name;
			ui32 slot;
		};


		struct InLayout {
			~InLayout();

			ui32* formats;
			ID3D11InputLayout* layout;

			bool isEqual(const D3D11_INPUT_ELEMENT_DESC* inElements, ui32 num) const;
		};


		bool _inElementsDirty;

		ID3DBlob* _vertBlob;
		ID3D11VertexShader* _vs;
		ID3D11PixelShader* _ps;

		ID3D11InputLayout* _curInLayout;
		D3D11_INPUT_ELEMENT_DESC* _inElements;
		ui32 _numInElements;

		std::vector<InVertexBufferInfo> _inVerBufInfos;
		std::vector<InLayout> _inLayouts;

		ui32 _numConstBuffers;

		void AE_CALL _release();
		ID3DBlob* AE_CALL _compileShader(const ProgramSource& source, const i8* target);
		ID3D11InputLayout* _getOrCreateInputLayout();
	};
}