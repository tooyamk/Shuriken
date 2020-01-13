#include "Graphics.h"
#include "CreateModule.h"
#include "BlendState.h"
#include "ConstantBuffer.h"
#include "IndexBuffer.h"
#include "PixelBuffer.h"
#include "Program.h"
#include "RasterizerState.h"
#include "Sampler.h"
#include "Texture2DResource.h"
#include "TextureView.h"
#include "VertexBuffer.h"
#include "base/Application.h"
#include "base/String.h"
#include "base/Time.h"
#include "GL/wglew.h"

namespace aurora::modules::graphics::win_gl {
	Graphics::Graphics(Ref* loader, Application* app, IProgramSourceTranslator* trans) :
		_loader(loader),
		_createBufferMask(Usage::NONE),
		_app(app),
		_trans(trans),
		_dc(nullptr),
		_rc(nullptr),
		_majorVer(0),
		_minorVer(0),
		_intVer(0) {
		_constantBufferManager.createShareConstantBufferCallback = std::bind(&Graphics::_createdShareConstantBuffer, this);
		_constantBufferManager.createExclusiveConstantBufferCallback = std::bind(&Graphics::_createdExclusiveConstantBuffer, this, std::placeholders::_1);
		memset(&_internalFeatures, 0, sizeof(InternalFeatures));
		memset(&_deviceFeatures, 0, sizeof(_deviceFeatures));
	}

	Graphics::~Graphics() {
		_release();
	}

	bool Graphics::createDevice(const GraphicsAdapter* adapter) {
		if (_dc || !_app->Win_getHWnd()) return false;

		/*
		pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
		pfd.nVersion = 1;
		pfd.dwFlags =
			PFD_DRAW_TO_WINDOW |            //��ʽ֧�ִ���
			PFD_SUPPORT_OPENGL |            //��ʽ����֧��OpenGL
			PFD_DOUBLEBUFFER;               //����֧��˫����
		pfd.iPixelType = PFD_TYPE_RGBA;     //����RGBA ��ʽ
		pfd.cColorBits = 24;                //ѡ��ɫ�����
		pfd.cRedBits = 0;                   //����RGBA
		pfd.cRedShift = 0;
		pfd.cGreenBits = 0;
		pfd.cGreenShift = 0;
		pfd.cBlueBits = 0;
		pfd.cBlueShift = 0;
		pfd.cAlphaBits = 0;
		pfd.cAlphaShift = 0;
		pfd.cAccumBits = 0;                 //���ۼӻ���
		pfd.cAccumRedBits = 0;              //���Ծۼ�λ
		pfd.cAccumGreenBits = 0;
		pfd.cAccumBlueBits = 0;
		pfd.cAccumAlphaBits = 0;
		pfd.cDepthBits = 32;                //32λZ-����(��Ȼ���)
		pfd.cStencilBits = 0;               //���ɰ建��
		pfd.cAuxBuffers = 0;                //�޸�������
		pfd.iLayerType = PFD_MAIN_PLANE;    //����ͼ��
		pfd.bReserved = 0;                  //Reserved
		pfd.dwLayerMask = 0;                //���Բ�����
		pfd.dwVisibleMask = 0;
		pfd.dwDamageMask = 0;
		*/

		if (!_glInit()) {
			_release();
			return false;
		}

		_dc = GetDC(_app->Win_getHWnd());
		if (!_dc) return false;

		int32_t iAttribIList[] = {
			WGL_SUPPORT_OPENGL_ARB, 1,
			WGL_DRAW_TO_WINDOW_ARB, 1,
			WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
			WGL_COLOR_BITS_ARB, 32,
			WGL_DEPTH_BITS_ARB, 24,
			WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
			WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
			WGL_STENCIL_BITS_ARB, 8,
			WGL_SAMPLE_BUFFERS_ARB, GL_TRUE,//MSAA
			WGL_SAMPLES_ARB, 4,//4x AA
			//WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,
			0 };

		int32_t nPixelFormat = -1;
		uint32_t nPixCount = 0;

		wglChoosePixelFormatARB(_dc, iAttribIList, nullptr, 1, &nPixelFormat, &nPixCount);
		if (nPixelFormat == -1) {
			_release();
			return false;
		}

		PIXELFORMATDESCRIPTOR pfd = { 0 };
		if (!SetPixelFormat(_dc, nPixelFormat, &pfd)) {
			_release();
			return false;
		}
		
		GLint attribs[] = {
			//WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
			//WGL_CONTEXT_MINOR_VERSION_ARB, 3,
			WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,//����ģʽ����
			//WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,//���Ĺ��ܻ���
#ifdef AE_DEBUG
			WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_DEBUG_BIT_ARB,
#endif
			0
		};

		_rc = wglCreateContextAttribsARB(_dc, nullptr, attribs);
		if (!_rc) {
			_release();
			return false;
		}

		wglMakeCurrent(_dc, _rc);

		glGetIntegerv(GL_MAJOR_VERSION, &_majorVer);
		glGetIntegerv(GL_MINOR_VERSION, &_minorVer);

#ifdef AE_DEBUG
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(&Graphics::_debugCallback, NULL);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
#endif

		_intVer = _majorVer * 100 + _minorVer * 10;
		_strVer = String::toString(_intVer);
		_deviceVersion = "OpenGL " + String::toString(_majorVer) + "." + String::toString(_minorVer);

		_internalFeatures.maxAnisotropy = 1.f;
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &_internalFeatures.maxAnisotropy);
		

		_deviceFeatures.supportPixelBuffer = true;
		_deviceFeatures.supportConstantBuffer = isGreatThanOrEqualVersion(3, 1);
		_deviceFeatures.supportSampler = isGreatThanOrEqualVersion(3, 3);
		_deviceFeatures.supportIndependentBlend = isGreatThanOrEqualVersion(4, 0);
		_internalFeatures.supportTexStorage = isGreatThanOrEqualVersion(4, 2);
		_deviceFeatures.supportTextureView = isGreatThanOrEqualVersion(4, 3);
		_deviceFeatures.supportPersistentMap = isGreatThanOrEqualVersion(4, 4);

		_createBufferMask = Usage::MAP_READ_WRITE | Usage::UPDATE | (_deviceFeatures.supportPersistentMap ? Usage::PERSISTENT_MAP : Usage::NONE);

		_setInitState();

		return true;
	}

	const std::string& Graphics::getVersion() const {
		return _deviceVersion;
	}

	const GraphicsDeviceFeatures& Graphics::getDeviceFeatures() const {
		return _deviceFeatures;
	}

	IBlendState* Graphics::createBlendState() {
		return new BlendState(*this);
	}

	IConstantBuffer* Graphics::createConstantBuffer() {
		return _deviceFeatures.supportConstantBuffer ? new ConstantBuffer(*this) : nullptr;
	}

	IIndexBuffer* Graphics::createIndexBuffer() {
		return new IndexBuffer(*this);
	}

	IProgram* Graphics::createProgram() {
		return new Program(*this);
	}

	IRasterizerState* Graphics::createRasterizerState() {
		return new RasterizerState(*this);
	}

	ISampler* Graphics::createSampler() {
		return _deviceFeatures.supportSampler ? new Sampler(*this) : nullptr;
	}

	ITexture1DResource* Graphics::createTexture1DResource() {
		return nullptr;
	}

	ITexture2DResource* Graphics::createTexture2DResource() {
		return new Texture2DResource(*this);
	}

	ITexture3DResource* Graphics::createTexture3DResource() {
		return nullptr;
	}

	ITextureView* Graphics::createTextureView() {
		return _deviceFeatures.supportTextureView ? new TextureView(*this) : nullptr;
	}

	IVertexBuffer* Graphics::createVertexBuffer() {
		return new VertexBuffer(*this);
	}

	IPixelBuffer* Graphics::createPixelBuffer() {
		return new PixelBuffer(*this);
	}

	void Graphics::setBlendState(IBlendState* state, const Vec4f32& constantFactors, uint32_t sampleMask) {
		if (state && state->getGraphics() == this) {
			auto bs = (BlendState*)state;
			auto& blend = _glStatus.blend;

			if (_deviceFeatures.supportIndependentBlend) {
				if (bs->isIndependentBlendEnabled()) {
					auto funcChanged = false, opChanged = false, writeMaskChanged = false;
					
					for (size_t i = 0; i < MAX_RTS; ++i) {
						auto& rt = bs->getInternalRenderTargetState(i);
						if (bool(blend.enabled >> i & 0x1) != rt.state.enabled) {
							if (rt.state.enabled) {
								glEnablei(GL_BLEND, i);
								blend.enabled |= 1 << i;
							} else {
								glDisablei(GL_BLEND, i);
								blend.enabled &= ~(1 << i);
							}
						}

						if (rt.state.enabled) {
							if (blend.func[i].featureValue != rt.internalFunc.featureValue) {
								glBlendFuncSeparatei(i, rt.internalFunc.srcColor, rt.internalFunc.dstColor, rt.internalFunc.srcAlpha, rt.internalFunc.dstAlpha);
								blend.func[i].featureValue = rt.internalFunc.featureValue;
								funcChanged = true;
							}

							if (blend.op[i].featureValue != rt.internalOp.featureValue) {
								glBlendEquationSeparatei(i, rt.internalOp.color, rt.internalOp.alpha);
								blend.op[i].featureValue = rt.internalOp.featureValue;
								opChanged = true;
							}
						}

						if (blend.writeMask[i].featureValue != rt.internalWriteMask.featureValue) {
							glColorMaski(i, rt.internalWriteMask.rgba[0], rt.internalWriteMask.rgba[1], rt.internalWriteMask.rgba[2], rt.internalWriteMask.rgba[3]);
							blend.writeMask[i].featureValue = rt.internalWriteMask.featureValue;
							writeMaskChanged = true;
						}
					}
					
					if (funcChanged) _checkBlendFuncIsSame();
					if (opChanged) _checkBlendOpIsSame();
					if (writeMaskChanged) _checkBlendWriteMaskIsSame();
				} else {
					_setDependentBlendState<true>(bs->getInternalRenderTargetState(0));
				}
			} else {
				_setDependentBlendState<false>(bs->getInternalRenderTargetState(0));
			}

			if (blend.enabled && !memEqual<sizeof(Vec4f32)>(&blend.constantFactors, &constantFactors)) {
				glBlendColor(constantFactors.data[0], constantFactors.data[1], constantFactors.data[2], constantFactors.data[3]);
				memcpy(&blend.constantFactors, &constantFactors, sizeof(Vec4f32));
			}
		}
	}

	template<bool SupportIndependentBlend>
	void AE_CALL Graphics::_setDependentBlendState(const InternalRenderTargetBlendState& rt) {
		auto& blend = _glStatus.blend;
		if (rt.state.enabled) {
			if (blend.enabled != 0xFF) {
				glEnable(GL_BLEND);
				blend.enabled = 0xFF;
			}

			if constexpr (SupportIndependentBlend) {
				if (blend.isFuncSame) {
					if (blend.func[0].featureValue != rt.internalFunc.featureValue) {
						glBlendFuncSeparate(rt.internalFunc.srcColor, rt.internalFunc.dstColor, rt.internalFunc.srcAlpha, rt.internalFunc.dstAlpha);
						for (size_t i = 0; i < MAX_RTS; ++i) blend.func[i].featureValue = rt.internalFunc.featureValue;
					}
				} else {
					auto needChange = false;
					for (size_t i = 0; i < MAX_RTS; ++i) {
						if (blend.func[i].featureValue != rt.internalFunc.featureValue) {
							blend.func[i].featureValue = rt.internalFunc.featureValue;
							needChange = true;
						}
					}
					if (needChange) glBlendFuncSeparate(rt.internalFunc.srcColor, rt.internalFunc.dstColor, rt.internalFunc.srcAlpha, rt.internalFunc.dstAlpha);
					blend.isFuncSame = true;
				}

				if (blend.isOpSame) {
					if (blend.op[0].featureValue != rt.internalOp.featureValue) {
						glBlendEquationSeparate(rt.internalOp.color, rt.internalOp.alpha);
						for (size_t i = 0; i < MAX_RTS; ++i) blend.op[i].featureValue = rt.internalOp.featureValue;
					}
				} else {
					auto needChange = false;
					for (size_t i = 0; i < MAX_RTS; ++i) {
						if (blend.op[i].featureValue != rt.internalOp.featureValue) {
							blend.op[i].featureValue = rt.internalOp.featureValue;
							needChange = true;
						}
					}
					if (needChange) glBlendEquationSeparate(rt.internalOp.color, rt.internalOp.alpha);
					blend.isOpSame = true;
				}
			} else {
				if (blend.func[0].featureValue != rt.internalFunc.featureValue) {
					glBlendFuncSeparate(rt.internalFunc.srcColor, rt.internalFunc.dstColor, rt.internalFunc.srcAlpha, rt.internalFunc.dstAlpha);
					blend.func[0].featureValue = rt.internalFunc.featureValue;
				}

				if (blend.op[0].featureValue != rt.internalOp.featureValue) {
					glBlendEquationSeparate(rt.internalOp.color, rt.internalOp.alpha);
					blend.op[0].featureValue = rt.internalOp.featureValue;
				}

				if (blend.writeMask[0].featureValue != rt.internalWriteMask.featureValue) {
					glColorMask(rt.internalWriteMask.rgba[0], rt.internalWriteMask.rgba[1], rt.internalWriteMask.rgba[2], rt.internalWriteMask.rgba[3]);
					blend.writeMask[0].featureValue = rt.internalWriteMask.featureValue;
				}
			}
		} else {
			if (blend.enabled) {
				glDisable(GL_BLEND);
				blend.enabled = 0;
			}
		}

		if (blend.isWriteMaskSame) {
			if (blend.writeMask[0].featureValue != rt.internalWriteMask.featureValue) {
				glColorMask(rt.internalWriteMask.rgba[0], rt.internalWriteMask.rgba[1], rt.internalWriteMask.rgba[2], rt.internalWriteMask.rgba[3]);
				for (size_t i = 0; i < MAX_RTS; ++i) blend.writeMask[i].featureValue = rt.internalWriteMask.featureValue;
			}
		} else {
			auto needChange = false;
			for (size_t i = 0; i < MAX_RTS; ++i) {
				if (blend.writeMask[i].featureValue != rt.internalWriteMask.featureValue) {
					blend.writeMask[i].featureValue = rt.internalWriteMask.featureValue;
					needChange = true;
				}
			}
			if (needChange) glColorMask(rt.internalWriteMask.rgba[0], rt.internalWriteMask.rgba[1], rt.internalWriteMask.rgba[2], rt.internalWriteMask.rgba[3]);
			blend.isWriteMaskSame = true;
		}
	}

	void Graphics::setRasterizerState(IRasterizerState* state) {
		if (state && state->getGraphics() == this) {
			auto rs = (RasterizerState*)state;
			rs->update();
			auto& rasterizer = _glStatus.rasterizer;
			if (rasterizer.featureValue != rs->getFeatureValue()) {
				auto& internalState = rs->getInternalState();

				if (rasterizer.state.fillMode != internalState.fillMode) {
					glPolygonMode(GL_FRONT_AND_BACK, internalState.fillMode);
					rasterizer.state.fillMode = internalState.fillMode;
				}

				if (internalState.cullEnabled) {
					if (!rasterizer.state.cullEnabled) {
						glEnable(GL_CULL_FACE);
						rasterizer.state.cullEnabled = true;
					}

					if (rasterizer.state.cullMode != internalState.cullMode) {
						glCullFace(internalState.cullMode);
						rasterizer.state.cullMode = internalState.cullMode;
					}
				} else {
					if (rasterizer.state.cullEnabled) {
						glDisable(GL_CULL_FACE);
						rasterizer.state.cullEnabled = false;
					}
				}

				if (rasterizer.state.frontFace != internalState.frontFace) {
					glFrontFace(internalState.frontFace);
					rasterizer.state.frontFace = internalState.frontFace;
				}

				rasterizer.featureValue = rs->getFeatureValue();
			}
		}
	}

	void Graphics::beginRender() {
		wglMakeCurrent(_dc, _rc);

		Vec2i32 size;
		_app->getInnerSize(size);
		glViewport(0, 0, size[0], size[1]);
	}

	void Graphics::draw(const VertexBufferFactory* vertexFactory, IProgram* program, const ShaderParameterFactory* paramFactory, const IIndexBuffer* indexBuffer, uint32_t count, uint32_t offset) {
		if (vertexFactory && indexBuffer && program && program->getGraphics() == this && indexBuffer->getGraphics() == this && count > 0) {
			auto ib = (IndexBuffer*)indexBuffer->getNativeBuffer();
			if (!ib) return;

			auto p = (Program*)program;
			if (p->use(vertexFactory, paramFactory)) {
				ib->draw(count, offset);

				_constantBufferManager.resetUsedShareConstantBuffers();
			}
		}
	}

	void Graphics::endRender() {
		//������ǰ�������ͺ�̨������
		SwapBuffers(_dc);

		//if (wglGetCurrentContext() == _rc) wglMakeCurrent(nullptr, nullptr);
	}

	void Graphics::present() {
	}

	void Graphics::clear(ClearFlag flag, const Vec4f32& color, f32 depth, size_t stencil) {
		GLbitfield mask = 0;
		if ((flag & ClearFlag::COLOR) != ClearFlag::NONE) {
			mask |= GL_COLOR_BUFFER_BIT;
			if (!memEqual<sizeof(Vec4f32)>(_glStatus.clear.color.data, color.data)) {
				glClearColor(color.data[0], color.data[1], color.data[2], color.data[3]);
				memcpy(_glStatus.clear.color.data, color.data, sizeof(Vec4f32));
			}
		}

		if ((flag & ClearFlag::DEPTH) != ClearFlag::NONE) {
			mask |= GL_DEPTH_BUFFER_BIT;
			if (_glStatus.clear.depth != depth) {
				glClearDepth(depth);
				_glStatus.clear.depth = depth;
			}
		}

		if ((flag & ClearFlag::STENCIL) != ClearFlag::NONE) {
			mask |= GL_STENCIL_BUFFER_BIT;
			if (_glStatus.clear.stencil != stencil) {
				glClearStencil(stencil);
				_glStatus.clear.stencil = stencil;
			}
		}

		if (mask) glClear(mask);
	}

	bool Graphics::_glInit() {
		bool initOk = false;

		auto hIns = GetModuleHandle(nullptr);
		std::wstring className = L"OpenGL Temp Window " + String::Utf8ToUnicode(String::toString(Time::now()));

		WNDCLASSEXW wnd;
		memset(&wnd, 0, sizeof(wnd));
		wnd.cbSize = sizeof(wnd);
		wnd.lpfnWndProc = [](HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
			return DefWindowProc(hWnd, msg, wParam, lParam);
		};
		wnd.hInstance = hIns;
		wnd.lpszClassName = className.data();

		RegisterClassExW(&wnd);

		auto hwnd = CreateWindowExW(0, className.data(), L"", 0, 0, 0, 40, 40, nullptr, nullptr, nullptr, nullptr);
		if (hwnd) {
			auto dc = GetDC(hwnd);
			if (dc) {
				PIXELFORMATDESCRIPTOR pfd = { 0 };
				if (SetPixelFormat(dc, 1, &pfd)) {
					auto rc = wglCreateContext(dc);
					if (rc) {
						wglMakeCurrent(dc, rc);
						initOk = glewInit() == GLEW_OK;
						wglMakeCurrent(nullptr, nullptr);
						wglDeleteContext(rc);
					}
				}
				ReleaseDC(hwnd, dc);
			}
			DestroyWindow(hwnd);
		}

		UnregisterClassW(className.data(), hIns);

		return initOk;
	}

	void Graphics::_setInitState() {
		{
			auto& clear = _glStatus.clear;

			GLfloat color[4];
			glGetFloatv(GL_COLOR_CLEAR_VALUE, color);
			clear.color.set(color);

			GLfloat depth;
			glGetFloatv(GL_DEPTH_CLEAR_VALUE, &depth);
			clear.depth = depth;

			GLint stencil;
			glGetIntegerv(GL_STENCIL_CLEAR_VALUE, &stencil);
			clear.stencil = stencil;
		}

		{
			auto& blend = _glStatus.blend;
			if (_deviceFeatures.supportIndependentBlend) {
				blend.enabled = 0;
				for (size_t i = 0; i < MAX_RTS; ++i) {
					blend.enabled |= glIsEnabledi(GL_BLEND, i) << i;

					GLint sc, dc, sa, da;
					glGetIntegeri_v(GL_BLEND_SRC_RGB, i, &sc);
					glGetIntegeri_v(GL_BLEND_DST_RGB, i, &dc);
					glGetIntegeri_v(GL_BLEND_SRC_ALPHA, i, &sa);
					glGetIntegeri_v(GL_BLEND_DST_ALPHA, i, &da);
					auto& bf = blend.func[i];
					bf.srcColor = sc;
					bf.dstColor = dc;
					bf.srcAlpha = sa;
					bf.dstAlpha = da;

					GLint oc, oa;
					glGetIntegeri_v(GL_BLEND_EQUATION_RGB, i, &oc);
					glGetIntegeri_v(GL_BLEND_EQUATION_ALPHA, i, &oa);
					auto& op = blend.op[i];
					op.color = oc;
					op.alpha = oa;

					GLboolean wms[4];
					glGetBooleani_v(GL_COLOR_WRITEMASK, i, wms);
					auto& wm = blend.writeMask[i];
					memcpy(wm.rgba, wms, sizeof(wm.rgba));
				}

				_checkBlendFuncIsSame();
				_checkBlendOpIsSame();
				_checkBlendWriteMaskIsSame();
			} else {
				blend.enabled = glIsEnabled(GL_BLEND) ? 0xFF : 0;

				GLint sc, dc, sa, da;
				glGetIntegerv(GL_BLEND_SRC_RGB, &sc);
				glGetIntegerv(GL_BLEND_DST_RGB, &dc);
				glGetIntegerv(GL_BLEND_SRC_ALPHA, &sa);
				glGetIntegerv(GL_BLEND_DST_ALPHA, &da);
				auto& bf = blend.func[0];
				bf.srcColor = sc;
				bf.dstColor = dc;
				bf.srcAlpha = sa;
				bf.dstAlpha = da;

				GLint oc, oa;
				glGetIntegerv(GL_BLEND_EQUATION_RGB, &oc);
				glGetIntegerv(GL_BLEND_EQUATION_ALPHA, &oa);
				auto& op = blend.op[0];
				op.color = oc;
				op.alpha = oa;

				GLboolean wms[4];
				glGetBooleanv(GL_COLOR_WRITEMASK, wms);
				auto& wm = blend.writeMask[0];
				memcpy(wm.rgba, wms, sizeof(wm.rgba));

				for (size_t i = 1; i < MAX_RTS; ++i) {
					blend.func[i].featureValue = bf.featureValue;
					blend.op[i].featureValue = op.featureValue;
					blend.writeMask[i].featureValue = wm.featureValue;
				}
				blend.isFuncSame = true;
				blend.isOpSame = true;
				blend.isWriteMaskSame = true;
			}

			glGetFloatv(GL_BLEND_COLOR, blend.constantFactors.data);
		}

		{
			auto& rasterizer = _glStatus.rasterizer;

			GLint val = 0;
			glGetIntegerv(GL_POLYGON_MODE, &val);
			rasterizer.state.fillMode = val;

			glGetIntegerv(GL_CULL_FACE_MODE, &val);
			rasterizer.state.cullMode = val;

			glGetIntegerv(GL_FRONT_FACE, &val);
			rasterizer.state.frontFace = val;

			rasterizer.state.cullEnabled = glIsEnabled(GL_CULL_FACE);

			rasterizer.featureValue = calcHash(rasterizer.state);
		}
	}

	void Graphics::_release() {
		if (wglGetCurrentContext() == _rc) wglMakeCurrent(nullptr, nullptr);

		if (_rc) {
			wglDeleteContext(_rc);
			_rc = nullptr;
		}

		if (_dc) {
			ReleaseDC(_app->Win_getHWnd(), _dc);
			_dc = nullptr;
		}

		memset(&_internalFeatures, 0, sizeof(_internalFeatures));
		memset(&_deviceFeatures, 0, sizeof(_deviceFeatures));
		_deviceVersion = "OpenGL Unknown";
		_createBufferMask = Usage::NONE;
	}

	void Graphics::_checkBlendFuncIsSame() {
		auto& blend = _glStatus.blend;
		blend.isFuncSame = true;
		for (size_t i = 1; i < MAX_RTS; ++i) {
			if (blend.func[0].featureValue != blend.func[i].featureValue) {
				blend.isFuncSame = false;
				break;
			}
		}
	}

	void Graphics::_checkBlendOpIsSame() {
		auto& blend = _glStatus.blend;
		blend.isOpSame = true;
		for (size_t i = 1; i < MAX_RTS; ++i) {
			if (blend.op[0].featureValue != blend.op[i].featureValue) {
				blend.isOpSame = false;
				break;
			}
		}
	}

	void Graphics::_checkBlendWriteMaskIsSame() {
		auto& blend = _glStatus.blend;
		blend.isWriteMaskSame = true;
		for (size_t i = 1; i < MAX_RTS; ++i) {
			if (blend.writeMask[0].featureValue != blend.writeMask[i].featureValue) {
				blend.isWriteMaskSame = false;
				break;
			}
		}
	}

	std::optional<Graphics::ConvertFormatResult> Graphics::convertFormat(TextureFormat fmt) {
		switch (fmt) {
		case TextureFormat::R8G8B8:
			return std::make_optional<ConvertFormatResult>(GL_RGB8, GL_RGB, GL_UNSIGNED_BYTE);
		case TextureFormat::R8G8B8A8:
			return std::make_optional<ConvertFormatResult>(GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE);
		default:
			return std::nullopt;
		}
	}

	uint32_t Graphics::getGLTypeSize(GLenum type) {
		switch (type) {
		case GL_BOOL:
		case GL_BYTE:
		case GL_UNSIGNED_BYTE:
			return 1;
		case GL_SHORT:
		case GL_BOOL_VEC2:
			return 2;
		case GL_BOOL_VEC3:
			return 3;
		case GL_FLOAT:
		case GL_INT:
		case GL_UNSIGNED_INT:
		case GL_BOOL_VEC4:
			return 4;
		case GL_DOUBLE:
		case GL_FLOAT_VEC2:
		case GL_INT_VEC2:
		case GL_UNSIGNED_INT_VEC2:
			return 8;
		case GL_FLOAT_VEC3:
		case GL_INT_VEC3:
		case GL_UNSIGNED_INT_VEC3:
			return 12;
		case GL_DOUBLE_VEC2:
		case GL_FLOAT_VEC4:
		case GL_INT_VEC4:
		case GL_UNSIGNED_INT_VEC4:
		case GL_FLOAT_MAT2:
			return 16;
		case GL_DOUBLE_VEC3:
		case GL_FLOAT_MAT2x3:
		case GL_FLOAT_MAT3x2:
			return 24;
		case GL_DOUBLE_VEC4:
		case GL_FLOAT_MAT2x4:
		case GL_FLOAT_MAT4x2:
		case GL_DOUBLE_MAT2:
			return 32;
		case GL_FLOAT_MAT3:
			return 36;
		case GL_FLOAT_MAT3x4:
		case GL_FLOAT_MAT4x3:
		case GL_DOUBLE_MAT2x3:
		case GL_DOUBLE_MAT3x2:
			return 48;
		case GL_FLOAT_MAT4:
		case GL_DOUBLE_MAT2x4:
		case GL_DOUBLE_MAT4x2:
			return 64;
		case GL_DOUBLE_MAT3:
			return 72;
		case GL_DOUBLE_MAT3x4:
		case GL_DOUBLE_MAT4x3:
			return 96;
		case GL_DOUBLE_MAT4:
			return 128;
		default:
			return 0;
		}
	}

	IConstantBuffer* Graphics::_createdShareConstantBuffer() {
		return new ConstantBuffer(*this);
	}

	IConstantBuffer* Graphics::_createdExclusiveConstantBuffer(uint32_t numParameters) {
		auto cb = new ConstantBuffer(*this);
		cb->recordUpdateIds = new uint32_t[numParameters];
		memset(cb->recordUpdateIds, 0, sizeof(uint32_t) * numParameters);
		return cb;
	}

	void Graphics::_debugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
		const char ERR[] = "error";
		const char WARNING[] = "warning";

		auto msgSize = strlen(message);

		if (String::findFirst(message, msgSize, ERR, sizeof(ERR) - 1) != std::string::npos ||
			String::findFirst(message, msgSize, WARNING, sizeof(WARNING) - 1) != std::string::npos) {
			println("gl message : ", message);
		}
	}
}