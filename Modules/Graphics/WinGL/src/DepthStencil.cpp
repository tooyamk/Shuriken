#include "DepthStencil.h"
#include "Graphics.h"

namespace aurora::modules::graphics::win_gl {
	DepthStencil::DepthStencil(Graphics& graphics) : IDepthStencil(graphics),
		_isMultisampling(false),
		_handle(0),
		_attachmentType(GL_NONE) {
	}

	DepthStencil::~DepthStencil() {
		destroy();
	}

	const void* DepthStencil::getNative() const {
		return this;
	}

	bool DepthStencil::isMultisampling() const {
		return _isMultisampling;
	}

	const Vec2ui32& DepthStencil::getSize() const {
		return _size;
	}

	bool DepthStencil::create(const Vec2ui32& size, DepthStencilFormat format, bool multisampling) {
		destroy();

		auto [fmt, attachment] = convertInternalFormat(format);
		if (fmt == GL_NONE) {
			_graphics.get<Graphics>()->error("openGL DepthStencil::create error : format error");
			return false;
		}

		glGenRenderbuffers(1, &_handle);
		glBindRenderbuffer(GL_RENDERBUFFER, _handle);
		glRenderbufferStorage(GL_RENDERBUFFER, fmt, size[0], size[1]);
		//glRenderbufferStorageMultisample()

		_size.set(size);
		_isMultisampling = multisampling;
		_attachmentType = attachment;

		return true;
	}

	void DepthStencil::destroy() {
		if (_handle) {
			glDeleteRenderbuffers(1, &_handle);
			_handle = 0;
		}

		_size.set(0);
		_isMultisampling = false;
		_attachmentType = GL_NONE;
	}

	std::tuple<GLenum, GLenum> DepthStencil::convertInternalFormat(DepthStencilFormat fmt) {
		switch (fmt) {
		case DepthStencilFormat::D16:
			return std::make_tuple(GL_DEPTH_COMPONENT16, GL_DEPTH_ATTACHMENT);
		case DepthStencilFormat::D24:
			return std::make_tuple(GL_DEPTH_COMPONENT24, GL_DEPTH_ATTACHMENT);
		case DepthStencilFormat::D24S8:
			return std::make_tuple(GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL_ATTACHMENT);
		case DepthStencilFormat::D32:
			return std::make_tuple(GL_DEPTH_COMPONENT32, GL_DEPTH_ATTACHMENT);
		case DepthStencilFormat::D32S8:
			return std::make_tuple(GL_DEPTH32F_STENCIL8, GL_DEPTH_STENCIL_ATTACHMENT);
		default:
			return std::make_tuple(GL_NONE, GL_NONE);
		}
	}
}