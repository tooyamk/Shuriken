#pragma once

#include "base/Global.h"

namespace aurora::hash {
	class AE_DLL CRC {
	public:
		AE_DECLA_CANNOT_INSTANTIATE(CRC);

		template<size_t Bits>
		using TableType = const uint_t<Bits>(&)[256];

		template<size_t Bits>
		static uint_t<Bits> AE_CALL hash(const uint8_t* data, size_t len, uint_t<Bits> initialValue, uint_t<Bits> finalXorValue, TableType<Bits> table) {
			static_assert(false, "only support 16, 32, 64 bits mode");
		}

		template<>
		static uint16_t AE_CALL hash<16>(const uint8_t* data, size_t len, uint_t<16> initialValue, uint_t<16> finalXorValue, TableType<16> table) {
			uint16_t crc = 0;
			for (size_t i = 0; i < len; ++i) crc = ((crc << 8) & 0xFFFF) ^ table[(crc >> 8) ^ ((uint16_t)data[i]) & 0xFF];
			return crc;
		}

		template<>
		static uint32_t AE_CALL hash<32>(const uint8_t* data, size_t len, uint_t<32> initialValue, uint_t<32> finalXorValue, TableType<32> table) {
			uint32_t crc = (std::numeric_limits<uint32_t>::max)();
			for (size_t i = 0; i < len; ++i) crc = ((crc >> 8) & 0x00FFFFFFui32) ^ table[(crc ^ (uint32_t)data[i]) & 0xFF];
			return crc ^ (std::numeric_limits<uint32_t>::max)();
		}

		template<>
		static uint64_t AE_CALL hash<64>(const uint8_t* data, size_t len, uint_t<64> initialValue, uint_t<64> finalXorValue, TableType<64> table) {
			auto crc = initialValue;
			for (size_t i = 0; i < len; ++i) crc = table[((uint32_t)(crc >> 56) ^ (uint32_t)data[i]) & 0xFF] ^ (crc << 8);
			return crc ^ finalXorValue;
		}

		template<size_t Bits>
		inline static uint_t<Bits> AE_CALL begin() {
			static_assert(Bits == 16 || Bits == 32 || Bits == 64, "only support 16, 32, 64 bits mode");

			return BitUInt<Bits>::MAX;
		}

		template<size_t Bits>
		static void AE_CALL update(uint_t<Bits>& crc, const uint8_t* data, size_t len, TableType<Bits> table) {
			static_assert(false, "only support 16, 32, 64 bits mode");
		}

		template<>
		static void AE_CALL update<16>(uint16_t& crc, const uint8_t* data, size_t len, TableType<16> table) {
			for (size_t i = 0; i < len; ++i) crc = ((crc << 8) & 0xFFFF) ^ table[(crc >> 8) ^ (data[i] & 0xFF)];
		}

		template<>
		static void AE_CALL update<32>(uint32_t& crc, const uint8_t* data, size_t len, TableType<32> table) {
			for (size_t i = 0; i < len; ++i) crc = ((crc >> 8) & 0x00FFFFFFui32) ^ table[(crc ^ (uint32_t)data[i]) & 0xFF];
		}

		template<>
		static void AE_CALL update<64>(uint64_t& crc, const uint8_t* data, size_t len, TableType<64> table) {
			for (size_t i = 0; i < len; ++i) crc = table[((uint32_t)(crc >> 56) ^ (uint32_t)data[i]) & 0xFF] ^ (crc << 8);
		}

		template<size_t Bits>
		inline static void AE_CALL end(uint_t<Bits>& crc) {
			static_assert(Bits == 16 || Bits == 32 || Bits == 64, "only support 16, 32, 64 bits mode");

			crc ^= BitUInt<Bits>::MAX;
		}

		inline static const uint16_t TABLE16[] = {
			0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50A5, 0x60C6, 0x70E7,
			0x8108, 0x9129, 0xA14A, 0xB16B, 0xC18C, 0xD1AD, 0xE1CE, 0xF1EF,
			0x1231, 0x0210, 0x3273, 0x2252, 0x52B5, 0x4294, 0x72F7, 0x62D6,
			0x9339, 0x8318, 0xB37B, 0xA35A, 0xD3BD, 0xC39C, 0xF3FF, 0xE3DE,
			0x2462, 0x3443, 0x0420, 0x1401, 0x64E6, 0x74C7, 0x44A4, 0x5485,
			0xA56A, 0xB54B, 0x8528, 0x9509, 0xE5EE, 0xF5CF, 0xC5AC, 0xD58D,
			0x3653, 0x2672, 0x1611, 0x0630, 0x76D7, 0x66F6, 0x5695, 0x46B4,
			0xB75B, 0xA77A, 0x9719, 0x8738, 0xF7DF, 0xE7FE, 0xD79D, 0xC7BC,
			0x48C4, 0x58E5, 0x6886, 0x78A7, 0x0840, 0x1861, 0x2802, 0x3823,
			0xC9CC, 0xD9ED, 0xE98E, 0xF9AF, 0x8948, 0x9969, 0xA90A, 0xB92B,
			0x5AF5, 0x4AD4, 0x7AB7, 0x6A96, 0x1A71, 0x0A50, 0x3A33, 0x2A12,
			0xDBFD, 0xCBDC, 0xFBBF, 0xEB9E, 0x9B79, 0x8B58, 0xBB3B, 0xAB1A,
			0x6CA6, 0x7C87, 0x4CE4, 0x5CC5, 0x2C22, 0x3C03, 0x0C60, 0x1C41,
			0xEDAE, 0xFD8F, 0xCDEC, 0xDDCD, 0xAD2A, 0xBD0B, 0x8D68, 0x9D49,
			0x7E97, 0x6EB6, 0x5ED5, 0x4EF4, 0x3E13, 0x2E32, 0x1E51, 0x0E70,
			0xFF9F, 0xEFBE, 0xDFDD, 0xCFFC, 0xBF1B, 0xAF3A, 0x9F59, 0x8F78,
			0x9188, 0x81A9, 0xB1CA, 0xA1EB, 0xD10C, 0xC12D, 0xF14E, 0xE16F,
			0x1080, 0x00A1, 0x30C2, 0x20E3, 0x5004, 0x4025, 0x7046, 0x6067,
			0x83B9, 0x9398, 0xA3FB, 0xB3DA, 0xC33D, 0xD31C, 0xE37F, 0xF35E,
			0x02B1, 0x1290, 0x22F3, 0x32D2, 0x4235, 0x5214, 0x6277, 0x7256,
			0xB5EA, 0xA5CB, 0x95A8, 0x8589, 0xF56E, 0xE54F, 0xD52C, 0xC50D,
			0x34E2, 0x24C3, 0x14A0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
			0xA7DB, 0xB7FA, 0x8799, 0x97B8, 0xE75F, 0xF77E, 0xC71D, 0xD73C,
			0x26D3, 0x36F2, 0x0691, 0x16B0, 0x6657, 0x7676, 0x4615, 0x5634,
			0xD94C, 0xC96D, 0xF90E, 0xE92F, 0x99C8, 0x89E9, 0xB98A, 0xA9AB,
			0x5844, 0x4865, 0x7806, 0x6827, 0x18C0, 0x08E1, 0x3882, 0x28A3,
			0xCB7D, 0xDB5C, 0xEB3F, 0xFB1E, 0x8BF9, 0x9BD8, 0xABBB, 0xBB9A,
			0x4A75, 0x5A54, 0x6A37, 0x7A16, 0x0AF1, 0x1AD0, 0x2AB3, 0x3A92,
			0xFD2E, 0xED0F, 0xDD6C, 0xCD4D, 0xBDAA, 0xAD8B, 0x9DE8, 0x8DC9,
			0x7C26, 0x6C07, 0x5C64, 0x4C45, 0x3CA2, 0x2C83, 0x1CE0, 0x0CC1,
			0xEF1F, 0xFF3E, 0xCF5D, 0xDF7C, 0xAF9B, 0xBFBA, 0x8FD9, 0x9FF8,
			0x6E17, 0x7E36, 0x4E55, 0x5E74, 0x2E93, 0x3EB2, 0x0ED1, 0x1EF0
		};

		inline static const uint32_t TABLE32[] = {
			0x00000000ui32, 0x77073096ui32, 0xEE0E612Cui32,
			0x990951BAui32, 0x076DC419ui32, 0x706AF48Fui32,
			0xE963A535ui32, 0x9E6495A3ui32, 0x0EDB8832ui32,
			0x79DCB8A4ui32, 0xE0D5E91Eui32, 0x97D2D988ui32,
			0x09B64C2Bui32, 0x7EB17CBDui32, 0xE7B82D07ui32,
			0x90BF1D91ui32, 0x1DB71064ui32, 0x6AB020F2ui32,
			0xF3B97148ui32, 0x84BE41DEui32, 0x1ADAD47Dui32,
			0x6DDDE4EBui32, 0xF4D4B551ui32, 0x83D385C7ui32,
			0x136C9856ui32, 0x646BA8C0ui32, 0xFD62F97Aui32,
			0x8A65C9ECui32, 0x14015C4Fui32, 0x63066CD9ui32,
			0xFA0F3D63ui32, 0x8D080DF5ui32, 0x3B6E20C8ui32,
			0x4C69105Eui32, 0xD56041E4ui32, 0xA2677172ui32,
			0x3C03E4D1ui32, 0x4B04D447ui32, 0xD20D85FDui32,
			0xA50AB56Bui32, 0x35B5A8FAui32, 0x42B2986Cui32,
			0xDBBBC9D6ui32, 0xACBCF940ui32, 0x32D86CE3ui32,
			0x45DF5C75ui32, 0xDCD60DCFui32, 0xABD13D59ui32,
			0x26D930ACui32, 0x51DE003Aui32, 0xC8D75180ui32,
			0xBFD06116ui32, 0x21B4F4B5ui32, 0x56B3C423ui32,
			0xCFBA9599ui32, 0xB8BDA50Fui32, 0x2802B89Eui32,
			0x5F058808ui32, 0xC60CD9B2ui32, 0xB10BE924ui32,
			0x2F6F7C87ui32, 0x58684C11ui32, 0xC1611DABui32,
			0xB6662D3Dui32, 0x76DC4190ui32, 0x01DB7106ui32,
			0x98D220BCui32, 0xEFD5102Aui32, 0x71B18589ui32,
			0x06B6B51Fui32, 0x9FBFE4A5ui32, 0xE8B8D433ui32,
			0x7807C9A2ui32, 0x0F00F934ui32, 0x9609A88Eui32,
			0xE10E9818ui32, 0x7F6A0DBBui32, 0x086D3D2Dui32,
			0x91646C97ui32, 0xE6635C01ui32, 0x6B6B51F4ui32,
			0x1C6C6162ui32, 0x856530D8ui32, 0xF262004Eui32,
			0x6C0695EDui32, 0x1B01A57Bui32, 0x8208F4C1ui32,
			0xF50FC457ui32, 0x65B0D9C6ui32, 0x12B7E950ui32,
			0x8BBEB8EAui32, 0xFCB9887Cui32, 0x62DD1DDFui32,
			0x15DA2D49ui32, 0x8CD37CF3ui32, 0xFBD44C65ui32,
			0x4DB26158ui32, 0x3AB551CEui32, 0xA3BC0074ui32,
			0xD4BB30E2ui32, 0x4ADFA541ui32, 0x3DD895D7ui32,
			0xA4D1C46Dui32, 0xD3D6F4FBui32, 0x4369E96Aui32,
			0x346ED9FCui32, 0xAD678846ui32, 0xDA60B8D0ui32,
			0x44042D73ui32, 0x33031DE5ui32, 0xAA0A4C5Fui32,
			0xDD0D7CC9ui32, 0x5005713Cui32, 0x270241AAui32,
			0xBE0B1010ui32, 0xC90C2086ui32, 0x5768B525ui32,
			0x206F85B3ui32, 0xB966D409ui32, 0xCE61E49Fui32,
			0x5EDEF90Eui32, 0x29D9C998ui32, 0xB0D09822ui32,
			0xC7D7A8B4ui32, 0x59B33D17ui32, 0x2EB40D81ui32,
			0xB7BD5C3Bui32, 0xC0BA6CADui32, 0xEDB88320ui32,
			0x9ABFB3B6ui32, 0x03B6E20Cui32, 0x74B1D29Aui32,
			0xEAD54739ui32, 0x9DD277AFui32, 0x04DB2615ui32,
			0x73DC1683ui32, 0xE3630B12ui32, 0x94643B84ui32,
			0x0D6D6A3Eui32, 0x7A6A5AA8ui32, 0xE40ECF0Bui32,
			0x9309FF9Dui32, 0x0A00AE27ui32, 0x7D079EB1ui32,
			0xF00F9344ui32, 0x8708A3D2ui32, 0x1E01F268ui32,
			0x6906C2FEui32, 0xF762575Dui32, 0x806567CBui32,
			0x196C3671ui32, 0x6E6B06E7ui32, 0xFED41B76ui32,
			0x89D32BE0ui32, 0x10DA7A5Aui32, 0x67DD4ACCui32,
			0xF9B9DF6Fui32, 0x8EBEEFF9ui32, 0x17B7BE43ui32,
			0x60B08ED5ui32, 0xD6D6A3E8ui32, 0xA1D1937Eui32,
			0x38D8C2C4ui32, 0x4FDFF252ui32, 0xD1BB67F1ui32,
			0xA6BC5767ui32, 0x3FB506DDui32, 0x48B2364Bui32,
			0xD80D2BDAui32, 0xAF0A1B4Cui32, 0x36034AF6ui32,
			0x41047A60ui32, 0xDF60EFC3ui32, 0xA867DF55ui32,
			0x316E8EEFui32, 0x4669BE79ui32, 0xCB61B38Cui32,
			0xBC66831Aui32, 0x256FD2A0ui32, 0x5268E236ui32,
			0xCC0C7795ui32, 0xBB0B4703ui32, 0x220216B9ui32,
			0x5505262Fui32, 0xC5BA3BBEui32, 0xB2BD0B28ui32,
			0x2BB45A92ui32, 0x5CB36A04ui32, 0xC2D7FFA7ui32,
			0xB5D0CF31ui32, 0x2CD99E8Bui32, 0x5BDEAE1Dui32,
			0x9B64C2B0ui32, 0xEC63F226ui32, 0x756AA39Cui32,
			0x026D930Aui32, 0x9C0906A9ui32, 0xEB0E363Fui32,
			0x72076785ui32, 0x05005713ui32, 0x95BF4A82ui32,
			0xE2B87A14ui32, 0x7BB12BAEui32, 0x0CB61B38ui32,
			0x92D28E9Bui32, 0xE5D5BE0Dui32, 0x7CDCEFB7ui32,
			0x0BDBDF21ui32, 0x86D3D2D4ui32, 0xF1D4E242ui32,
			0x68DDB3F8ui32, 0x1FDA836Eui32, 0x81BE16CDui32,
			0xF6B9265Bui32, 0x6FB077E1ui32, 0x18B74777ui32,
			0x88085AE6ui32, 0xFF0F6A70ui32, 0x66063BCAui32,
			0x11010B5Cui32, 0x8F659EFFui32, 0xF862AE69ui32,
			0x616BFFD3ui32, 0x166CCF45ui32, 0xA00AE278ui32,
			0xD70DD2EEui32, 0x4E048354ui32, 0x3903B3C2ui32,
			0xA7672661ui32, 0xD06016F7ui32, 0x4969474Dui32,
			0x3E6E77DBui32, 0xAED16A4Aui32, 0xD9D65ADCui32,
			0x40DF0B66ui32, 0x37D83BF0ui32, 0xA9BCAE53ui32,
			0xDEBB9EC5ui32, 0x47B2CF7Fui32, 0x30B5FFE9ui32,
			0xBDBDF21Cui32, 0xCABAC28Aui32, 0x53B39330ui32,
			0x24B4A3A6ui32, 0xBAD03605ui32, 0xCDD70693ui32,
			0x54DE5729ui32, 0x23D967BFui32, 0xB3667A2Eui32,
			0xC4614AB8ui32, 0x5D681B02ui32, 0x2A6F2B94ui32,
			0xB40BBE37ui32, 0xC30C8EA1ui32, 0x5A05DF1Bui32,
			0x2D02EF8Dui32
		};

		inline static const uint64_t TABLE64_WE[] = {
			0x0000000000000000ui64, 0x42F0E1EBA9EA3693ui64,
			0x85E1C3D753D46D26ui64, 0xC711223CFA3E5BB5ui64,
			0x493366450E42ECDFui64, 0x0BC387AEA7A8DA4Cui64,
			0xCCD2A5925D9681F9ui64, 0x8E224479F47CB76Aui64,
			0x9266CC8A1C85D9BEui64, 0xD0962D61B56FEF2Dui64,
			0x17870F5D4F51B498ui64, 0x5577EEB6E6BB820Bui64,
			0xDB55AACF12C73561ui64, 0x99A54B24BB2D03F2ui64,
			0x5EB4691841135847ui64, 0x1C4488F3E8F96ED4ui64,
			0x663D78FF90E185EFui64, 0x24CD9914390BB37Cui64,
			0xE3DCBB28C335E8C9ui64, 0xA12C5AC36ADFDE5Aui64,
			0x2F0E1EBA9EA36930ui64, 0x6DFEFF5137495FA3ui64,
			0xAAEFDD6DCD770416ui64, 0xE81F3C86649D3285ui64,
			0xF45BB4758C645C51ui64, 0xB6AB559E258E6AC2ui64,
			0x71BA77A2DFB03177ui64, 0x334A9649765A07E4ui64,
			0xBD68D2308226B08Eui64, 0xFF9833DB2BCC861Dui64,
			0x388911E7D1F2DDA8ui64, 0x7A79F00C7818EB3Bui64,
			0xCC7AF1FF21C30BDEui64, 0x8E8A101488293D4Dui64,
			0x499B3228721766F8ui64, 0x0B6BD3C3DBFD506Bui64,
			0x854997BA2F81E701ui64, 0xC7B97651866BD192ui64,
			0x00A8546D7C558A27ui64, 0x4258B586D5BFBCB4ui64,
			0x5E1C3D753D46D260ui64, 0x1CECDC9E94ACE4F3ui64,
			0xDBFDFEA26E92BF46ui64, 0x990D1F49C77889D5ui64,
			0x172F5B3033043EBFui64, 0x55DFBADB9AEE082Cui64,
			0x92CE98E760D05399ui64, 0xD03E790CC93A650Aui64,
			0xAA478900B1228E31ui64, 0xE8B768EB18C8B8A2ui64,
			0x2FA64AD7E2F6E317ui64, 0x6D56AB3C4B1CD584ui64,
			0xE374EF45BF6062EEui64, 0xA1840EAE168A547Dui64,
			0x66952C92ECB40FC8ui64, 0x2465CD79455E395Bui64,
			0x3821458AADA7578Fui64, 0x7AD1A461044D611Cui64,
			0xBDC0865DFE733AA9ui64, 0xFF3067B657990C3Aui64,
			0x711223CFA3E5BB50ui64, 0x33E2C2240A0F8DC3ui64,
			0xF4F3E018F031D676ui64, 0xB60301F359DBE0E5ui64,
			0xDA050215EA6C212Fui64, 0x98F5E3FE438617BCui64,
			0x5FE4C1C2B9B84C09ui64, 0x1D14202910527A9Aui64,
			0x93366450E42ECDF0ui64, 0xD1C685BB4DC4FB63ui64,
			0x16D7A787B7FAA0D6ui64, 0x5427466C1E109645ui64,
			0x4863CE9FF6E9F891ui64, 0x0A932F745F03CE02ui64,
			0xCD820D48A53D95B7ui64, 0x8F72ECA30CD7A324ui64,
			0x0150A8DAF8AB144Eui64, 0x43A04931514122DDui64,
			0x84B16B0DAB7F7968ui64, 0xC6418AE602954FFBui64,
			0xBC387AEA7A8DA4C0ui64, 0xFEC89B01D3679253ui64,
			0x39D9B93D2959C9E6ui64, 0x7B2958D680B3FF75ui64,
			0xF50B1CAF74CF481Fui64, 0xB7FBFD44DD257E8Cui64,
			0x70EADF78271B2539ui64, 0x321A3E938EF113AAui64,
			0x2E5EB66066087D7Eui64, 0x6CAE578BCFE24BEDui64,
			0xABBF75B735DC1058ui64, 0xE94F945C9C3626CBui64,
			0x676DD025684A91A1ui64, 0x259D31CEC1A0A732ui64,
			0xE28C13F23B9EFC87ui64, 0xA07CF2199274CA14ui64,
			0x167FF3EACBAF2AF1ui64, 0x548F120162451C62ui64,
			0x939E303D987B47D7ui64, 0xD16ED1D631917144ui64,
			0x5F4C95AFC5EDC62Eui64, 0x1DBC74446C07F0BDui64,
			0xDAAD56789639AB08ui64, 0x985DB7933FD39D9Bui64,
			0x84193F60D72AF34Fui64, 0xC6E9DE8B7EC0C5DCui64,
			0x01F8FCB784FE9E69ui64, 0x43081D5C2D14A8FAui64,
			0xCD2A5925D9681F90ui64, 0x8FDAB8CE70822903ui64,
			0x48CB9AF28ABC72B6ui64, 0x0A3B7B1923564425ui64,
			0x70428B155B4EAF1Eui64, 0x32B26AFEF2A4998Dui64,
			0xF5A348C2089AC238ui64, 0xB753A929A170F4ABui64,
			0x3971ED50550C43C1ui64, 0x7B810CBBFCE67552ui64,
			0xBC902E8706D82EE7ui64, 0xFE60CF6CAF321874ui64,
			0xE224479F47CB76A0ui64, 0xA0D4A674EE214033ui64,
			0x67C58448141F1B86ui64, 0x253565A3BDF52D15ui64,
			0xAB1721DA49899A7Fui64, 0xE9E7C031E063ACECui64,
			0x2EF6E20D1A5DF759ui64, 0x6C0603E6B3B7C1CAui64,
			0xF6FAE5C07D3274CDui64, 0xB40A042BD4D8425Eui64,
			0x731B26172EE619EBui64, 0x31EBC7FC870C2F78ui64,
			0xBFC9838573709812ui64, 0xFD39626EDA9AAE81ui64,
			0x3A28405220A4F534ui64, 0x78D8A1B9894EC3A7ui64,
			0x649C294A61B7AD73ui64, 0x266CC8A1C85D9BE0ui64,
			0xE17DEA9D3263C055ui64, 0xA38D0B769B89F6C6ui64,
			0x2DAF4F0F6FF541ACui64, 0x6F5FAEE4C61F773Fui64,
			0xA84E8CD83C212C8Aui64, 0xEABE6D3395CB1A19ui64,
			0x90C79D3FEDD3F122ui64, 0xD2377CD44439C7B1ui64,
			0x15265EE8BE079C04ui64, 0x57D6BF0317EDAA97ui64,
			0xD9F4FB7AE3911DFDui64, 0x9B041A914A7B2B6Eui64,
			0x5C1538ADB04570DBui64, 0x1EE5D94619AF4648ui64,
			0x02A151B5F156289Cui64, 0x4051B05E58BC1E0Fui64,
			0x87409262A28245BAui64, 0xC5B073890B687329ui64,
			0x4B9237F0FF14C443ui64, 0x0962D61B56FEF2D0ui64,
			0xCE73F427ACC0A965ui64, 0x8C8315CC052A9FF6ui64,
			0x3A80143F5CF17F13ui64, 0x7870F5D4F51B4980ui64,
			0xBF61D7E80F251235ui64, 0xFD913603A6CF24A6ui64,
			0x73B3727A52B393CCui64, 0x31439391FB59A55Fui64,
			0xF652B1AD0167FEEAui64, 0xB4A25046A88DC879ui64,
			0xA8E6D8B54074A6ADui64, 0xEA16395EE99E903Eui64,
			0x2D071B6213A0CB8Bui64, 0x6FF7FA89BA4AFD18ui64,
			0xE1D5BEF04E364A72ui64, 0xA3255F1BE7DC7CE1ui64,
			0x64347D271DE22754ui64, 0x26C49CCCB40811C7ui64,
			0x5CBD6CC0CC10FAFCui64, 0x1E4D8D2B65FACC6Fui64,
			0xD95CAF179FC497DAui64, 0x9BAC4EFC362EA149ui64,
			0x158E0A85C2521623ui64, 0x577EEB6E6BB820B0ui64,
			0x906FC95291867B05ui64, 0xD29F28B9386C4D96ui64,
			0xCEDBA04AD0952342ui64, 0x8C2B41A1797F15D1ui64,
			0x4B3A639D83414E64ui64, 0x09CA82762AAB78F7ui64,
			0x87E8C60FDED7CF9Dui64, 0xC51827E4773DF90Eui64,
			0x020905D88D03A2BBui64, 0x40F9E43324E99428ui64,
			0x2CFFE7D5975E55E2ui64, 0x6E0F063E3EB46371ui64,
			0xA91E2402C48A38C4ui64, 0xEBEEC5E96D600E57ui64,
			0x65CC8190991CB93Dui64, 0x273C607B30F68FAEui64,
			0xE02D4247CAC8D41Bui64, 0xA2DDA3AC6322E288ui64,
			0xBE992B5F8BDB8C5Cui64, 0xFC69CAB42231BACFui64,
			0x3B78E888D80FE17Aui64, 0x7988096371E5D7E9ui64,
			0xF7AA4D1A85996083ui64, 0xB55AACF12C735610ui64,
			0x724B8ECDD64D0DA5ui64, 0x30BB6F267FA73B36ui64,
			0x4AC29F2A07BFD00Dui64, 0x08327EC1AE55E69Eui64,
			0xCF235CFD546BBD2Bui64, 0x8DD3BD16FD818BB8ui64,
			0x03F1F96F09FD3CD2ui64, 0x41011884A0170A41ui64,
			0x86103AB85A2951F4ui64, 0xC4E0DB53F3C36767ui64,
			0xD8A453A01B3A09B3ui64, 0x9A54B24BB2D03F20ui64,
			0x5D45907748EE6495ui64, 0x1FB5719CE1045206ui64,
			0x919735E51578E56Cui64, 0xD367D40EBC92D3FFui64,
			0x1476F63246AC884Aui64, 0x568617D9EF46BED9ui64,
			0xE085162AB69D5E3Cui64, 0xA275F7C11F7768AFui64,
			0x6564D5FDE549331Aui64, 0x279434164CA30589ui64,
			0xA9B6706FB8DFB2E3ui64, 0xEB46918411358470ui64,
			0x2C57B3B8EB0BDFC5ui64, 0x6EA7525342E1E956ui64,
			0x72E3DAA0AA188782ui64, 0x30133B4B03F2B111ui64,
			0xF7021977F9CCEAA4ui64, 0xB5F2F89C5026DC37ui64,
			0x3BD0BCE5A45A6B5Dui64, 0x79205D0E0DB05DCEui64,
			0xBE317F32F78E067Bui64, 0xFCC19ED95E6430E8ui64,
			0x86B86ED5267CDBD3ui64, 0xC4488F3E8F96ED40ui64,
			0x0359AD0275A8B6F5ui64, 0x41A94CE9DC428066ui64,
			0xCF8B0890283E370Cui64, 0x8D7BE97B81D4019Fui64,
			0x4A6ACB477BEA5A2Aui64, 0x089A2AACD2006CB9ui64,
			0x14DEA25F3AF9026Dui64, 0x562E43B4931334FEui64,
			0x913F6188692D6F4Bui64, 0xD3CF8063C0C759D8ui64,
			0x5DEDC41A34BBEEB2ui64, 0x1F1D25F19D51D821ui64,
			0xD80C07CD676F8394ui64, 0x9AFCE626CE85B507ui64
		};
	};
}