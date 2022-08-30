#pragma once

#include "../BaseTester.h"
#include "srk/math/Math.h"

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
		auto srcDir = getAppPath().parent_path().u8string();
		auto src = extensions::PNGConverter::decode(readFile(srcDir + "/Resources/tex1.png"));
		if (!src) return 0;

		if (src->format == modules::graphics::TextureFormat::R8G8B8) {
			ByteArray dst(src->size.getMultiplies() * 4);
			dst.setLength(dst.getCapacity());
			Image::convertFormat(src->size, src->format, src->source.getSource(), modules::graphics::TextureFormat::R8G8B8A8, dst.getSource());
			src->format = modules::graphics::TextureFormat::R8G8B8A8;
			src->source = std::move(dst);
		}

		auto tiles = calcTiles(src->size);
		auto dstSize = tiles * TileSize;

		if (dstSize != src->size) {
			Image img;
			img.size = dstSize;
			src->scale(img);
			*src = std::move(img);
		}

		//meta::loop<1, 2>(lmd, 1, 2, 3);

		src->flipY();

		auto t0 = Time::now();
		auto astc = extensions::ASTCConverter::encode(*src, Vec3ui32(4, 4, 1), extensions::ASTCConverter::Profile::LDR, extensions::ASTCConverter::Quality::MEDIUM, extensions::ASTCConverter::Flags::WRITE_HEADER, 12);
		//printaln(Time::now() - t0);
		writeFile(srcDir + "/img.astc", astc);

		auto bc7 = extensions::BC7Converter::encode(*src, 2, 64, extensions::BC7Converter::Flags::WRITE_DDS_HEADER, 12);
		printaln(Time::now() - t0);
		writeFile(srcDir + "/img.dds", bc7);

		printaln("finish!!!");

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