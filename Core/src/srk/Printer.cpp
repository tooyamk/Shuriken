#include "Printer.h"
#include "srk/Application.h"
#include "srk/StringUtility.h"

#include <functional>
#include <any>

#if SRK_OS == SRK_OS_MACOS
#	include "srk/Application.h"
#endif

#if __has_include(<android/log.h>)
#	define SRK_HAS_ANDROID_LOG_H
#	include <android/log.h>
#endif

namespace srk {
	Printer::OutputBuffer::OutputBuffer() :
		_needFree(false),
		_pos(0),
		_size(0),
		_data(nullptr) {
	}

	Printer::OutputBuffer::OutputBuffer(wchar_t* data, size_t size, bool needFree) :
		_needFree(needFree),
		_pos(0),
		_size(size),
		_data(data) {
	}

	Printer::OutputBuffer::~OutputBuffer() {
		if (_needFree) delete[] _data;
	}

	void Printer::OutputBuffer::write(const char* buf, size_t size) {
		auto [chars, bytes] = StringUtility::calcUtf8(std::string_view(buf, size));
		if (chars) {
			dilatation(chars);
			_pos += StringUtility::utf8ToWide(buf, bytes, _data + _pos);
		}
	}

	void Printer::OutputBuffer::write(const wchar_t* buf, size_t size) {
		dilatation(size);
		memcpy(_data + _pos, buf, sizeof(wchar_t) * size);
		_pos += size;
	}

	void Printer::OutputBuffer::write(const char16_t* buf, size_t size) {
		dilatation(size);

		if constexpr (sizeof(wchar_t) == sizeof(char16_t)) {
			memcpy(_data + _pos, buf, sizeof(wchar_t) * size);
			_pos += size;
		} else {
			for (size_t i = 0; i < size; ++i) _data[_pos++] = buf[i];
		}
	}

	void Printer::OutputBuffer::write(const char32_t* buf, size_t size) {
		dilatation(size);

		if constexpr (sizeof(wchar_t) == sizeof(char32_t)) {
			memcpy(_data + _pos, buf, sizeof(wchar_t) * size);
			_pos += size;
		} else {
			for (size_t i = 0; i < size; ++i) _data[_pos++] = buf[i];
		}
	}

	void Printer::OutputBuffer::dilatation(size_t size) {
		if (size > _size - _pos) {
			_size = _pos + size + (size >> 1);
			auto buf = new wchar_t[this->_size];
			memcpy(buf, _data, _pos * sizeof(wchar_t));
			if (_needFree) {
				delete[] _data;
			} else {
				_needFree = true;
			}
			_data = buf;
		}
	}


	bool Printer::DebuggerOutputer::operator()(const std::wstring_view& data) const {
		if (Application::isDebuggerAttached()) {
#if SRK_OS == SRK_OS_WINDOWS
			::OutputDebugStringW(data.data());
			return true;
#elif SRK_OS == SRK_OS_ANDROID
#	ifdef SRK_HAS_ANDROID_LOG_H
			__android_log_print(ANDROID_LOG_INFO, "Shuriken", "%ls", data.data());
			return true;
#	endif
#endif
		}

		return false;
	}
}