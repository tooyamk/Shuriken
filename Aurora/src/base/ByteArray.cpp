#include "ByteArray.h"

namespace aurora {
	ByteArray::ByteArray(ByteArray&& bytes) :
		_endian(bytes._endian),
		_needReverse(bytes._needReverse),
		_capacity(bytes._capacity),
		_length(bytes._length),
		_position(bytes._position),
		_mode(bytes._mode),
		_data(bytes._data) {
		bytes._data = nullptr;
	}

	ByteArray::ByteArray(ui32 capacity, ui32 length) :
		_endian(NATIVE_ENDIAN),
		_needReverse(false),
		_capacity(capacity),
		_length(length > capacity ? capacity : length),
		_position(0),
		_mode(Mode::MEM),
		_data(capacity > 0 ? new i8[capacity] : nullptr) {
	}

	ByteArray::ByteArray(i8* bytes, ui32 size, ExtMemMode extMode) :
		_endian(NATIVE_ENDIAN),
		_needReverse(false),
		_capacity(size),
		_length(size),
		_position(0) {
		if (extMode == ExtMemMode::EXT) {
			this->_data = bytes;
			_mode = Mode::EXT;
		} else if (extMode == ExtMemMode::COPY) {
			_data = new i8[size];
			memcpy(_data, bytes, size);
			_mode = Mode::MEM;
		} else {
			this->_data = (i8*)bytes;
			_mode = Mode::MEM;
		}
	}

	ByteArray::ByteArray(i8* bytes, ui32 length, ui32 capacity, ExtMemMode extMode) :
		_endian(NATIVE_ENDIAN),
		_needReverse(false),
		_capacity(capacity),
		_length(length),
		_position(0) {
		if (extMode == ExtMemMode::EXT) {
			this->_data = bytes;
			_mode = Mode::EXT;
		} else if (extMode == ExtMemMode::COPY) {
			_data = new i8[capacity];
			memcpy(_data, bytes, length);
			_mode = Mode::MEM;
		} else {
			this->_data = (i8*)bytes;
			_mode = Mode::MEM;
		}
	}

	ByteArray& ByteArray::operator=(ByteArray&& value) {
		dispose(true);

		_endian = value._endian;
		_needReverse = value._needReverse;
		_capacity = value._capacity;
		_length = value._length;
		_position = value._position;
		_mode = value._mode;
		_data = value._data;

		value._data = nullptr;

		return *this;
	}

	ByteArray::~ByteArray() {
		dispose();
	}

	void ByteArray::dispose(bool free) {
		if (_data) {
			if (free && _mode != Mode::EXT) delete[] _data;
			_data = nullptr;
		}
	}

	void ByteArray::_resize(ui32 len) {
		i8* newBytes = len > 0 ? new i8[len] : nullptr;

		memcpy(newBytes, _data, len > _capacity ? _capacity : len);
		_capacity = len;

		if (_mode == Mode::EXT) {
			_mode = Mode::MEM;
		} else {
			if (_data != nullptr) delete[] _data;
		}

		_data = newBytes;
	}

	i64 ByteArray::readInt(ui8 numBytes) {
		switch (numBytes) {
		case 1:
			return _read<i8>();
		case 2:
			return _read<i16>();
		case 3:
		{
			i32 v;
			_read((i8*)&v, 3);
			return v > INT24_MAX ? v - INT24 : v;
		}
		case 4:
			return _read<i32>();
		case 5:
		{
			i64 v;
			_read((i8*)&v, 5);
			return v > INT40_MAX ? v - INT40 : v;
		}
		case 6:
		{
			i64 v;
			_read((i8*)&v, 6);
			return v > INT48_MAX ? v - INT48 : v;
		}
		case 7:
		{
			i64 v;
			_read((i8*)&v, 7);
			return v > INT56_MAX ? v - INT56 : v;
		}
		default:
			return _read<i64>();
		}
	}

	ui64 ByteArray::readUInt(ui8 numBytes) {
		switch (numBytes) {
		case 1:
			return _read<ui8>();
		case 2:
			return _read<ui16>();
		case 3:
		{
			ui32 v;
			_read((i8*)&v, 3);
			return v;
		}
		case 4:
			return _read<ui32>();
		case 5:
		case 6:
		case 7:
		{
			ui64 v;
			_read((i8*)&v, numBytes);
			return v;
		}
		default:
			return _read<ui64>();
		}
	}

	void ByteArray::writeInt(ui8 numBytes, i64 value) {
		switch (numBytes) {
		case 1:
		case 2:
		case 4:
			_write((i8*)&value, numBytes);
			break;
		case 3:
		{
			if (value < 0) value = INT24 + value;
			_write((i8*)&value, 3);

			break;
		}
		case 5:
		{
			if (value < 0) value = INT40 + value;
			_write((i8*)&value, 5);

			break;
		}
		case 6:
		{
			if (value < 0) value = INT48 + value;
			_write((i8*)&value, 6);

			break;
		}
		case 7:
		{
			if (value < 0) value = INT56 + value;
			_write((i8*)&value, 7);

			break;
		}
		default:
			_write((i8*)&value, 8);
			break;
		}
	}

	ui32 ByteArray::readBytes(i8* bytes, ui32 offset, ui32 length) {
		ui32 len = _length - _position;
		if (length > len) length = len;

		memcpy(bytes + offset, _data + _position, length);
		_position += length;

		return length;
	}

	ui32 ByteArray::readBytes(ByteArray& ba, ui32 offset, ui32 length) {
		ui32 len = _length - _position;
		if (length > len) length = len;

		ui32 pos = ba.getPosition();
		ba.setPosition(offset);
		ba.writeBytes((i8*)_data, _position, length);
		ba.setPosition(pos);
		_position += length;

		return length;
	}

	ui32 ByteArray::writeBytes(const ByteArray& ba, ui32 offset, ui32 length) {
		if (!length) return 0;
		auto len = ba.getLength();
		if (len <= offset) return 0;

		len -= offset;
		if (length > len) length = len;
		const i8* baBytes = ba.getBytes();

		_checkLength(length);

		memmove(_data + _position, baBytes + offset, length);
		_position += length;
		return length;
	}

	ui32 ByteArray::readStringLength(ui32 begin, ui32 size, bool chechBOM) const {
		begin += _bomOffset(begin);
		if (_length <= begin) return 0;

		auto len = _length - begin;
		if (size > len) size = len;
		if (!size) return 0;

		for (ui32 i = begin, n = begin + size; i < n; ++i) {
			if (!_data[i]) return i - begin;
		}
		return size;
	}

	void ByteArray::writeString(const i8* str, ui32 size) {
		for (ui32 i = 0; i < size; ++i) {
			if (str[i] == '\0') {
				size = i;

				break;
			}
		}

		_checkLength(size + 1);

		memcpy(_data + _position, str, size);
		_position += size;
		_data[_position++] = '\0';
	}

	void ByteArray::popFront(ui32 len) {
		if (_length <= len) {
			_length = 0;
			_position = 0;
		} else {
			_length -= len;
			for (ui32 i = 0; i < _length; ++i) _data[i] = _data[len + i];

			if (_position <= len) {
				_position = 0;
			} else {
				_position -= len;
			}
		}
	}

	void ByteArray::popBack(ui32 len) {
		if (_length < len) {
			_length = 0;
			_position = 0;
		} else {
			_length -= len;
			if (_position > _length) _position = _length;
		}
	}

	void ByteArray::insert(ui32 len) {
		if (len > 0) {
			ui32 oldLen = _length;

			_checkLength(len);

			for (i64 i = oldLen - 1; i >= _position; --i) _data[i + len] = _data[i];

			_position += len;
		}
	}

	bool ByteArray::isEqual(const i8* data1, ui32 data1Len, const i8* data2, ui32 data2Len) {
		if (data1Len != data2Len) return false;

		for (ui32 i = 0; i < data1Len; ++i) {
			if (data1[i] != data2[i]) return false;
		}

		return true;
	}
}