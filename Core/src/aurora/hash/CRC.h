#pragma once

#include "aurora/Global.h"
#include <array>

namespace aurora::hash {
	/**
		Predefined:
		                     InputReflected  ResultReflected  Polynomial          InitialValue        FinalXorValue
		CRC8                 false           false            0x07                0x00                0x00
		CRC8_SAE_J1850       false           false            0x1D                0xFF                0xFF
		CRC8_SAE_J1850_ZERO  false           false            0x1D                0x00                0x00
		CRC8_8H2F            false           false            0x2F                0xFF                0xFF
		CRC8_CDMA2000        false           false            0x9B                0xFF                0x00
		CRC8_DARC            true            true             0x39                0x00                0x00
		CRC8_DVB_S2          false           false            0xD5                0x00                0x00
		CRC8_EBU             true            true             0x1D                0xFF                0x00
		CRC8_ICODE           false           false            0x1D                0xFD                0x00
		CRC8_ITU             false           false            0x07                0x00                0x55
		CRC8_MAXIM           true            true             0x31                0x00                0x00
		CRC8_ROHC            true            true             0x07                0xFF                0x00
		CRC8_WCDMA           true            true             0x9B                0x00                0x00

		CRC16_CCIT_ZERO      false           false            0x1021              0x0000              0x0000
		CRC16_ARC            true            true             0x8005              0x0000              0x0000
		CRC16_AUG_CCITT      false           false            0x1021              0x1D0F              0x0000
		CRC16_BUYPASS        false           false            0x8005              0x0000              0x0000
		CRC16_CCITT_FALSE    false           false            0x1021              0xFFFF              0x0000
		CRC16_CDMA2000       false           false            0xC867              0xFFFF              0x0000
		CRC16_DDS_110        false           false            0x8005              0x800D              0x0000
		CRC16_DECT_R         false           false            0x0589              0x0000              0x0001
		CRC16_DECT_X         false           false            0x0589              0x0000              0x0000
		CRC16_DNP            true            true             0x3D65              0x0000              0xFFFF
		CRC16_EN_13757       false           false            0x3D65              0x0000              0xFFFF
		CRC16_GENIBUS        false           false            0x1021              0xFFFF              0xFFFF
		CRC16_MAXIM          true            true             0x8005              0x0000              0xFFFF
		CRC16_MCRF4XX        true            true             0x1021              0xFFFF              0x0000
		CRC16_RIELLO         true            true             0x1021              0xB2AA              0x0000
		CRC16_T10_DIF        false           false            0x8BB7              0x0000              0x0000
		CRC16_TELEDISK       false           false            0xA097              0x0000              0x0000
		CRC16_TMS37157       true            true             0x1021              0x89EC              0x0000
		CRC16_USB            true            true             0x8005              0xFFFF              0xFFFF
		CRC16_A              true            true             0x1021              0xC6C6              0x0000
		CRC16_KERMIT         true            true             0x1021              0x0000              0x0000
		CRC16_MODBUS         true            true             0x8005              0xFFFF              0x0000
		CRC16_X_25           true            true             0x1021              0xFFFF              0xFFFF
		CRC16_XMODEM         false           false            0x1021              0x0000              0x0000

		CRC32                true            true             0x04C11DB7          0xFFFFFFFF          0xFFFFFFFF
		CRC32_BZIP2          false           false            0x04C11DB7          0xFFFFFFFF          0xFFFFFFFF
		CRC32_C              true            true             0x1EDC6F41          0xFFFFFFFF          0xFFFFFFFF
		CRC32_D              true            true             0xA833982B          0xFFFFFFFF          0xFFFFFFFF
		CRC32_MPEG2          false           false            0x04C11DB7          0xFFFFFFFF          0x00000000
		CRC32_POSIX          false           false            0x04C11DB7          0x00000000          0xFFFFFFFF
		CRC32_Q              false           false            0x814141AB          0x00000000          0x00000000
		CRC32_JAMCRC         true            true             0x04C11DB7          0xFFFFFFFF          0x00000000
		CRC32_XFER           false           false            0x000000AF          0x00000000          0x00000000

		CRC64_ECMA_182       false           false            0x42F0E1EBA9EA3693  0x0000000000000000  0x0000000000000000
		CRC64_GO_ISO         true            true             0x000000000000001B  0xFFFFFFFFFFFFFFFF  0xFFFFFFFFFFFFFFFF
		CRC64_WE             false           false            0x42F0E1EBA9EA3693  0xFFFFFFFFFFFFFFFF  0xFFFFFFFFFFFFFFFF
		CRC64_XZ             true            true             0x42F0E1EBA9EA3693  0xFFFFFFFFFFFFFFFF  0xFFFFFFFFFFFFFFFF
	*/
	class CRC {
	private:
		class Helper {
		public:
			template<size_t Bits>
			inline static constexpr uint_t<Bits> AE_CALL reflectUnsigned(uint_t<Bits> x, size_t wordLength = std::numeric_limits<uint_t<Bits>>::digits) {
				for (uint_t<Bits> l = 1u, h = l << (wordLength - 1); h > l; h >>= 1, l <<= 1) {
					const uint_t<Bits> m = h | l, t = x & m;
					if ((t == h) || (t == l)) x ^= m;
				}
				return x;
			}
		};


	public:
		AE_DECLARE_CANNOT_INSTANTIATE(CRC);

		template<size_t Bits>
		using TableType = std::array<uint_t<Bits>, 256>;

		using ReflectionTableType = std::array<uint8_t, 256>;

		template<size_t Bits>
		static TableType<Bits> AE_CALL createTable(uint_t<Bits> polynomial) {
			std::array<uint_t<Bits>, 256> table;

			for (size_t i = 0; i < 256; ++i) {
				uint_t<Bits> remainder = 0;
				_wordUpdate<Bits>(remainder, polynomial, i, 8, false);
				table[_reflectOptionally<Bits>(i, false, 8)] = _reflectOptionally<Bits>(remainder, false, Bits);
			}

			return table;
		}

		inline static ReflectionTableType AE_CALL createReflectionTable() {
			ReflectionTableType table;
			for (size_t i = 0; i < 256; ++i) table[i] = Helper::reflectUnsigned<8>(i);
			return table;
		}

		template<size_t Bits>
		inline static constexpr uint_t<Bits> AE_CALL init(uint_t<Bits> initialValue) {
			return initialValue;
		}

		template<size_t Bits, bool InputReflected>
		static uint_t<Bits> AE_CALL update(uint_t<Bits> crc, const void* data, size_t len, const TableType<Bits>& table) {
			auto src = (const uint8_t*)data;
			uint_t<Bits> val;
			for (size_t i = 0; i < len; ++i) {
				if constexpr (InputReflected) {
					val = REFLECTION_TABLE[src[i]];
				} else {
					val = src[i];
				}
				crc = table[((uint_t<Bits>)(crc >> (Bits - 8)) ^ val) & 0xFF] ^ (uint_t<Bits>)(crc << 8);
			}
			return crc;
		}

		template<size_t Bits, bool ResultReflected>
		inline static uint_t<Bits> AE_CALL finish(uint_t<Bits> crc, uint_t<Bits> finalXorValue) {
			if constexpr (ResultReflected) {
				return (Helper::reflectUnsigned<Bits>(crc, Bits) ^ finalXorValue) & LOW_BITS_MASK<Bits>;
			} else {
				return (crc ^ finalXorValue) & LOW_BITS_MASK<Bits>;
			}
		}

		template<size_t Bits>
		inline static uint_t<Bits> AE_CALL calc(const void* data, size_t len, uint_t<Bits> initialValue, uint_t<Bits> finalXorValue, bool reflectInput, bool ResultReflected, const TableType<Bits>& table) {
			auto crc = init<Bits>(initialValue);
			crc = reflectInput ? update<Bits, true>(crc, data, len, table) : update<Bits, false>(crc, data, len, table);
			return ResultReflected ? finish<Bits, true>(crc, finalXorValue) : finish<Bits, false>(crc, finalXorValue);
		}

	private:
		inline static const ReflectionTableType REFLECTION_TABLE = createReflectionTable();

		template<size_t Bits>
		static constexpr uint_t<Bits> LOW_BITS_MASK = Bits ? (((((uint_t<Bits>)(1) << (Bits - 1)) - (uint_t<Bits>)(1)) << 1) | (uint_t<Bits>)(1)) : 0u;

		template<size_t Bits>
		static void AE_CALL _wordUpdate(uint_t<Bits>& remainder, uint_t<Bits> truncatedDivisor, uint_t<Bits> newDividendBits, size_t wordLength, bool reflect) {
			constexpr const uint_t<Bits> highBitMask = (uint_t<Bits>)1 << (Bits - 1);

			newDividendBits = _reflectOptionally<Bits>(newDividendBits, !reflect, wordLength);

			for (size_t i = 8; i; --i, newDividendBits >>= 1) {
				remainder ^= (newDividendBits & 1u) ? highBitMask : 0u;

				bool const quotient = remainder & highBitMask;

				remainder <<= 1;
				remainder ^= quotient ? truncatedDivisor : 0u;
			}
		}

		template<size_t Bits>
		inline static constexpr uint_t<Bits> AE_CALL _reflectOptionally(uint_t<Bits> x, bool reflect, size_t wordLength = std::numeric_limits<uint_t<Bits>>::digits) {
			return reflect ? Helper::reflectUnsigned<Bits>(x, wordLength) : x;
		}
	};
}