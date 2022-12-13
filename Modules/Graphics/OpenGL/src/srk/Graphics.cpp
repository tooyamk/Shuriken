#include "Graphics.h"
#include "CreateModule.h"
#include "BlendState.h"
#include "ConstantBuffer.h"
#include "DepthStencil.h"
#include "DepthStencilState.h"
#include "IndexBuffer.h"
#include "PixelBuffer.h"
#include "Program.h"
#include "RasterizerState.h"
#include "RenderTarget.h"
#include "RenderView.h"
#include "RenderViewSimulative.h"
#include "Sampler.h"
#include "Texture1DResource.h"
#include "Texture2DResource.h"
#include "Texture3DResource.h"
#include "TextureView.h"
#include "TextureViewSimulative.h"
#include "VertexBuffer.h"
#include "srk/String.h"
#include "srk/Time.h"

namespace srk::modules::graphics::gl {
	Graphics::Graphics() :
		_bufferCreateUsageMask(Usage::NONE),
		_texCreateUsageMask(Usage::NONE),
#if SRK_OS == SRK_OS_WINDOWS
		_dc(nullptr),
		_rc(nullptr),
#elif SRK_OS == SRK_OS_LINUX
		_context(nullptr),
#endif
		_majorVer(0),
		_minorVer(0),
		_intVer(0),
		_eventDispatcher(new events::EventDispatcher<GraphicsEvent>()) {
		_constantBufferManager.createShareConstantBufferCallback = std::bind(&Graphics::_createdShareConstantBuffer, this);
		_constantBufferManager.createExclusiveConstantBufferCallback = std::bind(&Graphics::_createdExclusiveConstantBuffer, this, std::placeholders::_1);
		memset(&_internalFeatures, 0, sizeof(InternalFeatures));
	}

	Graphics::~Graphics() {
		_release();
	}

	bool Graphics::createDevice(const CreateConfig& conf) {
		using namespace srk::enum_operators;

		if (!conf.win || !conf.win->getNative(windows::WindowNative::WINDOW)) return false;

#if SRK_OS == SRK_OS_WINDOWS
		if (_dc) return false;
#elif SRK_OS == SRK_OS_LINUX
		if (_context || !conf.win->getNative(windows::WindowNative::X_DISPLAY)) return false;
#endif

		/*
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
		*/

		if (!_glInit(conf.win)) {
			_release(conf.win);
			return false;
		}

		auto sampleCount = conf.sampleCount;
		if (sampleCount > _deviceFeatures.maxSampleCount) sampleCount = _deviceFeatures.maxSampleCount;

#if SRK_OS == SRK_OS_WINDOWS
		_dc = GetDC((HWND)conf.win->getNative(windows::WindowNative::WINDOW));
		if (!_dc) {
			_release(conf.win);
			return false;
		}

		if (sampleCount > _deviceFeatures.maxSampleCount) sampleCount = _deviceFeatures.maxSampleCount;

		int32_t attrList[] = {
			WGL_SUPPORT_OPENGL_ARB, 1,
			WGL_DRAW_TO_WINDOW_ARB, 1,
			WGL_ACCELERATION_ARB,   WGL_FULL_ACCELERATION_ARB,
			WGL_COLOR_BITS_ARB,     32,
			WGL_DEPTH_BITS_ARB,     24,
			WGL_STENCIL_BITS_ARB,   8,
			WGL_DOUBLE_BUFFER_ARB,  GL_TRUE,
			WGL_PIXEL_TYPE_ARB,     WGL_TYPE_RGBA_ARB,
			WGL_SAMPLE_BUFFERS_ARB, sampleCount > 1 ? GL_TRUE : GL_FALSE,
			WGL_SAMPLES_ARB,        sampleCount > 1 ? sampleCount : 0,
			//WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,
			0 };

		int32_t nPixelFormat = -1;
		uint32_t nPixCount = 0;

		wglChoosePixelFormatARB(_dc, attrList, nullptr, 1, &nPixelFormat, &nPixCount);
		if (nPixelFormat == -1) {
			_release(conf.win);
			return false;
		}

		PIXELFORMATDESCRIPTOR pfd = { 0 };
		if (!SetPixelFormat(_dc, nPixelFormat, &pfd)) {
			_release(conf.win);
			return false;
		}
		
		GLint attribs[5] = { 0 };
		attribs[0] = WGL_CONTEXT_PROFILE_MASK_ARB;
		attribs[1] = WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB;

		if (conf.debug) {
			attribs[2] = WGL_CONTEXT_FLAGS_ARB;
			attribs[3] = WGL_CONTEXT_DEBUG_BIT_ARB;
		}

		_rc = wglCreateContextAttribsARB(_dc, nullptr, attribs);
		if (!_rc) {
			_release(conf.win);
			return false;
		}

		wglMakeCurrent(_dc, _rc);
#elif SRK_OS == SRK_OS_LINUX
		int32_t attrListDouble[] = {
			GLX_RGBA,       GLX_DOUBLEBUFFER,
			GLX_RED_SIZE,   4,
			GLX_GREEN_SIZE, 4,
			GLX_BLUE_SIZE,  4,
			GLX_DEPTH_SIZE, 16,
			None
		};

		auto dis = (Display*)conf.win->getNative(windows::WindowNative::X_DISPLAY);
		auto vi = glXChooseVisual(dis, DefaultScreen(dis), attrListDouble);//XVisualInfo*
		if (vi) {
			int32_t attrListSingle[] = {
				GLX_RED_SIZE,   4,
				GLX_GREEN_SIZE, 4,
				GLX_BLUE_SIZE,  4,
				GLX_DEPTH_SIZE, 16,
				None
			};
			vi = glXChooseVisual(dis, DefaultScreen(dis), attrListSingle);
			if (!vi) {
				_release(conf.win);
				return false;
			}
		}

		_context = glXCreateContext(dis, vi, nullptr, True);
		XFree(vi);

		glXMakeCurrent(dis, (Window)conf.win->getNative(windows::WindowNative::WINDOW), _context);
#else
		_release(conf.win);
		return false;
#endif

		glGetIntegerv(GL_MAJOR_VERSION, &_majorVer);
		glGetIntegerv(GL_MINOR_VERSION, &_minorVer);

		_intVer = _majorVer * 100 + _minorVer * 10;
		_strVer = String::toString(_intVer);
		_deviceVersion = "OpenGL " + String::toString(_majorVer) + "." + String::toString(_minorVer);

		if (conf.debug) {
			if (isGreatThanOrEqualVersion(4, 3) || glewIsSupported("GL_KHR_debug") || glewIsSupported("GL_ARB_debug_output")) {
				glEnable(GL_DEBUG_OUTPUT);
				glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
				glDebugMessageCallback(&Graphics::_debugCallback, this);
				glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
			}
		}

		_internalFeatures.maxAnisotropy = 1.f;
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &_internalFeatures.maxAnisotropy);

		_deviceFeatures.pixelBuffer = true;
		_deviceFeatures.constantBuffer = isGreatThanOrEqualVersion(3, 1) || glewIsSupported("GL_ARB_uniform_buffer_object");
		_deviceFeatures.sampler = isGreatThanOrEqualVersion(3, 3) || glewIsSupported("GL_ARB_sampler_objects");
		_deviceFeatures.independentBlend = isGreatThanOrEqualVersion(4, 0) ||
			(isGreatThanOrEqualVersion(3, 0) &&
				glewIsSupported("GL_EXT_blend_func_separate") &&
				glewIsSupported("GL_EXT_blend_equation_separate"));
		_internalFeatures.supportTexStorage = isGreatThanOrEqualVersion(4, 2) || glewIsSupported("GL_ARB_texture_storage");
		_deviceFeatures.nativeTextureView = isGreatThanOrEqualVersion(4, 3) || glewIsSupported("ARB_texture_view");
		_deviceFeatures.nativeRenderView = _deviceFeatures.nativeTextureView;
		_deviceFeatures.textureMap = false;
		_deviceFeatures.persistentMap = isGreatThanOrEqualVersion(4, 4) || glewIsSupported("GL_ARB_buffer_storage");
		_deviceFeatures.stencilIndependentRef = true;
		_deviceFeatures.stencilIndependentMask = true;
		_deviceFeatures.vertexDim3Bit8 = false;
		_deviceFeatures.vertexDim3Bit16 = false;

		GLint val = 0;
		glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &val);
		_deviceFeatures.simultaneousRenderTargetCount = val;

		_deviceFeatures.indexTypes.emplace_back(IndexType::UI8);
		_deviceFeatures.indexTypes.emplace_back(IndexType::UI16);
		_deviceFeatures.indexTypes.emplace_back(IndexType::UI32);
		_deviceFeatures.textureFormats.emplace_back(TextureFormat::R8G8B8);
		_deviceFeatures.textureFormats.emplace_back(TextureFormat::R8G8B8A8);

		_bufferCreateUsageMask = Usage::MAP_READ_WRITE | Usage::UPDATE | Usage::RENDERABLE | (_deviceFeatures.persistentMap ? Usage::PERSISTENT_MAP : Usage::NONE);
		_texCreateUsageMask = Usage::UPDATE;

		//glEnable(GL_MULTISAMPLE);
		//glDisable(GL_MULTISAMPLE);

		_defaultBlendState = new BlendState(*this, true);
		_defaultDepthStencilState = new DepthStencilState(*this, true);
		_defaultRasterizerState = new RasterizerState(*this, true);

		_loader = conf.loader;
		_win = conf.win;
		_transpiler = conf.transpiler;

		_setInitState();
		_resize(_win->getContentSize());

		setBlendState(nullptr);
		setDepthStencilState(nullptr);
		setRasterizerState(nullptr);

		return true;
	}

	IntrusivePtr<events::IEventDispatcher<GraphicsEvent>> Graphics::getEventDispatcher() {
		return _eventDispatcher;
	}

	const std::string& Graphics::getVersion() const {
		return _deviceVersion;
	}

	const GraphicsDeviceFeatures& Graphics::getDeviceFeatures() const {
		return _deviceFeatures;
	}

	IntrusivePtr<IBlendState> Graphics::createBlendState() {
		return new BlendState(*this, false);
	}

	IntrusivePtr<IConstantBuffer> Graphics::createConstantBuffer() {
		return _deviceFeatures.constantBuffer ? new ConstantBuffer(*this) : nullptr;
	}

	IntrusivePtr<IDepthStencil> Graphics::createDepthStencil() {
		return new DepthStencil(*this);
	}

	IntrusivePtr<IDepthStencilState> Graphics::createDepthStencilState() {
		return new DepthStencilState(*this, false);
	}

	IntrusivePtr<IIndexBuffer> Graphics::createIndexBuffer() {
		return new IndexBuffer(*this);
	}

	IntrusivePtr<IProgram> Graphics::createProgram() {
		return new Program(*this);
	}

	IntrusivePtr<IRasterizerState> Graphics::createRasterizerState() {
		return new RasterizerState(*this, false);
	}

	IntrusivePtr<IRenderTarget> Graphics::createRenderTarget() {
		return new RenderTarget(*this);
	}

	IntrusivePtr<IRenderView> Graphics::createRenderView() {
		if (_deviceFeatures.nativeRenderView) {
			return new RenderView(*this);
		} else {
			return new RenderViewSimulative(*this);
		}
	}

	IntrusivePtr<ISampler> Graphics::createSampler() {
		return _deviceFeatures.sampler ? new Sampler(*this) : nullptr;
	}

	IntrusivePtr<ITexture1DResource> Graphics::createTexture1DResource() {
		return new Texture1DResource(*this);
	}

	IntrusivePtr<ITexture2DResource> Graphics::createTexture2DResource() {
		return new Texture2DResource(*this);
	}

	IntrusivePtr<ITexture3DResource> Graphics::createTexture3DResource() {
		return new Texture3DResource(*this);
	}

	IntrusivePtr<ITextureView> Graphics::createTextureView() {
		if (_deviceFeatures.nativeTextureView) {
			return new TextureView(*this);
		} else {
			return new TextureViewSimulative(*this);
		}
	}

	IntrusivePtr<IVertexBuffer> Graphics::createVertexBuffer() {
		return new VertexBuffer(*this);
	}

	IntrusivePtr<IPixelBuffer> Graphics::createPixelBuffer() {
		return new PixelBuffer(*this);
	}

	const Vec2ui32& Graphics::getBackBufferSize() const {
		return _glStatus.backSize;
	}

	void Graphics::setBackBufferSize(const Vec2ui32& size) {
		_resize(size);
	}

	Box2i32ui32 Graphics::getViewport() const {
		return _glStatus.viewport;
	}

	void Graphics::setViewport(const Box2i32ui32& vp) {
		if (_win) {
			if (_glStatus.viewport != vp) {
				_glStatus.viewport = vp;

				_updateViewport();
			}
		}
	}

	Box2i32ui32 Graphics::getScissor() const {
		return _glStatus.scissor;
	}

	void Graphics::setScissor(const Box2i32ui32& scissor) {
		if (_win) {
			if (_glStatus.scissor != scissor) {
				_glStatus.scissor = scissor;

				_updateScissor();
			}
		}
	}

	void Graphics::setBlendState(IBlendState* state, uint32_t sampleMask) {
		if (state && state->getGraphics() == this) {
			if (auto bs = state->getNative(); bs) {
				_setBlendState(*(BlendState*)bs, sampleMask);
			} else {
				_setBlendState(*(BlendState*)_defaultBlendState->getNative(), sampleMask);
			}
		} else if (_defaultBlendState) {
			_setBlendState(*(BlendState*)_defaultBlendState->getNative(), sampleMask);
		}
	}

	void Graphics::_setBlendState(BlendState& state, uint32_t sampleMask) {
		auto& blend = _glStatus.blend;

		if (_deviceFeatures.independentBlend) {
			auto count = state.getCount();
			for (size_t i = 0; i < count; ++i) {
				auto& rt = state.getInternalRenderTargetState(i);
				if (bool(blend.enabled >> i & 0x1) != rt.state.enabled) {
					if (rt.state.enabled) {
						glEnablei(GL_BLEND, i);
						blend.enabled |= 1 << i;

						if (blend.func[i].featureValue != rt.internalFunc.featureValue) {
							glBlendFuncSeparatei(i, rt.internalFunc.srcColor, rt.internalFunc.dstColor, rt.internalFunc.srcAlpha, rt.internalFunc.dstAlpha);
							blend.func[i].featureValue = rt.internalFunc.featureValue;
						}

						if (blend.op[i].featureValue != rt.internalOp.featureValue) {
							glBlendEquationSeparatei(i, rt.internalOp.color, rt.internalOp.alpha);
							blend.op[i].featureValue = rt.internalOp.featureValue;
						}
					}
					else {
						glDisablei(GL_BLEND, i);
						blend.enabled &= ~(1 << i);
					}
				}

				if (blend.writeMask[i].featureValue != rt.internalWriteMask.featureValue) {
					glColorMaski(i, rt.internalWriteMask.rgba[0], rt.internalWriteMask.rgba[1], rt.internalWriteMask.rgba[2], rt.internalWriteMask.rgba[3]);
					blend.writeMask[i].featureValue = rt.internalWriteMask.featureValue;
				}
			}
		} else {
			auto& rt = state.getInternalRenderTargetState(0);
			if (rt.state.enabled) {
				if (blend.enabled != 0xFF) {
					glEnable(GL_BLEND);
					blend.enabled = 0xFF;
				}

				if (blend.func[0].featureValue != rt.internalFunc.featureValue) {
					glBlendFuncSeparate(rt.internalFunc.srcColor, rt.internalFunc.dstColor, rt.internalFunc.srcAlpha, rt.internalFunc.dstAlpha);
					blend.func[0].featureValue = rt.internalFunc.featureValue;
				}

				if (blend.op[0].featureValue != rt.internalOp.featureValue) {
					glBlendEquationSeparate(rt.internalOp.color, rt.internalOp.alpha);
					blend.op[0].featureValue = rt.internalOp.featureValue;
				}
			} else {
				if (blend.enabled) {
					glDisable(GL_BLEND);
					blend.enabled = 0;
				}
			}

			if (blend.writeMask[0].featureValue != rt.internalWriteMask.featureValue) {
				glColorMask(rt.internalWriteMask.rgba[0], rt.internalWriteMask.rgba[1], rt.internalWriteMask.rgba[2], rt.internalWriteMask.rgba[3]);
				blend.writeMask[0].featureValue = rt.internalWriteMask.featureValue;
			}
		}

		if (blend.enabled) {
			auto& constants = state.getConstants();
			if (blend.constants != constants) {
				glBlendColor(constants.data[0], constants.data[1], constants.data[2], constants.data[3]);
				blend.constants = constants;
			}
		}
	}

	template<bool SupportIndependentBlend>
	void SRK_CALL Graphics::_setDependentBlendState(const InternalRenderTargetBlendState& rt) {
		auto& blend = _glStatus.blend;
		if (rt.state.enabled) {
			if (blend.enabled != 0xFF) {
				glEnable(GL_BLEND);
				blend.enabled = 0xFF;
			}

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
		} else {
			if (blend.enabled) {
				glDisable(GL_BLEND);
				blend.enabled = 0;
			}
		}
	}

	void Graphics::_updateCanvasSize(const Vec2ui32& size) {
		if (_glStatus.canvasSize != size) {
			_glStatus.canvasSize = size;
			_updateViewport();
			_updateScissor();
		}
	}

	void Graphics::_updateViewport() {
		auto& vp = _glStatus.viewport;
		glViewport(vp.pos[0], (int32_t)_glStatus.canvasSize[1] - (vp.pos[1] + (int32_t)vp.size[1]), vp.size[0], vp.size[1]);
	}

	void Graphics::_updateScissor() {
		auto& rect = _glStatus.scissor;
		glScissor(rect.pos[0], (int32_t)_glStatus.canvasSize[1] - (rect.pos[1] + (int32_t)rect.size[1]), rect.size[0], rect.size[1]);
	}

	void Graphics::setDepthStencilState(IDepthStencilState* state) {
		if (state && state->getGraphics() == this) {
			if (auto native = state->getNative(); native) {
				_setDepthStencilState(*(DepthStencilState*)native);
			} else {
				_setDepthStencilState(*(DepthStencilState*)_defaultDepthStencilState->getNative());
			}
		} else if (_defaultBlendState) {
			_setDepthStencilState(*(DepthStencilState*)_defaultDepthStencilState->getNative());
		}
	}

	void Graphics::_setDepthStencilState(DepthStencilState& state) {
		state.update();

		{
			auto& depth = _glStatus.depth;
			auto& ids = state.getInternalDepthState();
			auto changed = false;
			if (depth.enabled == ids.enabled) {
				if (ids.enabled && depth.featureValue != ids.featureValue) changed = true;
			} else {
				if (ids.enabled) {
					glEnable(GL_DEPTH_TEST);
					depth.enabled = true;

					if (depth.featureValue != ids.featureValue) changed = true;
				} else {
					glDisable(GL_DEPTH_TEST);
					depth.enabled = false;
				}
			}
			if (changed) {
				if (depth.writeable != ids.writeable) {
					glDepthMask(ids.writeable);
					depth.writeable = ids.writeable;
				}

				if (depth.func != ids.func) {
					glDepthFunc(ids.func);
					depth.func = ids.func;
				}
			}
		}

		{
			auto& stencil = _glStatus.stencil;
			auto& iss = state.getInternalStencilState();
			auto changed = false;
			if (stencil.state.enabled == iss.enabled) {
				if (iss.enabled && stencil.stencilFeatureValue != state.getStencilFeatureValue()) changed = true;
			} else {
				if (iss.enabled) {
					glEnable(GL_STENCIL_TEST);

					if (stencil.stencilFeatureValue != state.getStencilFeatureValue()) changed = true;
				} else {
					glDisable(GL_STENCIL_TEST);
				}
				stencil.state.enabled = iss.enabled;
			}

			if (changed) {
				auto& frontFace = stencil.state.face.front;
				auto& issFrontFace = iss.face.front;
				if (frontFace.func != issFrontFace.func || frontFace.ref != issFrontFace.ref || frontFace.mask.read != issFrontFace.mask.read) glStencilFuncSeparate(GL_FRONT, issFrontFace.func, issFrontFace.ref, issFrontFace.mask.read);

				auto& backFace = stencil.state.face.back;
				auto& issBackFace = iss.face.back;
				if (backFace.func != issBackFace.func || backFace.ref != issBackFace.ref || backFace.mask.read != issBackFace.mask.read) glStencilFuncSeparate(GL_BACK, issBackFace.func, issBackFace.ref, issBackFace.mask.read);

				if (frontFace.mask.write != issFrontFace.mask.write) glStencilMaskSeparate(GL_FRONT, issFrontFace.mask.write);
				if (backFace.mask.write != issBackFace.mask.write) glStencilMaskSeparate(GL_BACK, issBackFace.mask.write);

				if (frontFace.op.featureValue != issFrontFace.op.featureValue) glStencilOpSeparate(GL_FRONT, issFrontFace.op.fail, issFrontFace.op.depthFail, issFrontFace.op.pass);

				if (backFace.op.featureValue != issBackFace.op.featureValue) glStencilOpSeparate(GL_BACK, issBackFace.op.fail, issBackFace.op.depthFail, issBackFace.op.pass);

				stencil.state.face = iss.face;
				stencil.stencilFeatureValue = state.getStencilFeatureValue();
			}
		}
	}

	void Graphics::setRasterizerState(IRasterizerState* state) {
		if (state && state->getGraphics() == this) {
			if (auto native = state->getNative(); native) {
				_setRasterizerState(*(RasterizerState*)native);
			} else {
				_setRasterizerState(*(RasterizerState*)_defaultRasterizerState->getNative());
			}
		} else if (_defaultRasterizerState) {
			_setRasterizerState(*(RasterizerState*)_defaultRasterizerState->getNative());
		}
	}

	void Graphics::_setRasterizerState(RasterizerState& state) {
		state.update();
		auto& rasterizer = _glStatus.rasterizer;
		if (rasterizer.featureValue.trySet(state.getFeatureValue())) {
			auto& internalState = state.getInternalState();

			if (rasterizer.state.fillMode != internalState.fillMode) {
				glPolygonMode(GL_FRONT_AND_BACK, internalState.fillMode);
				rasterizer.state.fillMode = internalState.fillMode;
			}

			if (rasterizer.state.scissorEnabled != internalState.scissorEnabled) {
				if (internalState.scissorEnabled) {
					glEnable(GL_SCISSOR_TEST);
				} else {
					glDisable(GL_SCISSOR_TEST);
				}
				rasterizer.state.scissorEnabled = internalState.scissorEnabled;
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

			rasterizer.featureValue = state.getFeatureValue();
		}
	}

	void Graphics::beginRender() {
#if SRK_OS == SRK_OS_WINDOWS
		wglMakeCurrent(_dc, _rc);
#elif SRK_OS == SRK_OS_LINUX
		glXMakeCurrent((Display*)_win->getNative(windows::WindowNative::X_DISPLAY), (Window)_win->getNative(windows::WindowNative::WINDOW), _context);
#endif
	}

	void Graphics::draw(const IVertexAttributeGetter* vertexAttributeGetter, IProgram* program, const IShaderParameterGetter* shaderParamGetter, const IIndexBuffer* indexBuffer, uint32_t count, uint32_t offset) {
		if (!vertexAttributeGetter || !indexBuffer || !program || program->getGraphics() != this || indexBuffer->getGraphics() != this || !count) return;

		auto ib = (const IndexBuffer*)indexBuffer->getNative();
		if (!ib) return;

		auto p = (Program*)program->getNative();
		if (!p || !p->use(vertexAttributeGetter, shaderParamGetter)) return;

		ib->draw(count, offset);

		_constantBufferManager.resetUsedShareConstantBuffers();
	}

	void Graphics::drawInstanced(const IVertexAttributeGetter* vertexAttributeGetter, IProgram* program, const IShaderParameterGetter* shaderParamGetter, const IIndexBuffer* indexBuffer, uint32_t instancedCount, uint32_t count, uint32_t offset) {
		if (!vertexAttributeGetter || !indexBuffer || !program || program->getGraphics() != this || indexBuffer->getGraphics() != this || !count) return;

		auto ib = (const IndexBuffer*)indexBuffer->getNative();
		if (!ib) return;

		auto p = (Program*)program->getNative();
		if (!p || !p->use(vertexAttributeGetter, shaderParamGetter)) return;

		ib->drawInstanced(instancedCount, count, offset);
	}

	void Graphics::endRender() {
		//交换当前缓冲区和后台缓冲区
		//SwapBuffers(_dc);

		//if (wglGetCurrentContext() == _rc) wglMakeCurrent(nullptr, nullptr);
	}

	void Graphics::flush() {
		glFlush();
	}

	void Graphics::present() {
#if SRK_OS == SRK_OS_WINDOWS
		SwapBuffers(_dc);
#elif SRK_OS == SRK_OS_LINUX
		glXSwapBuffers((Display*)_win->getNative(windows::WindowNative::X_DISPLAY), (Window)_win->getNative(windows::WindowNative::WINDOW));
#endif
		
	}

	void Graphics::setRenderTarget(IRenderTarget* rt) {
		if (rt) {
			if (auto native = (RenderTarget*)rt->getNative(); native) {
				native->update();
				glBindFramebuffer(GL_FRAMEBUFFER, native->getInternalBuffer());
				_glStatus.isBack = false;
				_updateCanvasSize(rt->getSize());
			} else {
				glBindFramebuffer(GL_FRAMEBUFFER, 0);
				_glStatus.isBack = true;
				_updateCanvasSize(_glStatus.backSize);
			}
		} else {
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			_glStatus.isBack = true;
			_updateCanvasSize(_glStatus.backSize);
		}
	}

	void Graphics::clear(ClearFlag flags, const Vec4f32& color, float32_t depth, size_t stencil) {
		using namespace srk::enum_operators;

		GLbitfield mask = 0;
		if ((flags & ClearFlag::COLOR) != ClearFlag::NONE) {
			mask |= GL_COLOR_BUFFER_BIT;
			if (_glStatus.clear.color != color) {
				glClearColor(color.data[0], color.data[1], color.data[2], color.data[3]);
				memcpy(_glStatus.clear.color.data, color.data, sizeof(Vec4f32));
			}
		}

		auto tmpDepthWrite = false;
		if ((flags & ClearFlag::DEPTH) != ClearFlag::NONE) {
			mask |= GL_DEPTH_BUFFER_BIT;
			tmpDepthWrite = !_glStatus.depth.writeable;
			if (_glStatus.clear.depth != depth) {
				glClearDepth(depth);
				_glStatus.clear.depth = depth;
			}
		}

		if ((flags & ClearFlag::STENCIL) != ClearFlag::NONE) {
			mask |= GL_STENCIL_BUFFER_BIT;
			if (_glStatus.clear.stencil != stencil) {
				glClearStencil(stencil);
				_glStatus.clear.stencil = stencil;
			}
		}

		if (mask) {
			if (tmpDepthWrite) {
				glDepthMask(GL_TRUE);
				glClear(mask);
				glDepthMask(GL_FALSE);
			} else {
				glClear(mask);
			}
		}
	}

	bool Graphics::_glInit(windows::IWindow* win) {
		auto initOk = false;

#if SRK_OS == SRK_OS_WINDOWS
		auto hIns = GetModuleHandleW(nullptr);
		auto className = L"Shuriken OpenGL Temp Window";

		WNDCLASSEXW wnd;
		memset(&wnd, 0, sizeof(wnd));
		wnd.cbSize = sizeof(wnd);
		wnd.lpfnWndProc = [](HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
			return DefWindowProcW(hWnd, msg, wParam, lParam);
		};
		wnd.hInstance = hIns;
		wnd.lpszClassName = className;

		RegisterClassExW(&wnd);

		if (auto hwnd = CreateWindowExW(0, className, L"", 0, 0, 0, 40, 40, nullptr, nullptr, nullptr, nullptr); hwnd) {
			if (auto dc = GetDC(hwnd); dc) {
				PIXELFORMATDESCRIPTOR pfd = { 0 };
				if (SetPixelFormat(dc, 1, &pfd)) {
					if (auto rc = wglCreateContext(dc); rc) {
						wglMakeCurrent(dc, rc);

						initOk = _glewInit();

						wglMakeCurrent(nullptr, nullptr);
						wglDeleteContext(rc);
					}
				}
				ReleaseDC(hwnd, dc);
			}
			DestroyWindow(hwnd);
		}

		UnregisterClassW(className, hIns);
#elif SRK_OS == SRK_OS_LINUX
		auto dis = XOpenDisplay(nullptr);
		auto screen = DefaultScreen(dis);
		if (auto wnd = XCreateSimpleWindow(dis, RootWindow(dis, screen), 0, 0, 100, 100, 0, 0, 0); wnd) {
			int32_t attrList[] = {
				GLX_RED_SIZE,   4,
				GLX_GREEN_SIZE, 4,
				GLX_BLUE_SIZE,  4,
				GLX_DEPTH_SIZE, 16,
				None
			};

			if (auto vi = glXChooseVisual(dis, screen, attrList); vi) {
				auto context = glXCreateContext(dis, vi, nullptr, True);
				XFree(vi);

				glXMakeCurrent(dis, wnd, context);

				initOk = _glewInit();

				glXMakeCurrent(dis, None, nullptr);
				glXDestroyContext(dis, context);
			}

			XDestroyWindow(dis, wnd);
		}
		XCloseDisplay(dis);
#endif

		return initOk;
	}

	bool Graphics::_glewInit() {
		if (glewInit() == GLEW_OK) {
			GLint val;
			glGetIntegerv(GL_MAX_SAMPLES, &val);
			_deviceFeatures.maxSampleCount = val;

			return true;
		}

		return false;
	}

	void Graphics::_setInitState() {
		{
			_glStatus.isBack = true;
			_glStatus.backSize = _win->getContentSize();
			_glStatus.canvasSize = _glStatus.backSize;

			GLint rect[4];
			glGetIntegerv(GL_VIEWPORT, rect);

			_glStatus.viewport.pos.set(rect[0], (int32_t)_glStatus.canvasSize[1] - (rect[1] + rect[3]));
			_glStatus.viewport.size.set(rect[2], rect[3]);

			glGetIntegerv(GL_SCISSOR_BOX, rect);
			_glStatus.scissor.pos.set(rect[0], (int32_t)_glStatus.canvasSize[1] - (rect[1] + rect[3]));
			_glStatus.scissor.size.set(rect[2], rect[3]);
		}
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
			blend.func.resize(_deviceFeatures.simultaneousRenderTargetCount);
			blend.op.resize(_deviceFeatures.simultaneousRenderTargetCount);
			blend.writeMask.resize(_deviceFeatures.simultaneousRenderTargetCount);
			if (_deviceFeatures.independentBlend) {
				blend.enabled = 0;
				for (size_t i = 0; i < _deviceFeatures.simultaneousRenderTargetCount; ++i) {
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

				for (size_t i = 1; i < _deviceFeatures.simultaneousRenderTargetCount; ++i) {
					blend.func[i].featureValue = bf.featureValue;
					blend.op[i].featureValue = op.featureValue;
					blend.writeMask[i].featureValue = wm.featureValue;
				}
			}

			glGetFloatv(GL_BLEND_COLOR, blend.constants.data);
		}

		{
			auto& depth = _glStatus.depth;

			depth.enabled = glIsEnabled(GL_DEPTH_TEST);

			GLboolean b;
			glGetBooleanv(GL_DEPTH_WRITEMASK, &b);
			depth.writeable = b;

			GLint val;
			glGetIntegerv(GL_DEPTH_FUNC, &val);
			depth.func = val;
		}

		{
			auto& stencil = _glStatus.stencil;

			stencil.state.enabled = glIsEnabled(GL_STENCIL_TEST);

			GLint val[2];
			glGetIntegerv(GL_STENCIL_VALUE_MASK, &val[0]);
			glGetIntegerv(GL_STENCIL_BACK_VALUE_MASK, &val[1]);
			stencil.state.face.front.mask.read = val[0];
			stencil.state.face.back.mask.read = val[1];

			glGetIntegerv(GL_STENCIL_REF, &val[0]);
			glGetIntegerv(GL_STENCIL_BACK_REF, &val[1]);
			stencil.state.face.front.ref = val[0];
			stencil.state.face.back.ref = val[1];

			glGetIntegerv(GL_STENCIL_WRITEMASK, &val[0]);
			glGetIntegerv(GL_STENCIL_BACK_WRITEMASK, &val[1]);
			stencil.state.face.front.mask.write = val[0];
			stencil.state.face.back.mask.write = val[1];

			glGetIntegerv(GL_STENCIL_FUNC, &val[0]);
			glGetIntegerv(GL_STENCIL_BACK_FUNC, &val[1]);
			stencil.state.face.front.func = val[0];
			stencil.state.face.back.func = val[1];

			glGetIntegerv(GL_STENCIL_FAIL, &val[0]);
			glGetIntegerv(GL_STENCIL_BACK_FAIL, &val[1]);
			stencil.state.face.front.op.fail = val[0];
			stencil.state.face.back.op.fail = val[1];

			glGetIntegerv(GL_STENCIL_PASS_DEPTH_FAIL, &val[0]);
			glGetIntegerv(GL_STENCIL_BACK_PASS_DEPTH_FAIL, &val[1]);
			stencil.state.face.front.op.depthFail = val[0];
			stencil.state.face.back.op.depthFail = val[1];

			glGetIntegerv(GL_STENCIL_PASS_DEPTH_PASS, &val[0]);
			glGetIntegerv(GL_STENCIL_BACK_PASS_DEPTH_PASS, &val[1]);
			stencil.state.face.front.op.pass = val[0];
			stencil.state.face.back.op.pass = val[1];

			StencilState ss;
			ss.enabled = stencil.state.enabled;
			if (ss.enabled) {
				auto& dst = ss.face.front;
				auto& src = stencil.state.face.front;
				dst.func = convertComparisonFunc(src.func);
				dst.mask.read = src.mask.read;
				dst.mask.write = src.mask.write;
				dst.op.depthFail = convertStencilOp(src.op.depthFail);
				dst.op.fail = convertStencilOp(src.op.fail);
				dst.op.pass = convertStencilOp(src.op.pass);
				dst.ref = src.ref;

				dst = ss.face.back;
				src = stencil.state.face.back;
				dst.func = convertComparisonFunc(src.func);
				dst.mask.read = src.mask.read;
				dst.mask.write = src.mask.write;
				dst.op.depthFail = convertStencilOp(src.op.depthFail);
				dst.op.fail = convertStencilOp(src.op.fail);
				dst.op.pass = convertStencilOp(src.op.pass);
				dst.ref = src.ref;
			}
			stencil.stencilFeatureValue.set(ss);
		}

		{
			auto& rasterizer = _glStatus.rasterizer;

			GLint val[2];
			glGetIntegerv(GL_POLYGON_MODE, val);
			rasterizer.state.fillMode = val[0] == val[1] ? val[0] : 0;

			glGetIntegerv(GL_CULL_FACE_MODE, val);
			rasterizer.state.cullMode = val[0];

			glGetIntegerv(GL_FRONT_FACE, val);
			rasterizer.state.frontFace = val[0];

			rasterizer.state.cullEnabled = glIsEnabled(GL_CULL_FACE);
			rasterizer.state.scissorEnabled = glIsEnabled(GL_SCISSOR_TEST);

			RasterizerDescriptor desc;
			desc.cullMode = convertCullMode(rasterizer.state.cullMode);
			desc.fillMode = convertFillMode(rasterizer.state.fillMode);
			desc.frontFace = convertFrontFace(rasterizer.state.frontFace);
			desc.scissorEnabled = rasterizer.state.scissorEnabled;

			rasterizer.featureValue.set(desc);
		}
	}

	void Graphics::_release(windows::IWindow* win) {
		if (!win) win = _win;

#if SRK_OS == SRK_OS_WINDOWS
		if (wglGetCurrentContext() == _rc) wglMakeCurrent(nullptr, nullptr);

		if (_rc) {
			wglDeleteContext(_rc);
			_rc = nullptr;
		}

		if (_dc) {
			ReleaseDC((HWND)win->getNative(windows::WindowNative::WINDOW), _dc);
			_dc = nullptr;
		}
#elif SRK_OS == SRK_OS_LINUX
		if (glXGetCurrentContext() == _context) glXMakeCurrent((Display*)win->getNative(windows::WindowNative::X_DISPLAY), None, nullptr);

		if (_context) {
			glXDestroyContext((Display*)win->getNative(windows::WindowNative::X_DISPLAY), _context);
			_context = nullptr;
		}
#endif

		memset(&_internalFeatures, 0, sizeof(_internalFeatures));
		_deviceFeatures.reset();
		_deviceVersion = "OpenGL Unknown";
		_bufferCreateUsageMask = Usage::NONE;
	}

	void Graphics::_resize(const Vec2ui32& size) {
		_glStatus.backSize = size;
		if (_glStatus.isBack) _updateCanvasSize(_glStatus.backSize);
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

	GLenum Graphics::convertComparisonFunc(ComparisonFunc func) {
		switch (func) {
		case ComparisonFunc::NEVER:
			return GL_NEVER;
		case ComparisonFunc::LESS:
			return GL_LESS;
		case ComparisonFunc::EQUAL:
			return GL_EQUAL;
		case ComparisonFunc::LESS_EQUAL:
			return GL_LEQUAL;
		case ComparisonFunc::GREATER:
			return GL_GREATER;
		case ComparisonFunc::NOT_EQUAL:
			return GL_NOTEQUAL;
		case ComparisonFunc::GREATER_EQUAL:
			return GL_GEQUAL;
		case ComparisonFunc::ALWAYS:
			return GL_ALWAYS;
		default:
			return GL_NEVER;
		}
	}

	ComparisonFunc Graphics::convertComparisonFunc(GLenum func) {
		switch (func) {
		case GL_NEVER:
			return ComparisonFunc::NEVER;
		case GL_LESS:
			return ComparisonFunc::LESS;
		case GL_EQUAL:
			return ComparisonFunc::EQUAL;
		case GL_LEQUAL:
			return ComparisonFunc::LESS_EQUAL;
		case GL_GREATER:
			return ComparisonFunc::GREATER;
		case GL_NOTEQUAL:
			return ComparisonFunc::NOT_EQUAL;
		case GL_GEQUAL:
			return ComparisonFunc::GREATER_EQUAL;
		case GL_ALWAYS:
			return ComparisonFunc::ALWAYS;
		default:
			return ComparisonFunc::NEVER;
		}
	}

	GLenum Graphics::convertStencilOp(StencilOp op) {
		switch (op) {
		case StencilOp::KEEP:
			return GL_KEEP;
		case StencilOp::ZERO:
			return GL_ZERO;
		case StencilOp::REPLACE:
			return GL_REPLACE;
		case StencilOp::INCR_CLAMP:
			return GL_INCR;
		case StencilOp::DECR_CLAMP:
			return GL_DECR;
		case StencilOp::INCR_WRAP:
			return GL_INCR_WRAP;
		case StencilOp::DECR_WRAP:
			return GL_DECR_WRAP;
		case StencilOp::INVERT:
			return GL_INVERT;
		default:
			return GL_KEEP;
		}
	}

	StencilOp Graphics::convertStencilOp(GLenum op) {
		switch (op) {
		case GL_KEEP:
			return StencilOp::KEEP;
		case GL_ZERO:
			return StencilOp::ZERO;
		case GL_REPLACE:
			return StencilOp::REPLACE;
		case GL_INCR:
			return StencilOp::INCR_CLAMP;
		case GL_DECR:
			return StencilOp::DECR_CLAMP;
		case GL_INCR_WRAP:
			return StencilOp::INCR_WRAP;
		case GL_DECR_WRAP:
			return StencilOp::DECR_WRAP;
		case GL_INVERT:
			return StencilOp::INVERT;
		default:
			return StencilOp::KEEP;
		}
	}

	uint16_t Graphics::convertBlendFactor(BlendFactor factor) {
		switch (factor) {
		case BlendFactor::ZERO:
			return GL_ZERO;
		case BlendFactor::ONE:
			return GL_ONE;
		case BlendFactor::SRC_COLOR:
			return GL_SRC_COLOR;
		case BlendFactor::ONE_MINUS_SRC_COLOR:
			return GL_ONE_MINUS_SRC_COLOR;
		case BlendFactor::SRC_ALPHA:
			return GL_SRC_ALPHA;
		case BlendFactor::ONE_MINUS_SRC_ALPHA:
			return GL_ONE_MINUS_SRC_ALPHA;
		case BlendFactor::DST_COLOR:
			return GL_DST_COLOR;
		case BlendFactor::ONE_MINUS_DST_COLOR:
			return GL_ONE_MINUS_DST_COLOR;
		case BlendFactor::DST_ALPHA:
			return GL_DST_ALPHA;
		case BlendFactor::ONE_MINUS_DST_ALPHA:
			return GL_ONE_MINUS_DST_ALPHA;
		case BlendFactor::SRC_ALPHA_SATURATE:
			return GL_SRC_ALPHA_SATURATE;
		case BlendFactor::CONSTANT_COLOR:
			return GL_CONSTANT_COLOR;
		case BlendFactor::ONE_MINUS_CONSTANT_COLOR:
			return GL_ONE_MINUS_CONSTANT_COLOR;
		case BlendFactor::SRC1_COLOR:
			return GL_SRC1_COLOR;
		case BlendFactor::ONE_MINUS_SRC1_COLOR:
			return GL_ONE_MINUS_SRC1_COLOR;
		case BlendFactor::SRC1_ALPHA:
			return GL_SRC1_ALPHA;
		case BlendFactor::ONE_MINUS_SRC1_ALPHA:
			return GL_ONE_MINUS_SRC1_ALPHA;
		default:
			return GL_ZERO;
		}
	}

	uint16_t Graphics::convertBlendOp(BlendOp op) {
		switch (op) {
		case BlendOp::ADD:
			return GL_FUNC_ADD;
		case BlendOp::SUBTRACT:
			return GL_FUNC_SUBTRACT;
		case BlendOp::REV_SUBTRACT:
			return GL_FUNC_REVERSE_SUBTRACT;
		case BlendOp::MIN:
			return GL_MIN;
		case BlendOp::MAX:
			return GL_MAX;
		default:
			return GL_FUNC_ADD;
		}
	}

	GLenum Graphics::convertFillMode(FillMode mode) {
		switch (mode) {
		case FillMode::WIREFRAME:
			return GL_LINE;
		case FillMode::SOLID:
			return GL_FILL;
		default:
			return GL_FILL;
		}
	}

	FillMode Graphics::convertFillMode(GLenum mode) {
		switch (mode) {
		case GL_LINE:
			return FillMode::WIREFRAME;
		case GL_FILL:
			return FillMode::SOLID;
		default:
			return FillMode::SOLID;
		}
	}

	GLenum Graphics::convertCullMode(CullMode mode) {
		switch (mode) {
		case CullMode::FRONT:
			return GL_FRONT;
		case CullMode::BACK:
			return GL_BACK;
		default:
			return GL_BACK;
		}
	}

	CullMode Graphics::convertCullMode(GLenum mode) {
		switch (mode) {
		case GL_FRONT:
			return CullMode::FRONT;
		case GL_BACK:
			return CullMode::BACK;
		default:
			return CullMode::BACK;
		}
	}

	GLenum Graphics::convertFrontFace(FrontFace front) {
		switch (front) {
		case FrontFace::CW:
			return GL_CW;
		case FrontFace::CCW:
			return GL_CCW;
		default:
			return GL_CW;
		}
	}

	FrontFace Graphics::convertFrontFace(GLenum front) {
		switch (front) {
		case GL_CW:
			return FrontFace::CW;
		case GL_CCW:
			return FrontFace::CCW;
		default:
			return FrontFace::CW;
		}
	}

	GLenum Graphics::convertProgramStage(ProgramStage stage) {
		switch (stage) {
		case ProgramStage::GS:
			return GL_GEOMETRY_SHADER;
		case ProgramStage::PS:
			return GL_FRAGMENT_SHADER;
		case ProgramStage::VS:
			return GL_VERTEX_SHADER;
		default:
			return 0;
		}
	}

	GLenum Graphics::convertVertexFormat(VertexType type) {
		switch (type) {
		case VertexType::I8:
			return GL_BYTE;
		case VertexType::UI8:
			return GL_UNSIGNED_BYTE;
		case VertexType::I16:
			return GL_SHORT;
		case VertexType::UI16:
			return GL_UNSIGNED_SHORT;
		case VertexType::I32:
			return GL_INT;
		case VertexType::UI32:
			return GL_UNSIGNED_INT;
		case VertexType::F32:
			return GL_FLOAT;
		default:
			return 0;
		}
	}

	uint32_t Graphics::getGLTypeSize(GLenum type) {
		switch (type) {
		case GL_BOOL:
		case GL_BYTE:
		case GL_UNSIGNED_BYTE:
			return 1;
		case GL_SHORT:
		case GL_UNSIGNED_SHORT:
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
		std::string_view mv(message);

		if (String::find(mv, "error") != std::string::npos ||
			String::find(mv, "warning") != std::string::npos ||
			String::find(mv, "failed") != std::string::npos) {
			((Graphics*)userParam)->error(std::string("OpenGL sys message : ") + message);
		}
	}
}