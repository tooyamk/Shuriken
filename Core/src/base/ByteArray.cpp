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
		_endian(std::endian::native),
		_needReverse(false),
		_capacity(capacity),
		_length(length > capacity ? capacity : length),
		_position(0),
		_usage(Usage::EXCLUSIVE),
		_data(capacity > 0 ? new uint8_t[capacity] : nullptr) {
	}

	ByteArray::ByteArray(uint8_t* bytes, size_t size, Usage usage) : ByteArray(bytes, size, size, usage) {
	}

	ByteArray::ByteArray(uint8_t* bytes, size_t capacity, size_t length, Usage usage) :
		_endian(std::endian::native),
		_needReverse(false),
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

	int64_t ByteArray::readInt(uint8_t numBytes) {
		switch (numBytes) {
		case 1:
			return read<int8_t>();
		case 2:
			return read<uint16_t>();
		case 3:
		{
			int32_t v = 0;
			_read((uint8_t*)& v, 3);
			return v > intMax<24>() ? v - uintMax<24>() - 1 : v;
		}
		case 4:
			return read<int32_t>();
		case 5:
		{
			int64_t v = 0;
			_read((uint8_t*)&v, 5);
			return v > intMax<40>() ? v - uintMax<40>() - 1 : v;
		}
		case 6:
		{
			int64_t v = 0;
			_read((uint8_t*)&v, 6);
			return v > intMax<48>() ? v - uintMax<48>() - 1 : v;
		}
		case 7:
		{
			int64_t v = 0;
			_read((uint8_t*)&v, 7);
			return v > intMax<56>() ? v - uintMax<56>() - 1 : v;
		}
		default:
			return read<int64_t>();
		}
	}

	uint64_t ByteArray::readUInt(uint8_t numBytes) {
		switch (numBytes) {
		case 1:
			return read<uint8_t>();
		case 2:
			return read<uint16_t>();
		case 3:
		{
			uint32_t v = 0;
			_read((uint8_t*)&v, 3);
			return v;
		}
		case 4:
			return read<uint32_t>();
		case 5:
		case 6:
		case 7:
		{
			uint64_t v = 0;
			_read((uint8_t*)&v, numBytes);
			return v;
		}
		default:
			return read<uint64_t>();
		}
	}

	void ByteArray::writeInt(uint8_t numBytes, int64_t value) {
		switch (numBytes) {
		case 1:
			write<int8_t>(value);
			break;
		case 2:
			write<int16_t>(value);
			break;
		case 3:
		{
			if (value < 0) value = uintMax<24>() + 1 + value;
			_write((uint8_t*)&value, 3);

			break;
		}
		case 4:
			write<int32_t>(value);
			break;
		case 5:
		{
			if (value < 0) value = uintMax<40>() + 1 + value;
			_write((uint8_t*)&value, 5);

			break;
		}
		case 6:
		{
			if (value < 0) value = uintMax<48>() + 1 + value;
			_write((uint8_t*)&value, 6);

			break;
		}
		case 7:
		{
			if (value < 0) value = uintMax<56>() + 1 + value;
			_write((uint8_t*)&value, 7);

			break;
		}
		case 8:
			write<int64_t>(value);
			break;
		default:
			break;
		}
	}

	uint64_t ByteArray::readDynamicUInt64() {
		uint64_t rst = 0;
		uint32_t bits = 0;
		while (_position < _length) {
			uint64_t val = _data[_position++];
			auto hasNext = (val & 0x80) != 0;
			val &= 0x7F;
			val <<= bits;
			rst |= val;

			if (hasNext) {
				bits += 7;
			} else {
				break;
			}
		};

		return rst;
	}

	void ByteArray::writeDynamicUInt64(uint64_t value) {
		value &= 0xFFFFFFFFFFFFFFFui64;
		do {
			uint8_t val = value & 0x7F;
			value >>= 7;
			if (value) {
				val |= 0x80;
				write<uint8_t>(val);
			} else {
				write<uint8_t>(val);
				break;
			}
		} while (true);
	}

	size_t ByteArray::readBytes(uint8_t* bytes, size_t length) {
		size_t len = _length - _position;
		if (length > len) length = len;

		memmove(bytes, _data + _position, length);
		_position += length;

		return length;
	}

	size_t ByteArray::readBytes(ByteArray& ba, size_t offset, size_t length) {
		size_t len = _length - _position;
		if (length > len) length = len;

		auto pos = ba.getPosition();
		ba.setPosition(offset);
		ba.writeBytes(_data + _position, length);
		ba.setPosition(pos);
		_position += length;

		return length;
	}

	size_t ByteArray::writeBytes(const ByteArray& ba, size_t offset, size_t length) {
		if (!length) return 0;
		auto len = ba.getLength();
		if (len <= offset) return 0;

		len -= offset;
		if (length > len) length = len;
		auto baBytes = ba.getSource();

		_checkLength(length);

		memmove(_data + _position, baBytes + offset, length);
		_position += length;
		return length;
	}

	std::tuple<size_t, size_t, size_t> ByteArray::readString(size_t begin, size_t size, bool chechBOM) const {
		if (chechBOM) begin += _bomOffset(begin);
		if (_length <= begin) return std::make_tuple(begin, 0, begin);

		if (auto len = _length - begin; size > len) size = len;
		if (!size) return std::make_tuple(begin, 0, begin);

		for (size_t i = begin, n = begin + size; i < n; ++i) {
			if (!_data[i]) {
				auto s = i - begin;
				return std::make_tuple(begin, s, begin + s + 1);
			}
		}

		return std::make_tuple(begin, size, _length);
	}

	void ByteArray::writeString(const char* str, size_t size) {
		if (str) {
			for (size_t i = 0; i < size; ++i) {
				if (str[i] == '\0') {
					size = i;

					break;
				}
			}
		} else {
			size = 0;
		}

		_checkLength(size + 1);

		memcpy(_data + _position, str, size);
		_position += size;
		_data[_position++] = '\0';
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