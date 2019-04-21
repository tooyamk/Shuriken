#pragma once

#include "Base.h"
#include <unordered_set>
#include <bitset>

namespace aurora::modules::graphics::win_d3d11 {
	class ConstantBuffer;
	struct ConstantBufferLayout;

	class AE_MODULE_DLL Graphics : public IGraphicsModule {
	public:
		Graphics(Application* app);
		virtual ~Graphics();

		virtual bool AE_CALL createDevice(const GraphicsAdapter* adapter) override;

		virtual IConstantBuffer* AE_CALL createConstantBuffer() override;
		virtual IIndexBuffer* AE_CALL createIndexBuffer() override;
		virtual IProgram* AE_CALL createProgram() override;
		virtual ITexture2D* AE_CALL createTexture2D() override;
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

		void AE_CALL registerConstantLayout(ConstantBufferLayout& layout);
		void AE_CALL unregisterConstantLayout(ConstantBufferLayout& layout);
		ConstantBuffer* AE_CALL popShareConstantBuffer(ui32 size);
		void AE_CALL resetUsedShareConstantBuffers();
		ConstantBuffer* AE_CALL getExclusiveConstantBuffer(const std::vector<Constant*>& constants, const ConstantBufferLayout& layout);

		static DXGI_FORMAT AE_CALL getDXGIFormat(TextureFormat fmt);

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

		void AE_CALL _registerShareConstantLayout(ui32 size);
		void AE_CALL _unregisterShareConstantLayout(ui32 size);

		struct ExclusiveConstNode {
			ExclusiveConstNode() :
				numAssociativeBuffers(0),
				constant(nullptr),
				parent(nullptr) {
			}

			ui32 numAssociativeBuffers;
			std::unordered_map<const Constant*, ExclusiveConstNode> children;
			std::unordered_map<ui64, ConstantBuffer*> buffers;
			Constant* constant;
			ExclusiveConstNode* parent;
		};
		std::unordered_map<const Constant*, ExclusiveConstNode> _exclusiveConstRoots;
		std::unordered_map<const Constant*, std::unordered_set<ExclusiveConstNode*>> _exclusiveConstNodes;

		struct ExcLusiveConsts {
			ui32 rc;
			std::unordered_set<ExclusiveConstNode*> nodes;
		};
		std::unordered_map<ui64, ExcLusiveConsts> _exclusiveConstPool;

		ConstantBuffer* AE_CALL _getExclusiveConstantBuffer(const ConstantBufferLayout& layout, const std::vector<Constant*>& constants,
			ui32 cur, ui32 max, ExclusiveConstNode* parent, std::unordered_map <const Constant*, ExclusiveConstNode>& chindrenContainer);
		void AE_CALL _registerExclusiveConstantLayout(ConstantBufferLayout& layout);
		void AE_CALL _unregisterExclusiveConstantLayout(ConstantBufferLayout& layout);
		static void _releaseExclusiveConstant(void* graphics, const Constant& constant);
		void _releaseExclusiveConstant(const Constant& constant);
		void _releaseExclusiveConstantToRoot(ExclusiveConstNode* parent, ExclusiveConstNode* releaseChild, ui32 releaseNumAssociativeBuffers, bool releaseConstant);
		void _releaseExclusiveConstantToLeaf(ExclusiveConstNode& node, bool releaseConstant);
		void _releaseExclusiveConstantSelf(ExclusiveConstNode& node, bool releaseConstant);

		events::EventListener<ApplicationEvent, Graphics> _resizedListener;
		void AE_CALL _resizedHandler(events::Event<ApplicationEvent>& e);

		bool AE_CALL _createDevice(const GraphicsAdapter& adapter);

		void AE_CALL _release();
		void AE_CALL _resize(UINT w, UINT h);
	};
}