#pragma once

#include "IndexBuffer.h"
#include "Program.h"
#include "VertexBuffer.h"

namespace aurora::modules::graphics::win_glew {
	class AE_MODULE_DLL Graphics : public IGraphicsModule {
	public:
		Graphics(Application* app, IProgramSourceTranslator* trans);
		virtual ~Graphics();

		virtual bool AE_CALL createDevice() override;

		virtual IIndexBuffer* AE_CALL createIndexBuffer() override;
		virtual IProgram* AE_CALL createProgram() override;
		virtual IVertexBuffer* AE_CALL createVertexBuffer() override;
		
		virtual void AE_CALL beginRender() override;
		virtual void AE_CALL endRender() override;
		virtual void AE_CALL present() override;

		virtual void AE_CALL clear() override;

		inline IProgramSourceTranslator* getProgramSourceTranslator() const {
			return _trans;
		}

		inline i8 compareVersion(GLint major, GLint minor) const {
			if (major > _majorVer) {
				return 1;
			} else if (major < _majorVer) {
				return -1;
			} else {
				return minor - _minorVer;
			}
		}

		inline ui32 getIntVersion() const {
			return _intVer;
		}

		inline const std::string& getStringVersion() const {
			return _strVer;
		}

	private:
		Application* _app;
		IProgramSourceTranslator* _trans;

		HDC _dc;
		HGLRC _rc;

		GLint _majorVer;
		GLint _minorVer;
		ui32 _intVer;
		std::string _strVer;

		void AE_CALL _release();
	};
}