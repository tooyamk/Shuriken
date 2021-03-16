#pragma once

#include "../BaseTester.h"
#include <concepts>
#include <ranges>
#include "statical.h"

template<typename T>
concept Integral = std::is_integral<T>::value;

template<Integral T>
T Add(T a, T b) {
	return b;
}

template<typename R>
requires requires { typename string_data_t<std::remove_cvref_t<R>>; }
//requires StringData<std::remove_cvref_t<R>>
inline auto& AE_CALL operator+=(std::u8string& left, R&& right) {
	if constexpr (std::same_as<std::remove_cvref_t<R>, std::string>) {
		left += (const std::u8string&)right;
	} else if constexpr (std::same_as<std::remove_cvref_t<R>, std::string_view>) {
		left += (const std::u8string_view&)right;
	} else if constexpr (std::convertible_to<std::remove_cvref_t<R>, char const*>) {
		left += (const char8_t*)right;
	}

	return left;
}

template <std::unsigned_integral T>
constexpr int32_t Popcount_fallback(T val) noexcept {
	constexpr auto digits = std::numeric_limits<T>::digits;
	val = (T)(val - ((val >> 1) & (T)(0x5555'5555'5555'5555ull)));
	val = (T)((val & (T)(0x3333'3333'3333'3333ull)) + ((val >> 2) & (T)(0x3333'3333'3333'3333ull)));
	val = (T)((val + (val >> 4)) & (T)(0x0F0F'0F0F'0F0F'0F0Full));
	for (int32_t shiftDigits = 8; shiftDigits < digits; shiftDigits <<= 1) val += (T)(val >> shiftDigits);
	return val & (T)(digits + digits - 1);
}

template<packaged_concept<is_convertible_string8_data, std::remove_cvref> T>
inline std::conditional_t<string8_view<std::remove_cvref_t<T>>, std::remove_reference_t<T>&, convert_to_string8_view_t<std::remove_cvref_t<T>>> to_string8_view(T&& val) {
	if constexpr (string8_view<std::remove_cvref_t<T>>) {
		return val;
	} else {
		return std::move(convert_to_string8_view_t<std::remove_cvref_t<T>>(std::forward<T>(val)));
	}
}

template<auto V>
void AE_CALL get_name() {
	printdln(__FUNCSIG__);
}

inline constexpr uint8_t AE_CALL byteswap1(uint8_t val) {
	return val;
}

inline constexpr uint16_t AE_CALL byteswap1(uint16_t val) {
	if (std::is_constant_evaluated()) {
		return uint16_t(val << 8) | uint16_t(val >> 8);
	} else {
#if AE_COMPILER == AE_COMPILER_MSVC
		return _byteswap_ushort(val);
#elif AE_COMPILER == AE_COMPILER_GCC || AE_COMPILER == AE_COMPILER_CLANG
		return __builtin_bswap16(val);
#else
		return uint16_t(val << 8) | uint16_t(val >> 8);
#endif
	}
}

inline constexpr uint32_t AE_CALL byteswap1(uint32_t val) {
	if (std::is_constant_evaluated()) {
		return (val & 0x000000FFU << 24) | (val & 0x0000FF00U << 8) | (val & 0x00FF0000U >> 8) | (val & 0xFF000000U >> 24);
	} else {
#if AE_COMPILER == AE_COMPILER_MSVC
		return _byteswap_ulong(val);
#elif AE_COMPILER == AE_COMPILER_GCC || AE_COMPILER == AE_COMPILER_CLANG
		return __builtin_bswap32(val);
#else
		return (val & 0x000000FFU << 24) | (val & 0x0000FF00U << 8) | (val & 0x00FF0000U >> 8) | (val & 0xFF000000U >> 24);
#endif
	}
}

inline constexpr uint64_t AE_CALL byteswap1(uint64_t val) {
	if (std::is_constant_evaluated()) {
		uint64_t Hi = byteswap1(uint32_t(val));
		return (Hi << 32) | byteswap1(uint32_t(val >> 32));
	} else {
#if AE_COMPILER == AE_COMPILER_MSVC
		return _byteswap_uint64(val);
#elif AE_COMPILER == AE_COMPILER_GCC || AE_COMPILER == AE_COMPILER_CLANG
		return __builtin_bswap64(val);
#else
		uint64_t Hi = byteswap1(uint32_t(val));
		return (Hi << 32) | byteswap1(uint32_t(val >> 32));
#endif
	}
}

#define AE_REF_OBJECT(__CLASS__) \
protected: \
	mutable std::atomic_uint32_t _refCount = 0; \
public: \
	inline uint32_t AE_CALL getReferenceCount() const { return _refCount.load(std::memory_order_acquire); } \
	inline void AE_CALL ref() { _refCount.fetch_add(1, std::memory_order_release); } \
	inline static void AE_CALL unref(const __CLASS__& target) { \
		if (target._refCount.fetch_sub(1) <= 1) { delete &target; } \
	} \
private:

#define AE_REF_INTRUSIVE_PTR_OPERATE(__CLASS__) \
	inline void AE_CALL intrusivePtrAddRef(__CLASS__& val) { val.ref(); } \
	inline void AE_CALL intrusivePtrRelease(__CLASS__& val) { __CLASS__::unref(val); }


namespace BBCCCXX {
	class AABBCC {
		AE_REF_OBJECT(AABBCC)
	};

	AE_REF_INTRUSIVE_PTR_OPERATE(AABBCC)
}

class AAA123 {

};

class BBB : public AAA123 {

};

template<typename D, typename B>
concept BBSS = std::derived_from<D, B>;


template<auto T>
concept IIII = T == 1;


template<int T>
requires IIII<T>
void zzzz() {

}

bool adfwefwe() {
	return true;
}


template<typename T, template<typename> typename... Conditions>
concept logic_or = std::disjunction_v<Conditions<T>...>;

template<typename T, template<typename> typename... Conditions>
struct is_logic_or : std::bool_constant<logic_or<T, Conditions...>>{};


class OffscreenTester : public BaseTester {
public:
	virtual int32_t AE_CALL run() override {
		{
			IntrusivePtr aaa = new BBCCCXX::AABBCC();
		}

		 std::string ssss = "abc";
		zzzz<1>();

		int zzz111 = 1;

		constexpr std::array aefew{ ba_vt::BOOL, ba_vt::BOOL, ba_vt::BYTE };


		std::remove_const<const int&>::type fewfwe = zzz111;

		pakaged_modification<const int&, std::remove_reference, std::remove_const>::type zzz = zzz111;
		//auto bbbbb = same_any_of_in_tuple<char, std::tuple<void, char>>;
		///*
		//std::list<std::function<void()>> funcs;
		//std::packaged_task<int()> task(std::bind([]() {return 1; }));

		//auto f = [task(std::move(task))]() mutable {};
		//std::packaged_task task22(std::move(f));

		//funcs.emplace_back(std::move([task1(std::move(task))]() mutable {}));
		//*/

		constexpr std::string_view γ{ "0.5" };
		
		const char8_t dsfe[] = u8"abc";
		// uint16_t aa32 = byteswap1(uint16_t(0x0100));

		get_name<123>();

		using arr = statical::array<>;
		using arr1 = arr::push_back<std::string>;
		using arr2 = arr1::concat<arr>;
		using arr3 = arr::pop_back;
		arr3 a;

		auto monitors = Monitor::getMonitors();
		auto vms = monitors[0].getVideoModes();

		IntrusivePtr gml = new GraphicsModuleLoader();

		//if (gml->load(getDLLName("ae-graphics-gl"))) {
		if (gml->load("libs/" + getDLLName(u8"ae-graphics-d3d11"))) {
			SerializableObject args;

			IntrusivePtr gpstml = new ModuleLoader<IProgramSourceTranslator>();
			gpstml->load("libs/" + getDLLName("ae-program-source-translator"));

			args.insert("dxc", "libs/" + getDLLName("dxcompiler"));
			auto gpst = gpstml->create(&args);

			std::function<void(const std::string_view&)> createProcessInfoHandler = [](const std::string_view& msg) {
				printaln(msg);
			};

			args.insert("sampleCount", 1);
			args.insert("trans", gpst.uintptr());
			args.insert("offscreen", true);
			args.insert("driverType", "software");
			args.insert("createProcessInfoHandler", (uintptr_t)&createProcessInfoHandler);
			args.insert("debug", environment::is_debug);

			auto graphics = gml->create(&args);

			if (graphics) {
				printaln("Graphics Version : ", graphics->getVersion());

				graphics->getEventDispatcher().addEventListener(GraphicsEvent::ERR, createEventListener<GraphicsEvent>([](Event<GraphicsEvent>& e) {
					printaln(*(std::string_view*)e.getData());
					int a = 1;
				}));

				{
					{
						auto rs = graphics->createRasterizerState();
						rs->setFillMode(FillMode::SOLID);
						rs->setFrontFace(FrontFace::CW);
						rs->setCullMode(CullMode::NONE);
						graphics->setRasterizerState(rs);
					}

					auto tr = graphics->createTexture2DResource();
					auto rv = graphics->createRenderView();
					auto rt = graphics->createRenderTarget();
					tr->create(Vec2ui32(800, 600), 0, 1, 1, modules::graphics::TextureFormat::R8G8B8A8, modules::graphics::Usage::RENDERABLE);
					rv->create(tr, 0, 0, 0);
					rt->setRenderView(0, rv);

					auto program = graphics->createProgram();
					{
						const std::string _vs = R"(
struct VS_INPUT {
    float2 position : POSITION0;
    float2 uv : TEXCOORD0;
};

struct VS_OUTPUT {
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
};

VS_OUTPUT main(VS_INPUT input) {
    VS_OUTPUT output = (VS_OUTPUT)0;
    output.position = float4(input.position.x, input.position.y, 0.5f, 1.0f);
    output.uv = float2(input.uv.x, 1.0f - input.uv.y);
    return output;
}
)";
						const std::string _ps = R"(
struct PS_INPUT {
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
};

float4 main(PS_INPUT input) : SV_TARGET {
	return float4(1.0f, 0.0f, 0.0f, 1.0f);
}
)";

						ProgramSource vs;
						vs.language = ProgramLanguage::HLSL;
						vs.stage = ProgramStage::VS;
						vs.data = ByteArray((void*)_vs.data(), _vs.size(), ByteArray::Usage::SHARED);
						ProgramSource ps;
						ps.language = ProgramLanguage::HLSL;
						ps.stage = ProgramStage::VS;
						ps.data = ByteArray((void*)_ps.data(), _ps.size(), ByteArray::Usage::SHARED);
						program->create(vs, ps, nullptr, 0, nullptr);
					}

					IntrusivePtr vertexBuffers = new VertexBufferCollection();
					{
						float32_t vd[] = {
								-1.f, 1.f,
								1.f, 1.f,
								1.f, -1.f,
								-1.f, -1.f };

						using type = std::remove_cvref_t<decltype(vd[0])>;
						constexpr auto size = std::extent_v<decltype(vd)> *sizeof(type);

						modules::graphics::VertexFormat vf;
						vf.set<2, type>();

						auto vb = graphics->createVertexBuffer();
						vb->create(size, modules::graphics::Usage::NONE, vd, size);
						vb->setFormat(vf);
						vertexBuffers->set("POSITION0", vb);

						type uvd[] = {
								0.f, 0.f,
								1.f, 0.f,
								1.f, 1.f,
								0.f, 1.f };
						auto uvb = graphics->createVertexBuffer();
						uvb->create(size, modules::graphics::Usage::NONE, uvd, size);
						uvb->setFormat(vf);
						vertexBuffers->set("TEXCOORD0", uvb);
					}

					IntrusivePtr shaderParameters = new ShaderParameterCollection();
					{
						//shaderParameters->set("tex", new ShaderParameter(ShaderParameterUsage::AUTO))->set(tv, ShaderParameterUpdateBehavior::FORCE);
						//shaderParameters->set("texSampler", new ShaderParameter(ShaderParameterUsage::AUTO))->set(_pointTexSampler, ShaderParameterUpdateBehavior::FORCE);
					}

					auto indices = graphics->createIndexBuffer();
					{
						uint16_t id[] = {
								0, 1, 2,
								0, 2, 3 };

						using type = std::remove_cvref_t<decltype(id[0])>;
						constexpr auto size = std::extent_v<decltype(id)> *sizeof(type);

						indices->create(size, modules::graphics::Usage::NONE, id, size);
						indices->setFormat<type>();
					}

					graphics->setRenderTarget(rt);
					graphics->setViewport(Box2i32ui32(Vec2i32::ZERO, tr->getSize().cast<2>()));
					graphics->beginRender();
					graphics->clear(ClearFlag::COLOR | ClearFlag::DEPTH | ClearFlag::STENCIL, Vec4f32(0.0f, 0.0f, 0.25f, 1.0f), 1.0f, 0);
					graphics->setBlendState(nullptr, Vec4f32::ZERO);
					graphics->setDepthStencilState(nullptr, 0);
					graphics->draw(vertexBuffers, program, shaderParameters, indices);
					graphics->setRenderTarget(nullptr);

					{
						auto tex = graphics->createTexture2DResource();
						if (tex->create(tr->getSize(), 0, 1, 1, tr->getFormat(), modules::graphics::Usage::MAP_READ)) {
							if (tex->copyFrom(Vec3ui32::ZERO, 0, 0, tr, 0, 0, Box3ui32(Vec3ui32::ZERO, tr->getSize()))) {
								if (tex->map(0, 0, modules::graphics::Usage::MAP_READ) != modules::graphics::Usage::NONE) {
									auto pixelsSize = tr->getSize().getMultiplies() * 4;
									ByteArray pixels(pixelsSize, pixelsSize);
									tex->read(0, 0, 0, pixels.getSource(), pixels.getLength());
									tex->unmap(0, 0);

									Image img;
									img.format = modules::graphics::TextureFormat::R8G8B8A8;
									img.size = tr->getSize().cast<2>();
									img.source = std::move(pixels);

									/*
									auto aaa = extensions::PNGConverter::parse(readFile("D:/Users/Sephiroth/Desktop/6x6.png"));
									if (aaa->format == modules::graphics::TextureFormat::R8G8B8) {
										ByteArray dst(aaa->size.getMultiplies() * 4);
										dst.setLength(dst.getCapacity());
										Image::convertFormat(aaa->size, aaa->format, aaa->source.getSource(), modules::graphics::TextureFormat::R8G8B8A8, dst.getSource());
										aaa->format = modules::graphics::TextureFormat::R8G8B8A8;
										aaa->source = std::move(dst);
									}
									aaa->flipY();

									auto t0 = Time::now();
									auto out = extensions::ASTCConverter::encode(*aaa, extensions::ASTCConverter::BlockSize::BLOCK_4x4, extensions::ASTCConverter::Preset::FASTEST);
									if (out.isValid()) {
										printcln("use time : ", Time::now() - t0);
										writeFile("D:/Users/Sephiroth/Desktop/6x6.astc", out);
										printcln("doneeeeeeeeeeeeeeeeee");
									}
									*/

									//writeFile(getAppPath().parent_path().string() + "/offscreen.png", extensions::PNGConverter::encode(img));
								}
							}
						}
					}
				}

				IntrusivePtr looper = new Looper(1000.0 / 60.0);

				looper->getEventDispatcher().addEventListener(LooperEvent::TICKING, new EventListener(std::function([](Event<LooperEvent>& e) {
					auto dt = float64_t(*e.getData<int64_t>());
				})));

				(new Stats())->run(looper);
				//app->setVisible(true);
				looper->run(true);
			}
		}

		return 0;
	}
};