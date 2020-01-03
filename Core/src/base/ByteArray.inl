namespace aurora {
	inline ByteArray::operator bool() const {
		return _length > 0;
	}

	inline void ByteArray::clear() {
		_position = 0;
		_length = 0;
	}

	inline void ByteArray::detach() {
		if (_usage == Usage::SHARED) _resize(_capacity);
	}

	inline bool ByteArray::isValid() const {
		return _data != nullptr;
	}

	inline std::endian ByteArray::getEndian() const {
		return _endian;
	}

	inline void ByteArray::setEndian(std::endian endian) {
		_endian = endian;
		_needReverse = _endian != std::endian::native;
	}

	inline uint8_t* ByteArray::getSource() {
		return _data;
	}

	inline const uint8_t* ByteArray::getSource() const {
		return _data;
	}

	inline ByteArray ByteArray::slice(size_t start, size_t length, Usage usage) const {
		if (start >= _length) {
			return std::move(ByteArray(nullptr, 0, usage));
		} else {
			return std::move(ByteArray(_data + start, std::min<size_t>(length, _length - start), usage));
		}
	}

	inline ByteArray ByteArray::slice(size_t length, Usage usage) const {
		return std::move(ByteArray(_data + _position, std::min<size_t>(length, _length - _position), usage));
	}

	inline ByteArray ByteArray::slice(Usage usage) const {
		return std::move(ByteArray(_data + _position, _length - _position, usage));
	}

	inline size_t ByteArray::getCapacity() const {
		return _capacity;
	}

	inline void ByteArray::setCapacity(size_t capacity) {
		if (_capacity != capacity) {
			_resize(capacity);
			if (_length > _capacity) {
				_length = _capacity;
				if (_position > _length) _position = _length;
			}
		}
	}

	inline size_t ByteArray::getLength() const {
		return _length;
	}

	inline void ByteArray::setLength(size_t len) {
		if (len > _capacity) _dilatation(len);

		_length = len;
		if (_position > _length) _position = _length;
	}

	inline size_t ByteArray::getPosition() const {
		return _position;
	}

	inline void ByteArray::setPosition(size_t pos) {
		_position = pos > _length ? _length : pos;
	}

	inline void ByteArray::seekBegin() {
		_position = 0;
	}

	inline void ByteArray::seekEnd() {
		_position = _length;
	}

	inline size_t ByteArray::getBytesAvailable() const {
		return _length - _position;
	}

	inline bool ByteArray::isEqual(const ByteArray& data1, const ByteArray& data2) {
		return data1.getLength() == data2.getLength() ? isEqual(data1.getSource(), data2.getSource(), data1.getLength()) : false;
	}

	inline void ByteArray::_read(uint8_t* p, uint32_t len) {
		if (_position + len > _length) {
			_position = _length;
		} else {
			if (_needReverse) {
				for (int8_t i = len - 1; i >= 0; --i) p[i] = _data[_position++];
			} else {
				memmove(p, _data + _position, len);
				_position += len;
			}
		}
	}

	inline void ByteArray::_write(const uint8_t* p, uint32_t len) {
		_checkLength(len);

		if (_needReverse) {
			for (int32_t i = len - 1; i >= 0; --i) _data[_position++] = p[i];
		} else {
			memmove(_data + _position, p, len);
			_position += len;
		}
	}

	inline void ByteArray::_dilatation(size_t size) {
		_resize(size > 1 ? size + (size >> 1) : size + 1);
	}

	inline void ByteArray::_checkLength(size_t len) {
		len += _position;
		if (len > _length) {
			_length = len;
			if (_length > _capacity) _dilatation(len);
		}
	}

	inline uint8_t ByteArray::_bomOffset(size_t pos) const {
		if (pos + 3 > _length) return 0;
		return (uint8_t)_data[pos] == 0xEF && (uint8_t)_data[pos + 1] == 0xBB && (uint8_t)_data[pos + 2] == 0xBF ? 3 : 0;
	}
}