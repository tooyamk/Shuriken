#pragma once

#include "../BaseTester.h"
#include "srk/math/Math.h"
#include "srk/ThreadPool.h"

namespace srk::meta {
	/*template<int64_t I, int64_t N, auto... NonTypeArgs, typename Fn, typename... Args>
	concept aabbf = requires(Fn&& fn, Args&&... args) {
		fn.operator()<I, N, NonTypeArgs...>(std::forward<Args>(args)...);
		//std::invoke(std::forward<F>(f), std::forward<Args>(args)...);
	};

	template <typename Tuple, std::size_t... I>
	void process(Tuple const& tuple, std::index_sequence<I...>) {
		printaln(std::get<I>(tuple)...);
	}

	template <typename Tuple>
	void process(Tuple const& tuple) {
		process(tuple, std::make_index_sequence<std::tuple_size<Tuple>::value>());
	}

	template<auto... NonTypeArgs, typename Fn, typename... Args>
	//requires (aabbf<I, N, NonTypeArgs..., Fn, Args...>)
	inline void loop(Fn&& fn, Args&&... args) {
		//constexpr auto ret = fn.operator()<NonTypeArgs...>(std::forward<Args>(args)...);
		//if constexpr (ret) {
		//	loop<*ret>(std::forward<Fn>(fn), std::forward<Args>(args)...);
		//}
		//if constexpr (std::get<0>(ret)) {
			//process(nextNonTypeArgs);
			//constexpr auto i = std::make_index_sequence<std::tuple_size_v<nextNonTypeArgs>>;
			//loop<std::get<std::make_index_sequence<std::tuple_size<nextNonTypeArgs>::value>>(nextNonTypeArgs)...>(std::forward<Fn>(fn), std::forward<Args>(args)...);
		//}
	}*/

	/*template<int64_t I, int64_t N, typename Fn, typename... Args>
	inline void loopIf(Fn&& fn, Args&&... args) {
		if constexpr (I < N) {
			if constexpr (fn.operator()(std::forward<Args>(args)...)) {
				loop<I + 1, N>(std::forward<Fn>(fn), std::forward<Args>(args)...);
			}
		}
	}*/

	/*template<int64_t I, typename Fn, typename... Args>
	inline void loop(Fn&& fn, Args&&... args) {
		if constexpr (fn.operator()(std::forward<Args>(args)...)) {
			loop<I + 1>(std::forward<Fn>(fn), std::forward<Args>(args)...);
		}
	}*/
}

class CompressTextureTester : public BaseTester {
public:
	static constexpr uint32_t TileSize = 128;

	virtual int32_t SRK_CALL run() override {
		auto srcDir = Application::getAppPath().parent_path().u8string();
		IntrusivePtr<Image> src;
		{
			auto srcBin = readFile(srcDir + "/Resources/tex1.jpg");
#ifdef SRK_HAS_PNG_CONVERTER_H
			if (srcBin.seekBegin().read<uint32_t>() == extensions::PNGConverter::HEADER_MAGIC) src = extensions::PNGConverter::decode(srcBin.seekBegin());
#endif
#ifdef SRK_HAS_JPEG_CONVERTER_H
			if (!src && srcBin.seekBegin().read<ba_vt::UIX>(3) == extensions::JPEGConverter::HEADER_MAGIC) src = extensions::JPEGConverter::decode(srcBin.seekBegin());
#endif
		}
		if (!src) return 0;

		src->format = textureFormatTypeSwitch(src->format, false);
		if (src->format == TextureFormat::R8G8B8_UNORM || src->format == TextureFormat::R8G8B8_UNORM_SRGB) {
			ByteArray dst(src->dimensions.getMultiplies() * 4);
			dst.setLength(dst.getCapacity());
			Image::convertFormat(src->dimensions, src->format, src->source.getSource(), src->format == TextureFormat::R8G8B8_UNORM ? TextureFormat::R8G8B8A8_UNORM : TextureFormat::R8G8B8A8_UNORM_SRGB, dst.getSource());
			src->format = TextureFormat::R8G8B8A8_UNORM;
			src->source = std::move(dst);
		}

		Image img2;
		img2.dimensions = src->dimensions >> 2;
		src->scale(img2);

		writeFile(srcDir + "/Resources/tex11.png", extensions::PNGConverter::encode(img2));

		ThreadPool tp(11);

		auto tiles = calcTiles(src->dimensions);
		auto dstSize = tiles * TileSize;

		if (dstSize != src->dimensions) {
			Image img;
			img.dimensions = dstSize;
			src->scale(img);
			*src = std::move(img);
		}

		//meta::loop<1, 2>(lmd, 1, 2, 3);

		src->flipY();

		/*src->size = 4;
		src->source = ByteArray(64);
		for (auto i = 0; i < 64; ++i) src->source.write<uint8_t>(255);*/

		auto t0 = Time::now();
#ifdef SRK_HAS_ASTC_CONVERTER_H
		/*auto astc = extensions::ASTCConverter::encode(*src, Vec3ui32(4, 4, 1), extensions::ASTCConverter::Profile::LDR, extensions::ASTCConverter::Quality::MEDIUM, extensions::ASTCConverter::Flags::NONE, 12, [&tp](const std::function<void()>& fn) {
			return tp.enqueue(fn);
			});*/
#endif

		IntrusivePtr<Image> srcMp1 = new Image();
		srcMp1->dimensions = src->dimensions >> 1;
		src->scale(*srcMp1);
		/*auto astcMp1 = extensions::ASTCConverter::encode(*srcMp1, Vec3ui32(4, 4, 1), extensions::ASTCConverter::Profile::LDR, extensions::ASTCConverter::Quality::MEDIUM, extensions::ASTCConverter::Flags::NONE, 12, [&tp](const std::function<void()>& fn) {
			return tp.enqueue(fn);
			});*/

		//for (auto i = 0; i < 16; ++i) printaln(i, " : ", astc.getSource()[i]);
		//printaln(Time::now() - t0);
		//writeFile(srcDir + "/img.txt", astc);

#ifdef SRK_HAS_BC7_CONVERTER_H
		auto bc7 = extensions::BC7Converter::encode(*src, 2, 64, extensions::BC7Converter::Flags::NONE, 12, [&tp](const std::function<void()>& fn) {
			return tp.enqueue(fn);
			});
		auto bc7Mp1 = extensions::BC7Converter::encode(*srcMp1, 2, 64, extensions::BC7Converter::Flags::NONE, 12, [&tp](const std::function<void()>& fn) {
			return tp.enqueue(fn);
			});
		//for (auto i = 0; i < 16; ++i) printaln(i, " : ", bc7.getSource()[i]);
		//printaln(Time::now() - t0);
		bc7.setPosition(bc7.getLength());
		bc7.write<ba_vt::BYTE>(bc7Mp1);
		writeFile(srcDir + "/img.txt", bc7);
#endif

		printaln(L"finish!!!"sv);

		return 0;
	}

	uint32_t SRK_CALL calcTiles(uint32_t size) {
		auto n = size / (TileSize - 2);
		if (n == 0) return 1;

		auto d = size - n * (TileSize - 2);
		if (d > 25) ++n;
		return n;
	}

	Vec2ui32 SRK_CALL calcTiles(Vec2ui32 size) {
		return Vec2ui32(std::max(1u, (uint32_t)std::round((float32_t)size[0] / TileSize)), std::max(1u, (uint32_t)std::round((float32_t)size[1] / TileSize)));
	}
};