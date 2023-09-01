#pragma once

#include "srk/Bit.h"
#include <array>

namespace srk::hash {
	template<size_t Bits>
	requires (Bits == 32 || Bits == 64)
	class xxHash {
	public:
		using hash_t = uint_t<Bits>;

		void begin(uint_t<Bits> seed) {
			_seed = seed;
			_bufferCount = 0;
			_length = 0;

			auto& prime = PRIME_VALUE;
			_hash[0] = _seed + prime[0] + prime[1];
			_hash[1] = _seed + prime[1];
			_hash[2] = _seed;
			_hash[3] = _seed - prime[0];
		}

		template<std::endian DataEndian = std::endian::native>
		void update(const void* data, size_t len) {
			_length += len;

			auto src = (const uint8_t*)data;

			while (len > 0) {
				size_t cpyLen = HALF_BITS - _bufferCount;
				if (cpyLen > len) cpyLen = len;
				len -= cpyLen;
				memcpy(_buffer + _bufferCount, src, cpyLen);
				src += cpyLen;
				_bufferCount += cpyLen;

				if (_bufferCount == HALF_BITS) {
					auto dst = (const uint8_t*)_buffer;
					_hash[0] = _round(_hash[0], _readUInt<Bits, DataEndian, false>(dst)); dst += OFFSET;
					_hash[1] = _round(_hash[1], _readUInt<Bits, DataEndian, false>(dst)); dst += OFFSET;
					_hash[2] = _round(_hash[2], _readUInt<Bits, DataEndian, false>(dst)); dst += OFFSET;
					_hash[3] = _round(_hash[3], _readUInt<Bits, DataEndian, false>(dst)); dst += OFFSET;

					_bufferCount = 0;
				}
			};
		}

		template<std::endian DataEndian = std::endian::native>
		uint_t<Bits> digest() {
			hash_t ret;
			if (_length >= HALF_BITS) {
				ret = std::rotl(_hash[0], 1) + std::rotl(_hash[1], 7) + std::rotl(_hash[2], 12) + std::rotl(_hash[3], 18);
				_subMergeRound(ret, _hash[0], _hash[1], _hash[2], _hash[3]);
			} else {
				ret = _seed + PRIME_VALUE[4];
			}

			return _subEnding<DataEndian, false>(ret + (hash_t)_length, _buffer, _buffer + _bufferCount);
		}

		template<std::endian DataEndian = std::endian::native, bool AlignedAccess = false>
		static uint_t<Bits> SRK_CALL calc(const void* data, size_t len, uint_t<Bits> seed) {
			auto& prime = PRIME_VALUE;

			auto src = (const uint8_t*)data;
			auto dataEnd = src + len;
			hash_t ret;

			if (len >= HALF_BITS) {
				hash_t v1 = seed + prime[0] + prime[1];
				hash_t v2 = seed + prime[1];
				hash_t v3 = seed;
				hash_t v4 = seed - prime[0];

				do {
					v1 = _round(v1, _readUInt<Bits, DataEndian, AlignedAccess>(src)); src += OFFSET;
					v2 = _round(v2, _readUInt<Bits, DataEndian, AlignedAccess>(src)); src += OFFSET;
					v3 = _round(v3, _readUInt<Bits, DataEndian, AlignedAccess>(src)); src += OFFSET;
					v4 = _round(v4, _readUInt<Bits, DataEndian, AlignedAccess>(src)); src += OFFSET;
				} while (src <= (dataEnd - HALF_BITS));

				ret = std::rotl(v1, 1) + std::rotl(v2, 7) + std::rotl(v3, 12) + std::rotl(v4, 18);
				_subMergeRound(ret, v1, v2, v3, v4);
			} else {
				ret = seed + prime[4];
			}

			return _subEnding<DataEndian, AlignedAccess>(ret + (hash_t)len, src, dataEnd);
		}

	private:
		static constexpr size_t OFFSET = Bits >> 3;
		static constexpr size_t HALF_BITS = Bits >> 1;

		inline static constexpr auto SRK_CALL _createPrimeValue() {
			using T = std::array<uint_t<Bits>, 5>;
			if constexpr (Bits == 32) {
				T val = { 2654435761U, 2246822519U, 3266489917U, 668265263U, 374761393U };
				return val;
			} else {
				T val = { 11400714785074694791ULL, 14029467366897019727ULL, 1609587929392839161ULL, 9650029242287828579ULL, 2870177450012600261ULL };
				return val;
			}
		}

		inline static constexpr auto PRIME_VALUE = _createPrimeValue();

		uint_t<Bits> _seed;
		uint8_t _buffer[HALF_BITS];
		uint8_t _bufferCount;
		hash_t _hash[4];
		size_t _length;

		template<size_t nBits, std::endian DataEndian, bool AlignedAccess>
		inline static uint_t<nBits> SRK_CALL _readUInt(const uint8_t* data) {
			if constexpr (DataEndian == std::endian::native) {
				if constexpr (AlignedAccess) {
					uint_t<nBits> val;
					memcpy(&val, data, sizeof(val));
					return val;
				} else {
					return *(uint_t<nBits>*)data;
				}
			} else {
				return Bit::byteswap<nBits / 8, AlignedAccess>(data);
			}
		}

		inline static uint_t<Bits> SRK_CALL _round(uint_t<Bits> seed, uint_t<Bits> x) {
			constexpr int32_t SHIFT = Bits == 32 ? 13 : 31;

			seed += x * PRIME_VALUE[1];
			seed = std::rotl(seed, SHIFT);
			seed *= PRIME_VALUE[0];
			return seed;
		}

		inline static void SRK_CALL _subMergeRound(uint_t<Bits>& ret, uint_t<Bits> v1, uint_t<Bits> v2, uint_t<Bits> v3, uint_t<Bits> v4) {
			if constexpr (Bits == 64) {
				ret = _mergeRound(ret, v1);
				ret = _mergeRound(ret, v2);
				ret = _mergeRound(ret, v3);
				ret = _mergeRound(ret, v4);
			}
		}

		inline static uint_t<Bits> SRK_CALL _mergeRound(uint_t<Bits> acc, uint_t<Bits> val) {
			if constexpr (Bits == 64) {
				val = _round(0, val);
				acc ^= val;
				acc = acc * PRIME_VALUE[0] + PRIME_VALUE[3];
			}
			return acc;
		}

		template<std::endian DataEndian, bool AlignedAccess>
		static uint_t<Bits> SRK_CALL _subEnding(uint_t<Bits> ret, const uint8_t* data, const uint8_t* dataEnd) {
			if constexpr (Bits == 32) {
				while ((data + 4) <= dataEnd) {
					ret += _readUInt<Bits, DataEndian, AlignedAccess>(data) * PRIME_VALUE[2];
					ret = std::rotl(ret, 17) * PRIME_VALUE[3];
					data += 4;
				}

				while (data < dataEnd) {
					ret += (*data) * PRIME_VALUE[4];
					ret = std::rotl(ret, 11) * PRIME_VALUE[0];
					++data;
				}

				ret ^= ret >> 15;
				ret *= PRIME_VALUE[1];
				ret ^= ret >> 13;
				ret *= PRIME_VALUE[2];
				ret ^= ret >> 16;

				return ret;
			} else if constexpr (Bits == 64) {
				while (data + 8 <= dataEnd) {
					ret ^= _round(0, _readUInt<Bits, DataEndian, AlignedAccess>(data));
					ret = std::rotl(ret, 27) * PRIME_VALUE[0] + PRIME_VALUE[3];
					data += 8;
				}

				if (data + 4 <= dataEnd) {
					ret ^= (uint_t<Bits>)_readUInt<Bits / 2, DataEndian, AlignedAccess>(data) * PRIME_VALUE[0];
					ret = std::rotl(ret, 23) * PRIME_VALUE[1] + PRIME_VALUE[2];
					data += 4;
				}

				while (data < dataEnd) {
					ret ^= (*data) * PRIME_VALUE[4];
					ret = std::rotl(ret, 11) * PRIME_VALUE[0];
					++data;
				}

				ret ^= ret >> 33;
				ret *= PRIME_VALUE[1];
				ret ^= ret >> 29;
				ret *= PRIME_VALUE[2];
				ret ^= ret >> 32;

				return ret;
			} else {
				static_assert(Bits != 32, "not support bits mode");
			}
		}
	};
}