#pragma once

#include "Base.h"

namespace aurora::modules::graphics::win_d3d11 {
	class Graphics;

	class AE_MODULE_DLL Program : public IProgram {
	public:
		Program(Graphics& graphics);
		virtual ~Program();

		virtual bool AE_CALL upload(const ProgramSource& vert, const ProgramSource& frag) override;
		virtual void AE_CALL use() override;

	protected:
		struct InVertexBufferInfo {
			std::string name;
			ui32 slot;
			ui32 stride;
		};


		ID3D11VertexShader* _vs;
		ID3D11PixelShader* _ps;
		ID3D11InputLayout* _inputLayout;
		std::vector<InVertexBufferInfo> _inVerBufInfos;

		void AE_CALL _release();
		ID3DBlob* AE_CALL _compileShader(const ProgramSource& source, const i8* target);
	};
}