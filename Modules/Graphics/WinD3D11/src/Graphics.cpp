#include "Graphics.h"
#include "base/Application.h"
#include "CreateModule.h"

namespace aurora::modules::graphics::win_d3d11 {
	Graphics::Graphics(Application* app) :
		_app(app->ref<Application>()),
		_driverType(D3D_DRIVER_TYPE_NULL),
		_featureLevel(D3D_FEATURE_LEVEL_11_0),
		_device(nullptr),
		_context(nullptr),
		_swapChain(nullptr),
		_backBufferTarget(nullptr) {
	}

	Graphics::~Graphics() {
		_release();
		Ref::setNull(_app);
	}

	bool Graphics::createDevice() {
		if (_device) return false;

		D3D_DRIVER_TYPE driverTypes[] =
		{
		   D3D_DRIVER_TYPE_HARDWARE, D3D_DRIVER_TYPE_WARP,
		   D3D_DRIVER_TYPE_REFERENCE, D3D_DRIVER_TYPE_SOFTWARE
		};
		ui32 totalDriverTypes = ARRAYSIZE(driverTypes);

		D3D_FEATURE_LEVEL featureLevels[] =
		{
		   D3D_FEATURE_LEVEL_11_0,
		   D3D_FEATURE_LEVEL_10_1,
		   D3D_FEATURE_LEVEL_10_0
		};
		ui32 totalFeatureLevels = ARRAYSIZE(featureLevels);

		i32 w, h;
		_app->getInnerSize(w, h);

		DXGI_SWAP_CHAIN_DESC swapChainDesc;
		memset(&swapChainDesc, 0, sizeof(swapChainDesc));
		swapChainDesc.BufferCount = 1;
		swapChainDesc.BufferDesc.Width = w;
		swapChainDesc.BufferDesc.Height = h;
		swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.OutputWindow = _app->$Win$_getHWND();
		swapChainDesc.Windowed = true;
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SampleDesc.Quality = 0;

		ui32 creationFlags = 0;
#ifdef AE_DEBUG
		creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

		HRESULT hr;

		for (ui32 i = 0; i < totalDriverTypes; ++i) {
			hr = D3D11CreateDeviceAndSwapChain(0, driverTypes[i], 0, creationFlags,
				featureLevels, totalFeatureLevels,
				D3D11_SDK_VERSION, &swapChainDesc, &_swapChain,
				&_device, &_featureLevel, &_context);

			if (SUCCEEDED(hr)) {
				_driverType = driverTypes[i];
				break;
			}
		}

		if (FAILED(hr)) {
			_release();
			return false;
		}

		ID3D11Texture2D* backBufferTexture;

		hr = _swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBufferTexture);

		if (FAILED(hr)) {
			_release();
			return false;
		}

		hr = _device->CreateRenderTargetView(backBufferTexture, 0, &_backBufferTarget);
		if (backBufferTexture) backBufferTexture->Release();

		if (FAILED(hr)) {
			_release();
			return false;
		}

		_context->OMSetRenderTargets(1, &_backBufferTarget, 0);

		D3D11_VIEWPORT vp;
		vp.Width = f32(w);
		vp.Height = f32(h);
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		vp.TopLeftX = 0.0f;
		vp.TopLeftY = 0.0f;

		_context->RSSetViewports(1, &vp);

		return true;
	}

	IIndexBuffer* Graphics::createIndexBuffer() {
		return nullptr;
	}

	IProgram* Graphics::createProgram() {
		return new Program(*this);
	}

	IVertexBuffer* Graphics::createVertexBuffer() {
		return new VertexBuffer(*this);
	}

	void Graphics::beginRender() {
		f32 clearColor[] = { 0.0f, 0.0f, 0.25f, 1.0f };
		_context->ClearRenderTargetView(_backBufferTarget, clearColor);
	}

	void Graphics::endRender() {
	}

	void Graphics::present() {
		_swapChain->Present(0, 0);
	}

	void Graphics::clear() {
	}

	void Graphics::_release() {
		if (_backBufferTarget) {
			_backBufferTarget->Release();
			_backBufferTarget = nullptr;
		}
		if (_swapChain) {
			_swapChain->Release();
			_swapChain = nullptr;
		}
		if (_context) {
			_context->Release();
			_context = nullptr;
		}
		if (_device) {
			_device->Release();
			_device = nullptr;
		}
	}
}