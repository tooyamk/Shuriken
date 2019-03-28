#pragma once

#include "math/Rect.h"
#include "Program.h"
#include "VertexBuffer.h"

namespace aurora::modules::graphics_win_glew {
	class AE_MODULE_DLL Graphics : public GraphicsModule {
	public:
		Graphics(Graphics::CREATE_PARAMS_REF params);
		virtual ~Graphics();

		virtual bool AE_CALL createDevice() override;

		virtual GraphicsModule::VertexBuffer* AE_CALL createVertexBuffer() override;
		virtual GraphicsModule::Program* AE_CALL createProgram() override;
		
		virtual void AE_CALL beginRender() override;
		virtual void AE_CALL endRender() override;
		virtual void AE_CALL present() override;

		virtual void AE_CALL clear() override;

	private:
		Application* _app;

		HDC _dc;
		HGLRC _rc;

		void AE_CALL _release();
	};
}