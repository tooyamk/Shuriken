#pragma once

#include "Base.h"
#include <unordered_set>

namespace aurora::modules::graphics::win_d3d11 {
	class ConstantBuffer;

	class AE_MODULE_DLL Graphics : public IGraphicsModule {
	public:
		Graphics(Application* app);
		virtual ~Graphics();

		virtual bool AE_CALL createDevice(const GraphicsAdapter* adapter) override;

		virtual IConstantBuffer* AE_CALL createConstantBuffer() override;
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

		inline D3D_FEATURE_LEVEL AE_CALL getFeatureLevel() const {
			return _featureLevel;
		}

		inline const std::string& AE_CALL getSupportShaderModel() const {
			return _shaderModel;
		}

		inline const D3D11_FEATURE_DATA_D3D11_OPTIONS& AE_CALL getFeatureOptions() const {
			return _featureOptions;
		}

		void AE_CALL refShareConstantBuffer(ui32 size);
		void AE_CALL unrefShareConstantBuffer(ui32 size);
		ConstantBuffer* AE_CALL popShareConstantBuffer(ui32 size);
		void AE_CALL releaseUsedShareConstantBuffers();

	private:
		Application* _app;

		DXGI_RATIONAL _refreshRate;
		D3D_FEATURE_LEVEL _featureLevel;
		std::string _shaderModel;
		//D3D_DRIVER_TYPE _driverType;
		ID3D11Device5* _device;
		ID3D11DeviceContext4* _context;
		IDXGISwapChain4* _swapChain;
		ID3D11RenderTargetView1* _backBufferTarget;
		D3D11_FEATURE_DATA_D3D11_OPTIONS _featureOptions;

		struct ShareConstBufferPool {
			ui32 rc;
			ui32 idleIndex;
			std::vector<ConstantBuffer*> buffers;
		};

		std::unordered_map<ui32, ShareConstBufferPool> _shareConstBufferPool;
		std::unordered_set<ui32> _usedShareConstBufferPool;

		struct ExcLusiveConstNode {
			std::unordered_set<Constant*> prev;
			std::unordered_set<Constant*> next;
		};
		std::unordered_map<Constant*, ExcLusiveConstNode> _excLusiveConstBufferPool;
		ConstantBuffer* AE_CALL getExcLusiveConstantBuffer(const std::vector<Constant*>& constants);
		ConstantBuffer* AE_CALL _getExcLusiveConstantBuffer(const std::vector<Constant*>& constants, ui32 cur, ui32 max, ExcLusiveConstNode* prev);

		events::EventListener<ApplicationEvent, Graphics> _resizedListener;
		void AE_CALL _resizedHandler(events::Event<ApplicationEvent>& e);

		bool AE_CALL _createDevice(const GraphicsAdapter& adapter);

		void AE_CALL _release();
		void AE_CALL _resize(UINT w, UINT h);
	};
}