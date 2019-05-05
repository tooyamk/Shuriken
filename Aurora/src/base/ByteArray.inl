namespace aurora {
	inline ByteArray::operator bool() const {
		return _length > 0;
	}

	inline void ByteArray::clear() {
		_position = 0;
		_length = 0;
	}

	inline void ByteArray::detach() {
		if (_mode == Mode::EXT) _resize(_capacity);
	}

	inline bool ByteArray::isValid() const {
		return _data != nullptr;
	}

	inline ByteArray::Endian ByteArray::getEndian() const {
		return _endian;
	}

	inline void ByteArray::setEndian(ByteArray::Endian endian) {
		_endian = endian;
		_needReverse = _endian != SYS_ENDIAN;
	}

	inline const i8* ByteArray::getBytes() const {
		return _data;
	}

	inline ByteArray ByteArray::slice(ui32 start, ui32 length) const {
		if (start >= _length) {
			return std::move(ByteArray(nullptr, 0, ExtMemMode::EXT));
		} else {
			return std::move(ByteArray(_data + start, std::min<ui32>(length, _length - start), ExtMemMode::EXT));
		}
	}

	inline ui32 ByteArray::getCapacity() const {
		return _capacity;
	}

	inline void ByteArray::setCapacity(ui32 capacity) {
		if (_capacity != capacity) _resize(capacity);
	}

	inline ui32 ByteArray::getLength() const {
		return _length;
	}

	inline void ByteArray::setLength(ui32 len) {
		if (len > _capacity) _dilatation(len);

		_length = len;
		if (_position > _length) _position = _length;
	}

	inline ui32 ByteArray::getPosition() const {
		return _position;
	}

	inline void ByteArray::setPosition(ui32 pos) {
		if (pos > _length) pos = _length;
		_position = pos;
	}

	inline void ByteArray::seekBegin() {
		_position = 0;
	}

	inline void ByteArray::seekEnd() {
		_position = _length;
	}

	inline ui32 ByteArray::getBytesAvailable() const {
		return _length - _position;
	}

	inline i8 ByteArray::readInt8() {
		return _read<i8>();
	}

	inline ui8 ByteArray::readUInt8() {
		return _read<ui8>();
	}

	inline void ByteArray::writeInt8(char value) {
		_write((i8*)&value, 1);
	}

	inline void ByteArray::writeUInt8(ui8 value) {
		_write((i8*)&value, 1);
	}

	inline i16 ByteArray::readInt16() {
		return _read<i16>();
	}

	inline void ByteArray::writeInt16(i16 value) {
		_write((i8*)&value, 2);
	}

	inline ui16 ByteArray::readUInt16() {
		return _read<ui16>();
	}

	inline void ByteArray::writeUInt16(ui16 value) {
		_write((i8*)&value, 2);
	}

	inline i32 ByteArray::readInt32() {
		return _read<i32>();
	}

	inline void ByteArray::writeInt32(i32 value) {
		_write((i8*)&value, 4);
	}

	inline ui32 ByteArray::readUInt32() {
		return _read<ui32>();
	}

	inline void ByteArray::writeUInt32(ui32 value) {
		_write((i8*)&value, 4);
	}

	inline f32 ByteArray::readFloat32() {
		return _read<f32>();
	}

	inline void ByteArray::writeFloat32(f32 value) {
		_write((i8*)&value, 4);
	}

	inline f64 ByteArray::readFloat64() {
		return _read<f64>();
	}

	inline void ByteArray::writeFloat64(const f64& value) {
		_write((i8*)&value, 8);
	}

	inline i64 ByteArray::readInt64() {
		return _read<i64>();
	}

	inline void ByteArray::writeInt64(const i64& value) {
		_write((i8*)&value, 8);
	}

	inline ui64 ByteArray::readUInt64() {
		return _read<ui64>();
	}

	inline void ByteArray::writeUInt64(const ui64& value) {
		_write((i8*)&value, 8);
	}

	inline void ByteArray::writeUInt(ui8 numBytes, ui64 value) {
		_write((i8*)&value, numBytes > 8 ? 8 : numBytes);
	}

	inline void ByteArray::writeBytes(const i8* bytes, ui32 offset, ui32 length) {
		if (length > 0) {
			_checkLength(length);

			memcpy(_data + _position, bytes + offset, length);
			_position += length;
		}
	}

	inline ui32 ByteArray::readStringLength(ui32 size, bool chechBOM) const {
		return readStringLength(_position, size, chechBOM);
	}

	inline std::string ByteArray::readString(bool chechBOM) {
		auto len = readStringLength(UINT_MAX, chechBOM);
		auto begin = _position;
		_position += len;
		return std::move(std::string(_data + begin, len));
	}

	inline std::string_view ByteArray::readStringView(bool chechBOM) {
		auto len = readStringLength(UINT_MAX, chechBOM);
		auto begin = _position;
		_position += len;
		return std::move(std::string_view(_data + begin, len));
	}

	inline void ByteArray::writeString(const std::string& str) {
		writeString(str.c_str(), str.size());
	}

	inline bool ByteArray::readBool() {
		return readInt8() != 0;
	}

	inline void ByteArray::writeBool(bool b) {
		writeInt8(b ? 1 : 0);
	}

	inline void ByteArray::readTwoUInt12(ui16& value1, ui16& value2) {
		ui8 v1 = readUInt8();
		ui8 v2 = readUInt8();
		ui8 v3 = readUInt8();

		value1 = (v1 << 4) | ((v2 >> 4) & 0xF);
		value2 = ((v2 & 0xF) << 8) | v3;
	}

	inline void ByteArray::writeTwoUInt12(ui16 value1, ui16 value2) {
		writeUInt8((value1 >> 4) & 0xFF);
		writeUInt8(((value1 & 0xF) << 4) | ((value2 >> 8) & 0xF));
		writeUInt8(value2 & 0xFF);
	}

	inline void ByteArray::readTwoInt12(short& value1, short& value2) {
		ui16 v1, v2;
		readTwoUInt12(v1, v2);

		value1 = v1 > INT12_MAX ? v1 - INT12 : v1;
		value2 = v2 > INT12_MAX ? v2 - INT12 : v2;
	}

	inline void ByteArray::writeTwoInt12(short value1, short value2) {
		writeTwoUInt12(value1 < 0 ? INT12 + value1 : value1, value2 < 0 ? INT12 + value2 : value2);
	}

	inline bool ByteArray::isEqual(const ByteArray& data1, const ByteArray& data2) {
		return isEqual(data1.getBytes(), data1.getLength(), data2.getBytes(), data2.getLength());
	}

	inline void ByteArray::_read(i8* p, ui32 len) {
		if (_position + len > _length) memset(p, 0, len);

		if (_needReverse) {
			for (char i = len - 1; i >= 0; --i) p[i] = _data[_position++];
		} else {
			memcpy(p, _data + _position, len);
			_position += len;
		}
	}

	inline void ByteArray::_write(const i8* p, ui32 len) {
		_checkLength(len);

		if (_needReverse) {
			for (int i = len - 1; i >= 0; --i) _data[_position++] = p[i];
		} else {
			memcpy(_data + _position, p, len);
			_position += len;
		}
	}

	inline void ByteArray::_dilatation(ui32 size) {
		_resize(size > 1 ? size + (size >> 1) : size + 1);
	}

	inline void ByteArray::_checkLength(ui32 len) {
		len += _position;
		if (len > _length) {
			_length = len;
			if (_length > _capacity) _dilatation(len);
		}
	}

	inline ui8 ByteArray::_bomOffset(ui32 pos) const {
		if (pos + 3 > _length) return 0;
		return (ui8)_data[pos] == 0xEF && (ui8)_data[pos + 1] == 0xBB && (ui8)_data[pos + 2] == 0xBF ? 3 : 0;
	}
}