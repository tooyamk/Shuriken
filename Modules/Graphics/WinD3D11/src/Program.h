#pragma once

#include "Base.h"

namespace aurora::modules::graphics_win_d3d11 {
	class Graphics;

	class AE_MODULE_DLL Program : public IGraphicsProgram {
	public:
		Program(Graphics& graphics);
		virtual ~Program();

		virtual bool AE_CALL upload(const i8* vert, const i8* frag) override;
		virtual void AE_CALL use() override;

	protected:
		ID3D11VertexShader* _vs;
		ID3D11PixelShader* _ps;

		void AE_CALL _release();
		ID3DBlob* AE_CALL _compileShader(const i8* source, const i8* entry, const i8* target);
	};
}