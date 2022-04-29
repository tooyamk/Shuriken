#pragma once

#include "Base.h"
#include "aurora/modules/graphics/ConstantBufferManager.h"

namespace aurora::modules::graphics::gl {
	class BlendState;
	class DepthStencilState;
	class RasterizerState;

	class AE_MODULE_DLL Graphics : public IGraphicsModule {
	public:
		struct InternalFeatures {
			bool supportTexStorage;
			GLfloat maxAnisotropy;
		};


		struct CreateConfig {
			Ref* loader = nullptr;
			IApplication* app = nullptr;
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

		virtual IntrusivePtr<events::IEventDispatcher<GraphicsEvent>> AE_CALL getEventDispatcher() override;

		virtual const std::string& AE_CALL getVersion() const override;
		virtual const GraphicsDeviceFeatures& AE_CALL getDeviceFeatures() const override;
		virtual IntrusivePtr<IBlendState> AE_CALL createBlendState() override;
		virtual IntrusivePtr<IConstantBuffer> AE_CALL createConstantBuffer() override;
		virtual IntrusivePtr<IDepthStencil> AE_CALL createDepthStencil() override;
		virtual IntrusivePtr<IDepthStencilState> AE_CALL createDepthStencilState() override;
		virtual IntrusivePtr<IIndexBuffer> AE_CALL createIndexBuffer() override;
		virtual IntrusivePtr<IProgram> AE_CALL createProgram() override;
		virtual IntrusivePtr<IRasterizerState> AE_CALL createRasterizerState() override;
		virtual IntrusivePtr<IRenderTarget> AE_CALL createRenderTarget() override;
		virtual IntrusivePtr<IRenderView> AE_CALL createRenderView() override;
		virtual IntrusivePtr<ISampler> AE_CALL createSampler() override;
		virtual IntrusivePtr<ITexture1DResource> AE_CALL createTexture1DResource() override;
		virtual IntrusivePtr<ITexture2DResource> AE_CALL createTexture2DResource() override;
		virtual IntrusivePtr<ITexture3DResource> AE_CALL createTexture3DResource() override;
		virtual IntrusivePtr<ITextureView> AE_CALL createTextureView() override;
		virtual IntrusivePtr<IVertexBuffer> AE_CALL createVertexBuffer() override;
		virtual IntrusivePtr<IPixelBuffer> AE_CALL createPixelBuffer() override;

		virtual const Vec2ui32& AE_CALL getBackBufferSize() const override;
		virtual void AE_CALL setBackBufferSize(const Vec2ui32& size) override;
		virtual Box2i32ui32 AE_CALL getViewport() const override;
		virtual void AE_CALL setViewport(const Box2i32ui32& vp) override;
		virtual void AE_CALL setBlendState(IBlendState* state, const Vec4f32& constantFactors, uint32_t sampleMask = (std::numeric_limits<uint32_t>::max)()) override;
		virtual void AE_CALL setDepthStencilState(IDepthStencilState* state, uint32_t stencilFrontRef, uint32_t stencilBackRef) override;
		virtual void AE_CALL setRasterizerState(IRasterizerState* state) override;
		
		virtual void AE_CALL beginRender() override;
		virtual void AE_CALL draw(const IVertexBufferGetter* vertexBufferGetter, IProgram* program, const IShaderParameterGetter* shaderParamGetter,
			const IIndexBuffer* indexBuffer, uint32_t count = (std::numeric_limits<uint32_t>::max)(), uint32_t offset = 0) override;
		virtual void AE_CALL drawInstanced(const IVertexBufferGetter* vertexBufferGetter, IProgram* program, const IShaderParameterGetter* shaderParamGetter,
			const IIndexBuffer* indexBuffer, uint32_t instancedCount, uint32_t count = (std::numeric_limits<uint32_t>::max)(), uint32_t offset = 0) override;
		virtual void AE_CALL endRender() override;
		virtual void AE_CALL flush() override;
		virtual void AE_CALL present() override;

		virtual void AE_CALL setRenderTarget(IRenderTarget* rt) override;
		virtual void AE_CALL clear(ClearFlag flags, const Vec4f32& color, float32_t depth, size_t stencil) override;

		bool AE_CALL createDevice(const CreateConfig& conf);

		inline void AE_CALL error(const std::string_view& msg) {
			_eventDispatcher->dispatchEvent(this, GraphicsEvent::ERR, (std::string_view*)&msg);
		}

		inline IShaderTranspiler* AE_CALL getShaderTranspiler() const {
			return _transpiler.get();
		}

		inline ConstantBufferManager& AE_CALL getConstantBufferManager() {
			return _constantBufferManager;
		}

		inline bool AE_CALL isGreatThanOrEqualVersion(GLint major, GLint minor) const {
			return _majorVer > major ? true : (_majorVer < major ? false : _minorVer >= minor);
		}

		inline uint32_t AE_CALL getIntVersion() const {
			return _intVer;
		}

		inline const std::string& AE_CALL getStringVersion() const {
			return _strVer;
		}

		inline const InternalFeatures& AE_CALL getInternalFeatures() const {
			return _internalFeatures;
		}

		inline const Usage AE_CALL getBufferCreateUsageMask() const {
			return _bufferCreateUsageMask;
		}

		inline const Usage AE_CALL getTexCreateUsageMask() const {
			return _texCreateUsageMask;
		}

		struct ConvertFormatResult {
			ConvertFormatResult(GLenum internalFormat, GLenum format, GLenum type) : internalFormat(internalFormat), format(format), type(type) {}
			GLenum internalFormat;
			GLenum format;
			GLenum type;
		};

		static std::optional<ConvertFormatResult> AE_CALL convertFormat(TextureFormat fmt);
		static GLenum AE_CALL convertComparisonFunc(ComparisonFunc func);
		static uint32_t AE_CALL getGLTypeSize(GLenum type);

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
		IntrusivePtr<IApplication> _app;
		IntrusivePtr<IShaderTranspiler> _transpiler;

		IntrusivePtr<BlendState> _defaultBlendState;
		IntrusivePtr<DepthStencilState> _defaultDepthStencilState;
		IntrusivePtr<RasterizerState> _defaultRasterizerState;

		InternalFeatures _internalFeatures;
		GraphicsDeviceFeatures _deviceFeatures;


#if AE_OS == AE_OS_WINDOWS
		HDC _dc;
		HGLRC _rc;
#elif AE_OS == AE_OS_LINUX
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

		bool AE_CALL _glInit(IApplication* app);
		bool AE_CALL _glewInit();
		void AE_CALL _setInitState();
		void AE_CALL _release(IApplication* app = nullptr);
		void AE_CALL _resize(const Vec2ui32& size);

		void AE_CALL _setBlendState(BlendState& state, const Vec4f32& constantFactors, uint32_t sampleMask);
		void AE_CALL _setDepthStencilState(DepthStencilState& state, uint32_t stencilFrontRef, uint32_t stencilBackRef);
		void AE_CALL _setRasterizerState(RasterizerState& state);

		IConstantBuffer* AE_CALL _createdShareConstantBuffer();
		IConstantBuffer* AE_CALL _createdExclusiveConstantBuffer(uint32_t numParameters);

		template<bool SupportIndependentBlend>
		void AE_CALL _setDependentBlendState(const InternalRenderTargetBlendState& rt);

		void AE_CALL _updateCanvasSize(const Vec2ui32& size);
		void AE_CALL _updateViewport();

		static void GLAPIENTRY _debugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam);
	};
}