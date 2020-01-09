#pragma once

#include "base/Global.h"

namespace aurora::hash {
	class  xxHash {
	public:
		AE_DECLA_CANNOT_INSTANTIATE(xxHash);

		template<size_t Bits, std::endian DataEndian>
		static uint_t<Bits> AE_CALL calc(const uint8_t* data, size_t len, uint_t<Bits> seed) {
			static_assert(Bits == 32 || Bits == 64, "only support 32, 64 bits mode");

			using hash_t = uint_t<Bits>;

			constexpr size_t OFFSET = Bits >> 3;
			constexpr size_t HALF_BITS = Bits >> 1;

			auto& prime = Prime<Bits>::VALUE;

			auto dataEnd = data + len;
			hash_t ret;

			if (len >= HALF_BITS) {
				hash_t v1 = seed + prime[0] + prime[1];
				hash_t v2 = seed + prime[1];
				hash_t v3 = seed;
				hash_t v4 = seed - prime[0];

				do {
					v1 = _round<Bits>(v1, _readUInt<Bits, DataEndian>(data)); data += OFFSET;
					v2 = _round<Bits>(v2, _readUInt<Bits, DataEndian>(data)); data += OFFSET;
					v3 = _round<Bits>(v3, _readUInt<Bits, DataEndian>(data)); data += OFFSET;
					v4 = _round<Bits>(v4, _readUInt<Bits, DataEndian>(data)); data += OFFSET;
				} while (data <= (dataEnd - HALF_BITS));

				ret = rotl(v1, 1) + rotl(v2, 7) + rotl(v3, 12) + rotl(v4, 18);

				_subMergeRound<Bits>(ret, v1, v2, v3, v4);
			} else { 
				ret = seed + prime[4];
			}

			return _subEnding<Bits, DataEndian>(ret + (hash_t)len, data, dataEnd);
		}

	private:
		template<size_t Bits>
		struct Prime { inline static constexpr uint_t<Bits> VALUE[] = { 0, 0, 0, 0 }; };
		template<>
		struct Prime<32> { inline static constexpr uint_t<32> VALUE[] = { 2654435761ui32, 2246822519ui32, 3266489917ui32, 668265263ui32, 374761393ui32 }; };
		template<>
		struct Prime<64> { inline static constexpr uint_t<64> VALUE[] = { 11400714785074694791ui64, 14029467366897019727ui64, 1609587929392839161ui64, 9650029242287828579ui64, 2870177450012600261ui64 }; };

		template<size_t Bits, std::endian DataEndian>
		inline static uint_t<Bits> AE_CALL _readUInt(const uint8_t* data) {
			if constexpr (DataEndian == std::endian::native) {
				return *(uint_t<Bits>*)data;
			} else {
				return byteswap<Bits>(data);
			}
		}

		template<size_t Bits>
		inline static uint_t<Bits> AE_CALL _round(uint_t<Bits> seed, uint_t<Bits> x) {
			constexpr size_t SHIFT = Bits == 32 ? 13 : 31;

			seed += x * Prime<Bits>::VALUE[1];
			seed = rotl(seed, SHIFT);
			seed *= Prime<Bits>::VALUE[0];
			return seed;
		}

		template<size_t Bits>
		inline static void AE_CALL _subMergeRound(uint_t<Bits>& ret, uint_t<Bits> v1, uint_t<Bits> v2, uint_t<Bits> v3, uint_t<Bits> v4) {
			if constexpr (Bits == 64) {
				ret = _mergeRound<Bits>(ret, v1);
				ret = _mergeRound<Bits>(ret, v2);
				ret = _mergeRound<Bits>(ret, v3);
				ret = _mergeRound<Bits>(ret, v4);
			}
		}

		template<size_t Bits>
		inline static uint_t<Bits> AE_CALL _mergeRound(uint_t<Bits> acc, uint_t<Bits> val) {
			if constexpr (Bits == 64) {
				val = _round<Bits>(0, val);
				acc ^= val;
				acc = acc * Prime<Bits>::VALUE[0] + Prime<Bits>::VALUE[3];
			}
			return acc;
		}

		template<size_t Bits, std::endian DataEndian>
		inline static uint_t<Bits> AE_CALL _subEnding(uint_t<Bits> ret, const uint8_t* data, const uint8_t* dataEnd) {
			if constexpr (Bits == 32) {
				while ((data + 4) <= dataEnd) {
					ret += _readUInt<Bits, DataEndian>(data) * Prime<Bits>::VALUE[2];
					ret = rotl(ret, 17) * Prime<Bits>::VALUE[3];
					data += 4;
				}

				while (data < dataEnd) {
					ret += (*data) * Prime<Bits>::VALUE[4];
					ret = rotl(ret, 11) * Prime<Bits>::VALUE[0];
					++data;
				}

				ret ^= ret >> 15;
				ret *= Prime<Bits>::VALUE[1];
				ret ^= ret >> 13;
				ret *= Prime<Bits>::VALUE[2];
				ret ^= ret >> 16;

				return ret;
			} else if constexpr (Bits == 64) {
				while (data + 8 <= dataEnd) {
					ret ^= _round<Bits>(0, _readUInt<Bits, DataEndian>(data));
					ret = rotl(ret, 27) * Prime<Bits>::VALUE[0] + Prime<Bits>::VALUE[3];
					data += 8;
				}

				if (data + 4 <= dataEnd) {
					ret ^= (uint_t<Bits>)_readUInt<Bits / 2, DataEndian>(data) * Prime<Bits>::VALUE[0];
					ret = rotl(ret, 23) * Prime<Bits>::VALUE[1] + Prime<Bits>::VALUE[2];
					data += 4;
				}

				while (data < dataEnd) {
					ret ^= (*data) * Prime<Bits>::VALUE[4];
					ret = rotl(ret, 11) * Prime<Bits>::VALUE[0];
					++data;
				}

				ret ^= ret >> 33;
				ret *= Prime<Bits>::VALUE[1];
				ret ^= ret >> 29;
				ret *= Prime<Bits>::VALUE[2];
				ret ^= ret >> 32;

				return ret;
			} else {
				static_assert(false, "not support bits mode");
			}
		}
	};
}