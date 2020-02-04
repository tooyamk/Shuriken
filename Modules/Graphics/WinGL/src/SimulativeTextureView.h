#pragma once

#include "Base.h"

namespace aurora::modules::graphics::win_gl {
	class Graphics;

	class AE_MODULE_DLL SimulativeTextureView : public ITextureView {
	public:
		SimulativeTextureView(Graphics& graphics);
		virtual ~SimulativeTextureView();

		virtual bool AE_CALL isCreated() const override;
		virtual ITextureResource* AE_CALL getResource() const override;
		virtual const void* AE_CALL getNative() const override;
		virtual uint32_t AE_CALL getArraySize() const override;
		virtual uint32_t AE_CALL getMipLevels() const override;
		virtual bool AE_CALL create(ITextureResource* res, uint32_t mipBegin, uint32_t mipLevels, uint32_t arrayBegin, uint32_t arraySize) override;
		virtual void AE_CALL destroy() override;

	protected:
		uint32_t _mipBegin;
		uint32_t _mipLevels;
		uint32_t _createdMipLevels;
		uint32_t _arrayBegin;
		uint32_t _arraySize;
		uint32_t _createdArraySize;

		RefPtr<ITextureResource> _res;
		GLuint _handle;

		bool AE_CALL _createDone(bool succeeded, ITextureResource* res);
	};
}