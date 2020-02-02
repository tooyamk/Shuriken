#pragma once

#include "Base.h"
#include "modules/graphics/ConstantBufferManager.h"

namespace aurora::modules::graphics::win_gl {
	class BlendState;
	class DepthStencilState;
	class RasterizerState;

	class AE_MODULE_DLL Graphics : public IGraphicsModule {
	public:
		struct InternalFeatures {
			bool supportTexStorage;
			GLfloat maxAnisotropy;
		};


		Graphics(Ref* loader, Application* app, IProgramSourceTranslator* trans);
		virtual ~Graphics();

		virtual events::IEventDispatcher<GraphicsEvent>& AE_CALL getEventDispatcher() override;
		virtual const events::IEventDispatcher<GraphicsEvent>& AE_CALL getEventDispatcher() const override;

		virtual const std::string& AE_CALL getVersion() const override;
		virtual const GraphicsDeviceFeatures& AE_CALL getDeviceFeatures() const override;
		virtual IBlendState* AE_CALL createBlendState() override;
		virtual IConstantBuffer* AE_CALL createConstantBuffer() override;
		virtual IDepthStencilState* AE_CALL createDepthStencilState() override;
		virtual IIndexBuffer* AE_CALL createIndexBuffer() override;
		virtual IProgram* AE_CALL createProgram() override;
		virtual IRasterizerState* AE_CALL createRasterizerState() override;
		virtual ISampler* AE_CALL createSampler() override;
		virtual ITexture1DResource* AE_CALL createTexture1DResource() override;
		virtual ITexture2DResource* AE_CALL createTexture2DResource() override;
		virtual ITexture3DResource* AE_CALL createTexture3DResource() override;
		virtual ITextureView* AE_CALL createTextureView() override;
		virtual IVertexBuffer* AE_CALL createVertexBuffer() override;
		virtual IPixelBuffer* AE_CALL createPixelBuffer() override;

		virtual void AE_CALL setBlendState(IBlendState* state, const Vec4f32& constantFactors, uint32_t sampleMask = (std::numeric_limits<uint32_t>::max)()) override;
		virtual void AE_CALL setDepthStencilState(IDepthStencilState* state, uint32_t stencilFrontRef, uint32_t stencilBackRef) override;
		virtual void AE_CALL setRasterizerState(IRasterizerState* state) override;
		
		virtual void AE_CALL beginRender() override;
		virtual void AE_CALL draw(const VertexBufferFactory* vertexFactory, IProgram* program, const ShaderParameterFactory* paramFactory,
			const IIndexBuffer* indexBuffer, uint32_t count = (std::numeric_limits<uint32_t>::max)(), uint32_t offset = 0) override;
		virtual void AE_CALL endRender() override;
		virtual void AE_CALL present() override;

		virtual void AE_CALL clear(ClearFlag flags, const Vec4f32& color, f32 depth, size_t stencil) override;

		bool AE_CALL createDevice(const GraphicsAdapter* adapter);

		inline void AE_CALL error(const std::string_view& msg) {
			_eventDispatcher.dispatchEvent(this, GraphicsEvent::ERR, (std::string_view*)&msg);
		}

		inline IProgramSourceTranslator* AE_CALL getProgramSourceTranslator() const {
			return _trans.get();
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
				bool isFuncSame;
				bool isOpSame;
				bool isWriteMaskSame;
				InternalBlendFunc func[MAX_RTS];
				InternalBlendOp op[MAX_RTS];
				InternalBlendWriteMask writeMask[MAX_RTS];
				Vec4f32 constantFactors;
			} blend;

			struct {
				Vec4f32 color;
				f32 depth;
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
		} _glStatus;


		Usage _bufferCreateUsageMask;
		Usage _texCreateUsageMask;

		RefPtr<Ref> _loader;
		RefPtr<Application> _app;
		RefPtr<IProgramSourceTranslator> _trans;

		RefPtr<BlendState> _defaultBlendState;
		RefPtr<DepthStencilState> _defaultDepthStencilState;
		RefPtr<RasterizerState> _defaultRasterizerState;

		InternalFeatures _internalFeatures;
		GraphicsDeviceFeatures _deviceFeatures;

		HDC _dc;
		HGLRC _rc;

		GLint _majorVer;
		GLint _minorVer;
		uint32_t _intVer;
		std::string _strVer;
		inline static const std::string _moduleVersion = "0.1.0";
		std::string _deviceVersion;

		ConstantBufferManager _constantBufferManager;

		events::EventDispatcher<GraphicsEvent> _eventDispatcher;

		bool AE_CALL _glInit();
		void AE_CALL _setInitState();
		void AE_CALL _release();

		void AE_CALL _setBlendState(IBlendState& state, const Vec4f32& constantFactors, uint32_t sampleMask);
		void AE_CALL _setDepthStencilState(IDepthStencilState& state, uint32_t stencilFrontRef, uint32_t stencilBackRef);
		void AE_CALL _setRasterizerState(IRasterizerState& state);

		IConstantBuffer* AE_CALL _createdShareConstantBuffer();
		IConstantBuffer* AE_CALL _createdExclusiveConstantBuffer(uint32_t numParameters);

		void AE_CALL _checkBlendFuncIsSame();
		void AE_CALL _checkBlendOpIsSame();
		void AE_CALL _checkBlendWriteMaskIsSame();

		template<bool SupportIndependentBlend>
		void AE_CALL _setDependentBlendState(const InternalRenderTargetBlendState& rt);

		static void GLAPIENTRY _debugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam);
	};
}