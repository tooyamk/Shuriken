#pragma once

#include "Base.h"

namespace aurora::modules::graphics::win_glew {
	class AE_MODULE_DLL Graphics : public IGraphicsModule {
	public:
		Graphics(Application* app, IProgramSourceTranslator* trans);
		virtual ~Graphics();

		virtual bool AE_CALL createDevice(const GraphicsAdapter* adapter) override;

		virtual const std::string& AE_CALL getVersion() const override;
		virtual const GraphicsFeatures& AE_CALL getFeatures() const override;
		virtual IConstantBuffer* AE_CALL createConstantBuffer() override;
		virtual IIndexBuffer* AE_CALL createIndexBuffer() override;
		virtual IProgram* AE_CALL createProgram() override;
		virtual ISampler* AE_CALL createSampler() override;
		virtual ITexture1DResource* AE_CALL createTexture1DResource() override;
		virtual ITexture2DResource* AE_CALL createTexture2DResource() override;
		virtual ITexture3DResource* AE_CALL createTexture3DResource() override;
		virtual ITextureView* AE_CALL createTextureView() override;
		virtual IVertexBuffer* AE_CALL createVertexBuffer() override;
		
		virtual void AE_CALL beginRender() override;
		virtual void AE_CALL endRender() override;
		virtual void AE_CALL present() override;

		virtual void AE_CALL clear() override;

		inline IProgramSourceTranslator* AE_CALL getProgramSourceTranslator() const {
			return _trans.get();
		}

		inline bool AE_CALL isGreatThanVersion(GLint major, GLint minor) const {
			if (_majorVer > major) {
				return true;
			} else if (_majorVer < major) {
				return false;
			} else {
				return _minorVer >= minor;
			}
		}

		inline ui32 AE_CALL getIntVersion() const {
			return _intVer;
		}

		inline const std::string& AE_CALL getStringVersion() const {
			return _strVer;
		}

		static GLenum AE_CALL convertInternalFormat(TextureFormat fmt);

	private:
		RefPtr<Application> _app;
		RefPtr<IProgramSourceTranslator> _trans;

		GraphicsFeatures _features;

		HDC _dc;
		HGLRC _rc;

		GLint _majorVer;
		GLint _minorVer;
		ui32 _intVer;
		std::string _strVer;
		std::string _fullVer;

		void AE_CALL _release();
	};
}