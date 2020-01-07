#include "ByteArray.h"

namespace aurora {
	ByteArray::ByteArray(ByteArray&& bytes) noexcept :
		_endian(bytes._endian),
		_needReverse(bytes._needReverse),
		_capacity(bytes._capacity),
		_length(bytes._length),
		_position(bytes._position),
		_usage(bytes._usage),
		_data(bytes._data) {
		bytes._data = nullptr;
	}

	ByteArray::ByteArray(size_t capacity, size_t length) :
		_endian(std::endian::little),
		_needReverse(std::endian::native != std::endian::little),
		_capacity(capacity),
		_length(length > capacity ? capacity : length),
		_position(0),
		_usage(Usage::EXCLUSIVE),
		_data(capacity > 0 ? new uint8_t[capacity] : nullptr) {
	}

	ByteArray::ByteArray(uint8_t* bytes, size_t size, Usage usage) : ByteArray(bytes, size, size, usage) {
	}

	ByteArray::ByteArray(uint8_t* bytes, size_t capacity, size_t length, Usage usage) :
		_endian(std::endian::little),
		_needReverse(std::endian::native != std::endian::little),
		_capacity(capacity),
		_length(length),
		_position(0) {
		if (usage == Usage::SHARED) {
			this->_data = bytes;
			_usage = Usage::SHARED;
		} else if (usage == Usage::COPY) {
			_data = new uint8_t[capacity];
			memcpy(_data, bytes, length);
			_usage = Usage::EXCLUSIVE;
		} else {
			this->_data = (uint8_t*)bytes;
			_usage = Usage::EXCLUSIVE;
		}
	}

	ByteArray& ByteArray::operator=(ByteArray&& value) noexcept {
		dispose(true);

		_endian = value._endian;
		_needReverse = value._needReverse;
		_capacity = value._capacity;
		_length = value._length;
		_position = value._position;
		_usage = value._usage;
		_data = value._data;

		value._data = nullptr;

		return *this;
	}

	ByteArray::~ByteArray() {
		dispose();
	}

	void ByteArray::dispose(bool free) {
		if (_data) {
			if (free && _usage != Usage::SHARED) delete[] _data;
			_data = nullptr;
		}
	}

	void ByteArray::_resize(size_t len) {
		auto newBytes = len ? new uint8_t[len] : nullptr;

		memcpy(newBytes, _data, len > _capacity ? _capacity : len);
		_capacity = len;

		if (_usage == Usage::SHARED) {
			_usage = Usage::EXCLUSIVE;
		} else {
			if (_data != nullptr) delete[] _data;
		}

		_data = newBytes;
	}

	void ByteArray::popFront(size_t len) {
		if (len) {
			if (_length <= len) {
				_length = 0;
				_position = 0;
			} else {
				_length -= len;
				for (size_t i = 0; i < _length; ++i) _data[i] = _data[len + i];

				if (_position <= len) {
					_position = 0;
				} else {
					_position -= len;
				}
			}
		}
	}

	void ByteArray::popBack(size_t len) {
		if (len) {
			if (_length < len) {
				_length = 0;
				_position = 0;
			} else {
				_length -= len;
				if (_position > _length) _position = _length;
			}
		}
	}

	void ByteArray::insert(size_t len) {
		if (len) {
			auto oldLen = _length;

			_checkLength(len);

			if (oldLen) {
				for (size_t i = oldLen - 1; ; --i) {
					_data[i + len] = _data[i];
					if (i == _position) break;
				}
			}
		}
	}

	bool ByteArray::isEqual(const uint8_t* data1, const uint8_t* data2, size_t len) {
		for (size_t i = 0; i < len; ++i) {
			if (data1[i] != data2[i]) return false;
		}

		return true;
	}
}