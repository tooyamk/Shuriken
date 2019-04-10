#pragma once

#include "IndexBuffer.h"
#include "Program.h"
#include "VertexBuffer.h"

namespace aurora::modules::graphics::win_d3d11 {
	class AE_MODULE_DLL Graphics : public IGraphicsModule {
	public:
		Graphics(Application* app);
		virtual ~Graphics();

		virtual bool AE_CALL createDevice(const GraphicsAdapter* adapter) override;

		virtual IIndexBuffer* AE_CALL createIndexBuffer() override;
		virtual IProgram* AE_CALL createProgram() override;
		virtual IVertexBuffer* AE_CALL createVertexBuffer() override;
		
		virtual void AE_CALL beginRender() override;
		virtual void AE_CALL endRender() override;
		virtual void AE_CALL present() override;

		virtual void AE_CALL clear() override;

		inline ID3D11Device5* AE_CALL getDevice() const {
			return _device;
		}

		inline ID3D11DeviceContext4* AE_CALL getContext() const {
			return _context;
		}

	private:
		class DXObjGuard {
		public:
			~DXObjGuard();

			void add(IUnknown* obj);
			void clear();
		private:
			std::vector<IUnknown*> _objs;
		};


		Application* _app;

		DXGI_RATIONAL _refreshRate;
		//D3D_DRIVER_TYPE _driverType;
		ID3D11Device5* _device;
		ID3D11DeviceContext4* _context;
		IDXGISwapChain4* _swapChain;
		ID3D11RenderTargetView1* _backBufferTarget;

		events::EventListener<ApplicationEvent, Graphics> _resizedListener;
		void AE_CALL _resizedHandler(events::Event<ApplicationEvent>& e);

		bool AE_CALL _createDevice(const GraphicsAdapter& adapter);

		void AE_CALL _release();
		void AE_CALL _resize(UINT w, UINT h);
	};
}