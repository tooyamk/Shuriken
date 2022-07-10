#pragma once

#include "Base.h"
#include "srk/windows/IWindow.h"
#include "srk/modules/graphics/ConstantBufferManager.h"

namespace srk::modules::graphics::gl {
	class BlendState;
	class DepthStencilState;
	class RasterizerState;

	class SRK_MODULE_DLL Graphics : public IGraphicsModule {
	public:
		struct InternalFeatures {
			bool supportTexStorage;
			GLfloat maxAnisotropy;
		};


		struct CreateConfig {
			Ref* loader = nullptr;
			IWindow* win = nullptr;
			IShaderTranspiler* transpiler = nullptr;
			GraphicsAdapter* adapter = nullptr;
			SampleCount sampleCount = 1;
			bool debug = false;
		};


		Graphics();
		virtual ~Graphics();

		void operator delete(Graphics* p, std::destroying_delete_t) {
			auto l = p->_loader;
			p->~Graphics();
			::operator delete(p);
		}

		virtual IntrusivePtr<events::IEventDispatcher<GraphicsEvent>> SRK_CALL getEventDispatcher() override;

		virtual const std::string& SRK_CALL getVersion() const override;
		virtual const GraphicsDeviceFeatures& SRK_CALL getDeviceFeatures() const override;
		virtual IntrusivePtr<IBlendState> SRK_CALL createBlendState() override;
		virtual IntrusivePtr<IConstantBuffer> SRK_CALL createConstantBuffer() override;
		virtual IntrusivePtr<IDepthStencil> SRK_CALL createDepthStencil() override;
		virtual IntrusivePtr<IDepthStencilState> SRK_CALL createDepthStencilState() override;
		virtual IntrusivePtr<IIndexBuffer> SRK_CALL createIndexBuffer() override;
		virtual IntrusivePtr<IProgram> SRK_CALL createProgram() override;
		virtual IntrusivePtr<IRasterizerState> SRK_CALL createRasterizerState() override;
		virtual IntrusivePtr<IRenderTarget> SRK_CALL createRenderTarget() override;
		virtual IntrusivePtr<IRenderView> SRK_CALL createRenderView() override;
		virtual IntrusivePtr<ISampler> SRK_CALL createSampler() override;
		virtual IntrusivePtr<ITexture1DResource> SRK_CALL createTexture1DResource() override;
		virtual IntrusivePtr<ITexture2DResource> SRK_CALL createTexture2DResource() override;
		virtual IntrusivePtr<ITexture3DResource> SRK_CALL createTexture3DResource() override;
		virtual IntrusivePtr<ITextureView> SRK_CALL createTextureView() override;
		virtual IntrusivePtr<IVertexBuffer> SRK_CALL createVertexBuffer() override;
		virtual IntrusivePtr<IPixelBuffer> SRK_CALL createPixelBuffer() override;

		virtual const Vec2ui32& SRK_CALL getBackBufferSize() const override;
		virtual void SRK_CALL setBackBufferSize(const Vec2ui32& size) override;
		virtual Box2i32ui32 SRK_CALL getViewport() const override;
		virtual void SRK_CALL setViewport(const Box2i32ui32& vp) override;
		virtual void SRK_CALL setBlendState(IBlendState* state, const Vec4f32& constantFactors, uint32_t sampleMask = (std::numeric_limits<uint32_t>::max)()) override;
		virtual void SRK_CALL setDepthStencilState(IDepthStencilState* state, uint32_t stencilFrontRef, uint32_t stencilBackRef) override;
		virtual void SRK_CALL setRasterizerState(IRasterizerState* state) override;
		
		virtual void SRK_CALL beginRender() override;
		virtual void SRK_CALL draw(const IVertexBufferGetter* vertexBufferGetter, IProgram* program, const IShaderParameterGetter* shaderParamGetter,
			const IIndexBuffer* indexBuffer, uint32_t count = (std::numeric_limits<uint32_t>::max)(), uint32_t offset = 0) override;
		virtual void SRK_CALL drawInstanced(const IVertexBufferGetter* vertexBufferGetter, IProgram* program, const IShaderParameterGetter* shaderParamGetter,
			const IIndexBuffer* indexBuffer, uint32_t instancedCount, uint32_t count = (std::numeric_limits<uint32_t>::max)(), uint32_t offset = 0) override;
		virtual void SRK_CALL endRender() override;
		virtual void SRK_CALL flush() override;
		virtual void SRK_CALL present() override;

		virtual void SRK_CALL setRenderTarget(IRenderTarget* rt) override;
		virtual void SRK_CALL clear(ClearFlag flags, const Vec4f32& color, float32_t depth, size_t stencil) override;

		bool SRK_CALL createDevice(const CreateConfig& conf);

		inline void SRK_CALL error(const std::string_view& msg) {
			_eventDispatcher->dispatchEvent(this, GraphicsEvent::ERR, (std::string_view*)&msg);
		}

		inline IShaderTranspiler* SRK_CALL getShaderTranspiler() const {
			return _transpiler.get();
		}

		inline ConstantBufferManager& SRK_CALL getConstantBufferManager() {
			return _constantBufferManager;
		}

		inline bool SRK_CALL isGreatThanOrEqualVersion(GLint major, GLint minor) const {
			return _majorVer > major ? true : (_majorVer < major ? false : _minorVer >= minor);
		}

		inline uint32_t SRK_CALL getIntVersion() const {
			return _intVer;
		}

		inline const std::string& SRK_CALL getStringVersion() const {
			return _strVer;
		}

		inline const InternalFeatures& SRK_CALL getInternalFeatures() const {
			return _internalFeatures;
		}

		inline const Usage SRK_CALL getBufferCreateUsageMask() const {
			return _bufferCreateUsageMask;
		}

		inline const Usage SRK_CALL getTexCreateUsageMask() const {
			return _texCreateUsageMask;
		}

		struct ConvertFormatResult {
			ConvertFormatResult(GLenum internalFormat, GLenum format, GLenum type) : internalFormat(internalFormat), format(format), type(type) {}
			GLenum internalFormat;
			GLenum format;
			GLenum type;
		};

		static std::optional<ConvertFormatResult> SRK_CALL convertFormat(TextureFormat fmt);
		static GLenum SRK_CALL convertComparisonFunc(ComparisonFunc func);
		static uint32_t SRK_CALL getGLTypeSize(GLenum type);

	private:
		struct {
			struct {
				uint8_t enabled;
				std::vector<InternalBlendFunc> func;
				std::vector<InternalBlendOp> op;
				std::vector<InternalBlendWriteMask> writeMask;
				Vec4f32 constantFactors;
			} blend;

			struct {
				Vec4f32 color;
				float32_t depth;
				size_t stencil;
			} clear;

			struct {
				uint64_t featureValue;
				InternalRasterizerState state;
			} rasterizer;

			InternalDepthState depth;

			struct {
				uint64_t featureValue;
				struct {
					uint32_t front;
					uint32_t back;
				} ref;
				InternalStencilState state;
			} stencil;

			bool isBack;
			Vec2ui32 backSize;
			Vec2ui32 canvasSize;
			Box2i32ui32 vp;
		} _glStatus;


		Usage _bufferCreateUsageMask;
		Usage _texCreateUsageMask;

		IntrusivePtr<Ref> _loader;
		IntrusivePtr<IWindow> _win;
		IntrusivePtr<IShaderTranspiler> _transpiler;

		IntrusivePtr<BlendState> _defaultBlendState;
		IntrusivePtr<DepthStencilState> _defaultDepthStencilState;
		IntrusivePtr<RasterizerState> _defaultRasterizerState;

		InternalFeatures _internalFeatures;
		GraphicsDeviceFeatures _deviceFeatures;


#if SRK_OS == SRK_OS_WINDOWS
		HDC _dc;
		HGLRC _rc;
#elif SRK_OS == SRK_OS_LINUX
		GLXContext _context;
#endif

		GLint _majorVer;
		GLint _minorVer;
		uint32_t _intVer;
		std::string _strVer;
		inline static const std::string _moduleVersion = "0.1.0";
		std::string _deviceVersion;

		ConstantBufferManager _constantBufferManager;

		IntrusivePtr<events::IEventDispatcher<GraphicsEvent>> _eventDispatcher;

		bool SRK_CALL _glInit(IWindow* win);
		bool SRK_CALL _glewInit();
		void SRK_CALL _setInitState();
		void SRK_CALL _release(IWindow* win = nullptr);
		void SRK_CALL _resize(const Vec2ui32& size);

		void SRK_CALL _setBlendState(BlendState& state, const Vec4f32& constantFactors, uint32_t sampleMask);
		void SRK_CALL _setDepthStencilState(DepthStencilState& state, uint32_t stencilFrontRef, uint32_t stencilBackRef);
		void SRK_CALL _setRasterizerState(RasterizerState& state);

		IConstantBuffer* SRK_CALL _createdShareConstantBuffer();
		IConstantBuffer* SRK_CALL _createdExclusiveConstantBuffer(uint32_t numParameters);

		template<bool SupportIndependentBlend>
		void SRK_CALL _setDependentBlendState(const InternalRenderTargetBlendState& rt);

		void SRK_CALL _updateCanvasSize(const Vec2ui32& size);
		void SRK_CALL _updateViewport();

		static void GLAPIENTRY _debugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam);
	};
}