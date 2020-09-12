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
#include "aurora/Application.h"
#include "aurora/String.h"
#include "aurora/Time.h"
#include "GL/wglew.h"

namespace aurora::modules::graphics::win_gl {
	Graphics::Graphics() :
		_bufferCreateUsageMask(Usage::NONE),
		_texCreateUsageMask(Usage::NONE),
		_dc(nullptr),
		_rc(nullptr),
		_majorVer(0),
		_minorVer(0),
		_intVer(0),
		_resizedListener(&Graphics::_resizedHandler, this) {
		_resizedListener.ref();
		_constantBufferManager.createShareConstantBufferCallback = std::bind(&Graphics::_createdShareConstantBuffer, this);
		_constantBufferManager.createExclusiveConstantBufferCallback = std::bind(&Graphics::_createdExclusiveConstantBuffer, this, std::placeholders::_1);
		memset(&_internalFeatures, 0, sizeof(InternalFeatures));
	}

	Graphics::~Graphics() {
		if (_app) _app->getEventDispatcher().removeEventListener(ApplicationEvent::RESIZED, _resizedListener);
		_release();
	}

	bool Graphics::createDevice(Ref* loader, Application* app, IProgramSourceTranslator* trans, const GraphicsAdapter* adapter, SampleCount sampleCount) {
		if (_dc || !app->Win_getHWnd()) return false;

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

		if (!_glInit()) {
			_release();
			return false;
		}

		_dc = GetDC(app->Win_getHWnd());
		if (!_dc) {
			_release();
			return false;
		}

		if (sampleCount > _deviceFeatures.maxSampleCount) sampleCount = _deviceFeatures.maxSampleCount;

		int32_t iAttribIList[] = {
			WGL_SUPPORT_OPENGL_ARB, 1,
			WGL_DRAW_TO_WINDOW_ARB, 1,
			WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
			WGL_COLOR_BITS_ARB, 32,
			WGL_DEPTH_BITS_ARB, 24,
			WGL_STENCIL_BITS_ARB, 8,
			WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
			WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
			WGL_SAMPLE_BUFFERS_ARB, sampleCount > 1 ? GL_TRUE : GL_FALSE,
			WGL_SAMPLES_ARB, sampleCount > 1 ? sampleCount : 0,
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
			//WGL_CONTEXT_MINOR_VERSION_ARB, 1,
			WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,//兼容模式环境
			//WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_ES_PROFILE_BIT_EXT,
			//WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,//核心功能环境
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

		_intVer = _majorVer * 100 + _minorVer * 10;
		_strVer = String::toString(_intVer);
		_deviceVersion = "OpenGL " + String::toString(_majorVer) + "." + String::toString(_minorVer);

#ifdef AE_DEBUG
		if (isGreatThanOrEqualVersion(4, 3) || glewIsSupported("GL_KHR_debug") || glewIsSupported("GL_ARB_debug_output")) {
			glEnable(GL_DEBUG_OUTPUT);
			glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
			glDebugMessageCallback(&Graphics::_debugCallback, this);
			glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
		}
#endif

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

		_loader = loader;
		_app = app;
		_trans = trans;
		_app->getEventDispatcher().addEventListener(ApplicationEvent::RESIZED, _resizedListener);

		_setInitState();
		_resize(_app->getClientSize());

		return true;
	}

	events::IEventDispatcher<GraphicsEvent>& Graphics::getEventDispatcher() {
		return _eventDispatcher;
	}

	const events::IEventDispatcher<GraphicsEvent>& Graphics::getEventDispatcher() const {
		return _eventDispatcher;
	}

	const std::string& Graphics::getVersion() const {
		return _deviceVersion;
	}

	const GraphicsDeviceFeatures& Graphics::getDeviceFeatures() const {
		return _deviceFeatures;
	}

	IBlendState* Graphics::createBlendState() {
		return new BlendState(*this, false);
	}

	IConstantBuffer* Graphics::createConstantBuffer() {
		return _deviceFeatures.constantBuffer ? new ConstantBuffer(*this) : nullptr;
	}

	IDepthStencil* Graphics::createDepthStencil() {
		return new DepthStencil(*this);
	}

	IDepthStencilState* Graphics::createDepthStencilState() {
		return new DepthStencilState(*this, false);
	}

	IIndexBuffer* Graphics::createIndexBuffer() {
		return new IndexBuffer(*this);
	}

	IProgram* Graphics::createProgram() {
		return new Program(*this);
	}

	IRasterizerState* Graphics::createRasterizerState() {
		return new RasterizerState(*this, false);
	}

	IRenderTarget* Graphics::createRenderTarget() {
		return new RenderTarget(*this);
	}

	IRenderView* Graphics::createRenderView() {
		if (_deviceFeatures.nativeRenderView) {
			return new RenderView(*this);
		} else {
			return new RenderViewSimulative(*this);
		}
	}

	ISampler* Graphics::createSampler() {
		return _deviceFeatures.sampler ? new Sampler(*this) : nullptr;
	}

	ITexture1DResource* Graphics::createTexture1DResource() {
		return new Texture1DResource(*this);
	}

	ITexture2DResource* Graphics::createTexture2DResource() {
		return new Texture2DResource(*this);
	}

	ITexture3DResource* Graphics::createTexture3DResource() {
		return new Texture3DResource(*this);
	}

	ITextureView* Graphics::createTextureView() {
		if (_deviceFeatures.nativeTextureView) {
			return new TextureView(*this);
		} else {
			return new TextureViewSimulative(*this);
		}
	}

	IVertexBuffer* Graphics::createVertexBuffer() {
		return new VertexBuffer(*this);
	}

	IPixelBuffer* Graphics::createPixelBuffer() {
		return new PixelBuffer(*this);
	}

	Box2i32ui32 Graphics::getViewport() const {
		return _glStatus.vp;
	}

	void Graphics::setViewport(const Box2i32ui32& vp) {
		if (_app) {
			if (_glStatus.vp != vp) {
				_glStatus.vp = vp;

				_updateViewport();
			}
		}
	}

	void Graphics::setBlendState(IBlendState* state, const Vec4f32& constantFactors, uint32_t sampleMask) {
		if (state && state->getGraphics() == this) {
			if (auto bs = state->getNative(); bs) {
				_setBlendState(*(BlendState*)bs, constantFactors, sampleMask);
			} else {
				_setBlendState(*(BlendState*)_defaultBlendState->getNative(), constantFactors, sampleMask);
			}
		} else if (_defaultBlendState) {
			_setBlendState(*(BlendState*)_defaultBlendState->getNative(), constantFactors, sampleMask);
		}
	}

	void Graphics::_setBlendState(BlendState& state, const Vec4f32& constantFactors, uint32_t sampleMask) {
		auto& blend = _glStatus.blend;

		if (_deviceFeatures.independentBlend) {
			if (state.isIndependentBlendEnabled()) {
				auto funcChanged = false, opChanged = false, writeMaskChanged = false;

				for (size_t i = 0; i < _deviceFeatures.simultaneousRenderTargetCount; ++i) {
					auto& rt = state.getInternalRenderTargetState(i);
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
				_setDependentBlendState<true>(state.getInternalRenderTargetState(0));
			}
		} else {
			_setDependentBlendState<false>(state.getInternalRenderTargetState(0));
		}

		if (blend.enabled && blend.constantFactors != constantFactors) {
			glBlendColor(constantFactors.data[0], constantFactors.data[1], constantFactors.data[2], constantFactors.data[3]);
			memcpy(&blend.constantFactors, &constantFactors, sizeof(Vec4f32));
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
						for (size_t i = 0; i < _deviceFeatures.simultaneousRenderTargetCount; ++i) blend.func[i].featureValue = rt.internalFunc.featureValue;
					}
				} else {
					auto needChange = false;
					for (size_t i = 0; i < _deviceFeatures.simultaneousRenderTargetCount; ++i) {
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
						for (size_t i = 0; i < _deviceFeatures.simultaneousRenderTargetCount; ++i) blend.op[i].featureValue = rt.internalOp.featureValue;
					}
				} else {
					auto needChange = false;
					for (size_t i = 0; i < _deviceFeatures.simultaneousRenderTargetCount; ++i) {
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
				for (size_t i = 0; i < _deviceFeatures.simultaneousRenderTargetCount; ++i) blend.writeMask[i].featureValue = rt.internalWriteMask.featureValue;
			}
		} else {
			auto needChange = false;
			for (size_t i = 0; i < _deviceFeatures.simultaneousRenderTargetCount; ++i) {
				if (blend.writeMask[i].featureValue != rt.internalWriteMask.featureValue) {
					blend.writeMask[i].featureValue = rt.internalWriteMask.featureValue;
					needChange = true;
				}
			}
			if (needChange) glColorMask(rt.internalWriteMask.rgba[0], rt.internalWriteMask.rgba[1], rt.internalWriteMask.rgba[2], rt.internalWriteMask.rgba[3]);
			blend.isWriteMaskSame = true;
		}
	}

	void Graphics::_updateCanvasSize(const Vec2ui32& size) {
		if (_glStatus.canvasSize != size) {
			_glStatus.canvasSize = size;
			_updateViewport();
		}
	}

	void Graphics::_updateViewport() {
		auto& vp = _glStatus.vp;
		glViewport(vp.pos[0], (int32_t)_glStatus.canvasSize[1] - (vp.pos[1] + (int32_t)vp.size[1]), vp.size[0], vp.size[1]);
	}

	void Graphics::setDepthStencilState(IDepthStencilState* state, uint32_t stencilFrontRef, uint32_t stencilBackRef) {
		if (state && state->getGraphics() == this) {
			if (auto native = state->getNative(); native) {
				_setDepthStencilState(*(DepthStencilState*)native, stencilFrontRef, stencilBackRef);
			} else {
				_setDepthStencilState(*(DepthStencilState*)_defaultDepthStencilState->getNative(), stencilFrontRef, stencilBackRef);
			}
		} else if (_defaultBlendState) {
			_setDepthStencilState(*(DepthStencilState*)_defaultDepthStencilState->getNative(), stencilFrontRef, stencilBackRef);
		}
	}

	void Graphics::_setDepthStencilState(DepthStencilState& state, uint32_t stencilFrontRef, uint32_t stencilBackRef) {
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
			auto frontRefChanged = false, backRefChanged = false;
			if (stencil.state.enabled == iss.enabled) {
				if (iss.enabled) {
					if (stencil.featureValue == state.getStencilFeatureValue()) {
						frontRefChanged = stencil.ref.front != stencilFrontRef;
						backRefChanged = stencil.ref.back != stencilBackRef;
					} else {
						changed = true;
					}
				}
			} else {
				if (iss.enabled) {
					glEnable(GL_STENCIL_TEST);

					if (stencil.featureValue == state.getStencilFeatureValue()) {
						frontRefChanged = stencil.ref.front != stencilFrontRef;
						backRefChanged = stencil.ref.back != stencilBackRef;
					} else {
						changed = true;
					}
				} else {
					glDisable(GL_STENCIL_TEST);
				}
				stencil.state.enabled = iss.enabled;
			}

			if (changed) {
				if (stencil.state.face.front.func != iss.face.front.func || stencil.ref.front != stencilFrontRef || stencil.state.face.front.mask.read != iss.face.front.mask.read) {
					glStencilFuncSeparate(GL_FRONT, iss.face.front.func, stencilFrontRef, iss.face.front.mask.read);
					stencil.ref.front = stencilFrontRef;
				}
				if (stencil.state.face.back.func != iss.face.back.func || stencil.ref.back != stencilBackRef || stencil.state.face.back.mask.read != iss.face.back.mask.read) {
					glStencilFuncSeparate(GL_BACK, iss.face.back.func, stencilBackRef, iss.face.back.mask.read);
					stencil.ref.back = stencilBackRef;
				}

				if (stencil.state.face.front.mask.write != iss.face.front.mask.write) glStencilMaskSeparate(GL_FRONT, iss.face.front.mask.write);
				if (stencil.state.face.back.mask.write != iss.face.back.mask.write) glStencilMaskSeparate(GL_BACK, iss.face.back.mask.write);

				if (stencil.state.face.front.op.featureValue != iss.face.front.op.featureValue)
					glStencilOpSeparate(GL_FRONT, iss.face.front.op.fail, iss.face.front.op.depthFail, iss.face.front.op.pass);

				if (stencil.state.face.back.op.featureValue != iss.face.back.op.featureValue)
					glStencilOpSeparate(GL_BACK, iss.face.back.op.fail, iss.face.back.op.depthFail, iss.face.back.op.pass);

				stencil.state.face = iss.face;
				stencil.featureValue = state.getStencilFeatureValue();
			} else {
				if (frontRefChanged) {
					glStencilFuncSeparate(GL_FRONT, iss.face.front.func, stencilFrontRef, iss.face.front.mask.read);
					stencil.ref.front = stencilFrontRef;
				}

				if (backRefChanged) {
					glStencilFuncSeparate(GL_BACK, iss.face.back.func, stencilBackRef, iss.face.back.mask.read);
					stencil.ref.back = stencilBackRef;
				}
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
		if (rasterizer.featureValue != state.getFeatureValue()) {
			auto& internalState = state.getInternalState();

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

			rasterizer.featureValue = state.getFeatureValue();
		}
	}

	void Graphics::beginRender() {
		wglMakeCurrent(_dc, _rc);
	}

	void Graphics::draw(const IVertexBufferGetter* vertexBufferGetter, IProgram* program, const IShaderParameterGetter* shaderParamGetter, const IIndexBuffer* indexBuffer, uint32_t count, uint32_t offset) {
		if (vertexBufferGetter && indexBuffer && program && program->getGraphics() == this && indexBuffer->getGraphics() == this && count > 0) {
			if (auto ib = (const IndexBuffer*)indexBuffer->getNative(); ib) {
				if (auto p = (Program*)program->getNative(); p && p->use(vertexBufferGetter, shaderParamGetter)) {
					ib->draw(count, offset);

					_constantBufferManager.resetUsedShareConstantBuffers();
				}
			}
		}
	}

	void Graphics::endRender() {
		//交换当前缓冲区和后台缓冲区
		//SwapBuffers(_dc);

		//if (wglGetCurrentContext() == _rc) wglMakeCurrent(nullptr, nullptr);
	}

	void Graphics::present() {
		SwapBuffers(_dc);
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

	bool Graphics::_glInit() {
		auto initOk = false;

		auto hIns = GetModuleHandle(nullptr);
		std::wstring className = L"Aurora OpenGL Temp Window " + String::Utf8ToUnicode(String::toString(Time::now()));

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

						if (glewInit() == GLEW_OK) {
							initOk = true;

							GLint val;
							glGetIntegerv(GL_MAX_SAMPLES, &val);
							_deviceFeatures.maxSampleCount = val;
						}

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
			_glStatus.isBack = true;
			_glStatus.backSize = _app->getClientSize();
			_glStatus.canvasSize = _glStatus.backSize;

			GLint vp[4];
			glGetIntegerv(GL_VIEWPORT, vp);

			_glStatus.vp.pos.set(vp[0], (int32_t)_glStatus.canvasSize[1] - (vp[1] + vp[3]));
			_glStatus.vp.size.set(vp[2], vp[3]);
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

				for (size_t i = 1; i < _deviceFeatures.simultaneousRenderTargetCount; ++i) {
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
			stencil.ref.front = val[0];
			stencil.ref.back = val[1];

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

			stencil.featureValue = calcHash(stencil.state.face);
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

			rasterizer.featureValue = calcHash(rasterizer.state);
		}
	}

	void Graphics::_resizedHandler(events::Event<ApplicationEvent>& e) {
		_resize(_app->getClientSize());
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
		_deviceFeatures.reset();
		_deviceVersion = "OpenGL Unknown";
		_bufferCreateUsageMask = Usage::NONE;
	}

	void Graphics::_resize(const Vec2ui32& size) {
		_glStatus.backSize = size;
		if (_glStatus.isBack) _updateCanvasSize(_glStatus.backSize);
	}

	void Graphics::_checkBlendFuncIsSame() {
		auto& blend = _glStatus.blend;
		blend.isFuncSame = true;
		for (size_t i = 1; i < _deviceFeatures.simultaneousRenderTargetCount; ++i) {
			if (blend.func[0].featureValue != blend.func[i].featureValue) {
				blend.isFuncSame = false;
				break;
			}
		}
	}

	void Graphics::_checkBlendOpIsSame() {
		auto& blend = _glStatus.blend;
		blend.isOpSame = true;
		for (size_t i = 1; i < _deviceFeatures.simultaneousRenderTargetCount; ++i) {
			if (blend.op[0].featureValue != blend.op[i].featureValue) {
				blend.isOpSame = false;
				break;
			}
		}
	}

	void Graphics::_checkBlendWriteMaskIsSame() {
		auto& blend = _glStatus.blend;
		blend.isWriteMaskSame = true;
		for (size_t i = 1; i < _deviceFeatures.simultaneousRenderTargetCount; ++i) {
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
		std::string_view mv(message);

		if (String::find(mv, "error") != std::string::npos ||
			String::find(mv, "warning") != std::string::npos ||
			String::find(mv, "failed") != std::string::npos) {
			((Graphics*)userParam)->error(std::string("OpenGL sys message : ") + message);
		}
	}
}