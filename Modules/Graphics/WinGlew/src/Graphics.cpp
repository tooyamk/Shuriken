#include "Graphics.h"
#include "base/Application.h"
#include "base/String.h"
#include "CreateModule.h"

namespace aurora::modules::graphics::win_glew {
	Graphics::Graphics(Application* app, IProgramSourceTranslator* trans) :
		_app(app->ref<Application>()),
		_trans(trans ? trans->ref<IProgramSourceTranslator>() : nullptr),
		_dc(nullptr),
		_rc(nullptr),
		_majorVer(0),
		_minorVer(0),
		_intVer(0) {
	}

	Graphics::~Graphics() {
		_release();
		Ref::setNull(_trans);
		Ref::setNull(_app);
	}

	bool Graphics::createDevice(const GraphicsAdapter* adapter) {
		if (_dc || !_app->Win_getHWND()) return false;

		_dc = GetDC(_app->Win_getHWND());
		if (!_dc) return false;

		PIXELFORMATDESCRIPTOR pfd;
		pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
		pfd.nVersion = 1;
		pfd.dwFlags =
			PFD_DRAW_TO_WINDOW |            //格式支持窗口
			PFD_SUPPORT_OPENGL |            //格式必须支持OpenGL
			PFD_DOUBLEBUFFER;               //必须支持双缓冲
		pfd.iPixelType = PFD_TYPE_RGBA;     //申请RGBA 格式
		pfd.cColorBits = 24;                //选定色彩深度
		pfd.cRedBits = 0;                   //忽略RGBA
		pfd.cRedShift = 0;
		pfd.cGreenBits = 0;
		pfd.cGreenShift = 0;
		pfd.cBlueBits = 0;
		pfd.cBlueShift = 0;
		pfd.cAlphaBits = 0;
		pfd.cAlphaShift = 0;
		pfd.cAccumBits = 0;                 //无累加缓存
		pfd.cAccumRedBits = 0;              //忽略聚集位
		pfd.cAccumGreenBits = 0;
		pfd.cAccumBlueBits = 0;
		pfd.cAccumAlphaBits = 0;
		pfd.cDepthBits = 32;                //32位Z-缓存(深度缓存)
		pfd.cStencilBits = 0;               //无蒙板缓存
		pfd.cAuxBuffers = 0;                //无辅助缓存
		pfd.iLayerType = PFD_MAIN_PLANE;    //主绘图层
		pfd.bReserved = 0;                  //Reserved
		pfd.dwLayerMask = 0;                //忽略层遮罩
		pfd.dwVisibleMask = 0;
		pfd.dwDamageMask = 0;

		auto pf = ChoosePixelFormat(_dc, &pfd);
		if (!SetPixelFormat(_dc, pf, &pfd)) {
			_release();
			return false;
		}

		_rc = wglCreateContext(_dc);
		if (!_rc) {
			_release();
			return false;
		}

		wglMakeCurrent(_dc, _rc);
		if (glewInit() != GLEW_OK) {
			_release();
			return false;
		}

		glFrontFace(GL_CW);

		glGetIntegerv(GL_MAJOR_VERSION, &_majorVer);
		glGetIntegerv(GL_MINOR_VERSION, &_minorVer);

		_intVer = _majorVer * 100 + _minorVer * 10;
		_strVer = String::toString(_intVer);

		return true;
	}

	IIndexBuffer* Graphics::createIndexBuffer() {
		return new IndexBuffer(*this);
	}

	IProgram* Graphics::createProgram() {
		return new Program(*this);
	}

	IVertexBuffer* Graphics::createVertexBuffer() {
		return new VertexBuffer(*this);
	}

	void Graphics::beginRender() {
		wglMakeCurrent(_dc, _rc);

		i32 w, h;
		_app->getInnerSize(w, h);
		glViewport(0, 0, w, h);
	}

	void Graphics::endRender() {
		//交换当前缓冲区和后台缓冲区
		SwapBuffers(_dc);

		if (wglGetCurrentContext() == _rc) wglMakeCurrent(nullptr, nullptr);
	}

	void Graphics::present() {
	}

	void Graphics::clear() {
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
	}

	void Graphics::_release() {
		if (wglGetCurrentContext() == _rc) wglMakeCurrent(nullptr, nullptr);

		if (_rc) {
			wglDeleteContext(_rc);
			_rc = nullptr;
		}

		if (_dc) {
			ReleaseDC(_app->Win_getHWND(), _dc);
			_dc = nullptr;
		}
	}
}