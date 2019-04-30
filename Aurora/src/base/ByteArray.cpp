#include "ByteArray.h"

namespace aurora {
	ByteArray::ByteArray(ByteArray&& bytes) :
		_endian(bytes._endian),
		_needReverse(bytes._needReverse),
		_capacity(bytes._capacity),
		_length(bytes._length),
		_position(bytes._position),
		_mode(bytes._mode),
		_bytes(bytes._bytes) {
		bytes._bytes = nullptr;
	}

	ByteArray::ByteArray(ui32 capacity, ui32 length) :
		_capacity(capacity),
		_length(length > capacity ? capacity : length),
		_position(0),
		_mode(Mode::MEM),
		_bytes(capacity > 0 ? new i8[capacity] : nullptr) {
		setEndian(ByteArray::SYS_ENDIAN);
	}

	ByteArray::ByteArray(i8* bytes, ui32 size, ExtMemMode extMode) :
		_capacity(size),
		_length(size),
		_position(0) {
		setEndian(ByteArray::SYS_ENDIAN);
		if (extMode == ExtMemMode::EXT) {
			this->_bytes = bytes;
			_mode = Mode::EXT;
		} else if (extMode == ExtMemMode::COPY) {
			_bytes = new i8[size];
			memcpy(_bytes, bytes, size);
			_mode = Mode::MEM;
		} else {
			this->_bytes = (i8*)bytes;
			_mode = Mode::MEM;
		}
	}

	ByteArray::ByteArray(i8* bytes, ui32 length, ui32 capacity, ExtMemMode extMode) :
		_capacity(capacity),
		_length(length),
		_position(0) {
		setEndian(ByteArray::SYS_ENDIAN);
		if (extMode == ExtMemMode::EXT) {
			this->_bytes = bytes;
			_mode = Mode::EXT;
		} else if (extMode == ExtMemMode::COPY) {
			_bytes = new i8[capacity];
			memcpy(_bytes, bytes, length);
			_mode = Mode::MEM;
		} else {
			this->_bytes = (i8*)bytes;
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
		_bytes = value._bytes;

		value._bytes = nullptr;

		return *this;
	}

	ByteArray::~ByteArray() {
		dispose();
	}

	const ByteArray::Endian ByteArray::SYS_ENDIAN = ((ui8*)(&TEST_ENDIAN_VALUE))[0] ? Endian::LITTLE : Endian::BIG;

	void ByteArray::dispose(bool free) {
		if (_bytes) {
			if (free && _mode != Mode::EXT) delete[] _bytes;
			_bytes = nullptr;
		}
	}

	void ByteArray::_resize(ui32 len) {
		i8* newBytes = len > 0 ? new i8[len] : nullptr;

		memcpy(newBytes, _bytes, len > _capacity ? _capacity : len);
		_capacity = len;

		if (_mode == Mode::EXT) {
			_mode = Mode::MEM;
		} else {
			if (_bytes != nullptr) delete[] _bytes;
		}

		_bytes = newBytes;
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
		if (length == 0) {
			if (_length <= _position) return 0;

			length = _length - _position;
		} else {
			ui32 len = _length - _position;
			if (length > len) length = len;
		}

		memcpy(bytes + offset, _bytes + _position, length);
		_position += length;

		return length;
	}

	ui32 ByteArray::readBytes(ByteArray& ba, ui32 offset, ui32 length) {
		if (length == 0) {
			if (_length <= _position) return 0;

			length = _length - _position;
		} else {
			ui32 len = _length - _position;
			if (length > len) length = len;
		}

		ui32 pos = ba.getPosition();
		ba.setPosition(offset);
		ba.writeBytes((i8*)_bytes, _position, length);
		ba.setPosition(pos);
		_position += length;

		return length;
	}

	void ByteArray::writeBytes(const ByteArray& ba, ui32 offset, ui32 length) {
		if (length == 0) {
			if (ba.getLength() <= offset) return;

			length = ba.getLength() - offset;
		}

		const i8* baBytes = ba.getBytes();

		_checkLength(length);

		memmove(_bytes + _position, baBytes + offset, length);
		_position += length;
	}

	const i8* ByteArray::readCString(bool chechBOM, ui32* size) {
		bool find = false;
		ui32 pos = 0;

		for (ui32 i = _position; i < _length; ++i) {
			if (_bytes[i] == '\0') {
				pos = i;
				find = true;
				break;
			}
		}

		if (find) {
			ui32 len = pos - _position;

			if (chechBOM) {
				ui8 offset = _bomOffset(_position);
				len -= offset;
				_position += offset;
			}

			ui32 offset = _position;
			_position += len + 1;

			if (size != nullptr) *size = len;
			return _bytes + offset;
		} else {
			if (size != nullptr) *size = 0;
			return nullptr;
		}
	}

	std::string ByteArray::readString(ui32 start, ui32 size, bool chechBOM) {
		if (chechBOM) start += _bomOffset(start);

		if (_length == 0 || size == 0) {
			return "";
		} else {
			if (start >= _length) return "";

			ui32 checkSize = _length - start;
			if (checkSize < size) size = checkSize;
		}

		for (ui32 i = 0; i < size; ++i) {
			if (_bytes[start + i] == '\0') {
				size = i;
				break;
			}
		}

		return std::move(std::string(_bytes + start, size));
	}

	void ByteArray::writeString(const i8* str, ui32 size) {
		for (ui32 i = 0; i < size; ++i) {
			if (str[i] == '\0') {
				size = i;

				break;
			}
		}

		_checkLength(size + 1);

		memcpy(_bytes + _position, str, size);
		_position += size;
		_bytes[_position++] = '\0';
	}

	void ByteArray::popFront(ui32 len) {
		if (_length <= len) {
			_length = 0;
			_position = 0;
		} else {
			_length -= len;
			for (ui32 i = 0; i < _length; ++i) _bytes[i] = _bytes[len + i];

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

			for (i64 i = oldLen - 1; i >= _position; --i) _bytes[i + len] = _bytes[i];

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