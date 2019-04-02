#pragma once

#include "IndexBuffer.h"
#include "Program.h"
#include "VertexBuffer.h"

namespace aurora::modules::graphics_win_glew {
	class AE_MODULE_DLL Graphics : public IGraphicsModule {
	public:
		Graphics(Application* app);
		virtual ~Graphics();

		virtual bool AE_CALL createDevice() override;

		virtual IGraphicsIndexBuffer* AE_CALL createIndexBuffer() override;
		virtual IGraphicsProgram* AE_CALL createProgram() override;
		virtual IGraphicsVertexBuffer* AE_CALL createVertexBuffer() override;
		
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