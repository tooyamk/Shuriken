#pragma once

#include "Program.h"
#include "VertexBuffer.h"

namespace aurora::modules::graphics_win_d3d11 {
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

		inline ID3D11Device* AE_CALL getDevice() const {
			return _device;
		}

		inline ID3D11DeviceContext* AE_CALL getContext() const {
			return _context;
		}

	private:
		Application* _app;

		D3D_DRIVER_TYPE _driverType;
		D3D_FEATURE_LEVEL _featureLevel;
		ID3D11Device* _device;
		ID3D11DeviceContext* _context;
		IDXGISwapChain* _swapChain;
		ID3D11RenderTargetView* _backBufferTarget;

		void AE_CALL _release();
	};
}