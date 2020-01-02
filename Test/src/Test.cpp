// Test.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

//#define WIN32_LEAN_AND_MEAN

//#include <windows.h>
//#include <tchar.h>
//#include <string>
//#include "shellapi.h"

//#include <iostream>

#include "Aurora.h"

using namespace aurora;
using namespace aurora::events;
using namespace aurora::nodes;
using namespace aurora::modules;
using namespace aurora::modules::graphics;
using namespace aurora::modules::inputs;
//using namespace aurora::renderers;
//using namespace aurora::renderers::commands;

#include "../../Extensions/Files/Images/PNGConverter/src/PNGConverter.h"
/*
int main() {
	createWindow();
	std::cout << "Hello World!\n";
}
*/

/*
void RenderScene() {
	g_D3DDevice->Clear(0, nullptr, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);
	g_D3DDevice->BeginScene();
	// 3D图形数据
	g_D3DDevice->EndScene();

	// 显示backbuffer内容到屏幕
	g_D3DDevice->Present(nullptr, nullptr, nullptr, nullptr);
}
*/

#include <unordered_map>
#include <vector>
#include <functional>
#include <atomic>
#include <thread>
#include <assert.h>
#include <mutex>
#include <any>
#include <fstream>
#include <bitset>


uint32_t value = 0;
RWAtomicLock<false, false> m;
//std::atomic<uint32_t> value(0);
std::atomic<bool> ready(false);    // can be checked without being set
std::atomic_flag winner = ATOMIC_FLAG_INIT;    // always set when checked

int iv = 0;

int fn(int a) {
	iv += a;
	return iv;
}

void zzzzzzzz() {
	for (int i = 0; i < 10; ++i) {
		m.writeLock();
		++value;
		m.writeUnlock();
	}
}

void zzzzzzzz2() {
	for (int i = 0; i < 10; ++i) {
		m.readLock();
		auto z = value;
		m.readUnlock();
	}
}

void count1m(int id) {
	while (!ready) {
		std::this_thread::yield();
	}

	for (int i = 0; i < 100; ++i) {
		m.writeLock();
		zzzzzzzz();
		m.writeUnlock();
	}
};

void count2m(int id) {
	while (!ready) {
		std::this_thread::yield();
	}

	for (int i = 0; i < 100; ++i) {
		m.readLock();
		zzzzzzzz2();
		m.readUnlock();
	}
};

template<typename T = void>
class C0 {
public:
	C0(void(*l)()) :
		_target((uint32_t)l) {
	}

	template<bool b>
	void call() {
		println("other");
		((void(*)())_target)();
	}

	template<>
	void call<true>() {
		println("void");
		((void(*)())_target)();
	}

	void operator()() {
		println("T is void : ", sizeof(T));
		//call<std::is_void_v<T>>();
	}

protected:
	bool bbb = std::is_void_v<T>;
	uint64_t _target;
};

class ICommand {
public:
	virtual ~ICommand() {};
	void execute() {
		_fn(this);
	}

	void* operator new(size_t size) {
		println("new 0");
		return malloc(size);
	}

protected:
	ICommand() {};
	void(*_fn)(void*) = nullptr;
};

template<typename T>
class AbstractCommand : public ICommand {
public:
	AbstractCommand() {
		_fn = &T::execute;
	}
	void* operator new(size_t size) {
		println("new 1");
		return malloc(size);
	}
protected:
	
	virtual ~AbstractCommand() {};
};

class Test1Command : public AbstractCommand<Test1Command>{
public:
	Test1Command(int a) :
		_v(a) {
	}

	void* operator new(size_t size) {
		println("new 2");
		return malloc(size);
	}
protected:
	int _v = 1;
public:
	static void execute(void* cmd) {
		println("%d", ((Test1Command*)cmd)->_v);
	}
};

int tttt = 0;

template<typename M>
class PPPP {
public:
};

ByteArray readFile(const std::string& path) {
	ByteArray dst;
	std::ifstream stream(path, std::ios::in | std::ios::binary);
	if (stream.good()) {
		auto beg = stream.tellg();
		stream.seekg(0, std::ios::end);
		auto end = stream.tellg();
		auto size = end - beg;

		auto data = new uint8_t[size];

		stream.seekg(0, std::ios::beg);
		stream.read((char*)data, size);

		dst = ByteArray(data, size, ByteArray::Usage::EXCLUSIVE);
	}
	stream.close();
	return std::move(dst);
}

ProgramSource readProgramSourcee(const std::string& path, ProgramStage type) {
	ProgramSource s;
	s.language = ProgramLanguage::HLSL;
	s.stage = type;
	s.data = readFile(path);
	return std::move(s);
}


char* printGamepadKey(GamepadKeyCode code) {
	switch (code) {
	case GamepadKeyCode::LEFT_STICK:
		return "left stick";
	case GamepadKeyCode::RIGHT_STICK:
		return "right stick";
	case GamepadKeyCode::LEFT_THUMB:
		return "left thumb";
	case GamepadKeyCode::RIGHT_THUMB:
		return "right thumb";
	case GamepadKeyCode::DPAD:
		return "dpad";
	case GamepadKeyCode::LEFT_SHOULDER:
		return "left shoulder";
	case GamepadKeyCode::RIGHT_SHOULDER:
		return "right shoulder";
	case GamepadKeyCode::LEFT_TRIGGER:
		return "left trigger";
	case GamepadKeyCode::RIGHT_TRIGGER:
		return "right trigger";
	case GamepadKeyCode::SELECT:
		return "select";
	case GamepadKeyCode::START:
		return "start";
	case GamepadKeyCode::A:
		return "a";
	case GamepadKeyCode::B:
		return "b";
	case GamepadKeyCode::X:
		return "x";
	case GamepadKeyCode::Y:
		return "y";
	case GamepadKeyCode::TOUCH_PAD:
		return "touch pad";
	default:
		return "undefined";
	}
}


template<typename T>
void aabbccdsd(const T& v) {
	bool b0 = std::is_same_v<T, i8 const*>;
	bool b1 = std::is_convertible_v<T, i8 const*>;
	bool b2 = std::is_pointer_v<T>;
	bool b3 = std::is_array_v<T>;
	auto ss = typeid(v).name();
	int a = 1;
}

class IINNN {
public:
	virtual std::string toString() {
		return "";
	}
};

template<typename EvtType, typename... Args>
class CB1 : public IEventListener<EvtType> {
public:
	virtual void AE_CALL onEvent(Event<EvtType>& e) override {
	}
};

template<typename EvtType>
class CB1<EvtType, EvtFn<EvtType>> : public IEventListener<EvtType> {
public:
	virtual void AE_CALL onEvent(Event<EvtType>& e) override {
	}
};

void CBBBBBB(Event<ApplicationEvent>& e) {
	int a = 1;
}

void CBBBBBB222(Event<ApplicationEvent>& e) {
	int a = 1;
}


template<typename T>
struct has_toString {
	template<typename P, std::string (P::*k)()> struct detector {};
	template<typename P> static char func(detector<P, &P::toString>*);
	template<typename P> static long func(...);
	static constexpr bool value = sizeof(func<T>(nullptr)) == sizeof(char);
};


template<typename A>
class Class1 {

};


[[deprecated]]
void mmmmmmm(const Matrix34& m) {

}


template<uint32_t Size>
struct UUII {
	using type = uint64_t;
};

template<>
struct UUII<2> {
	using type = uint16_t;
};

//#include <version>
#include <type_traits>
//#include <bit>

#if __cpp_char8_t
#define ASDF 1
#endif

#include <any>
#include <immintrin.h>
//#include <type_traits>

template<typename T>
struct isFunctor : std::false_type {

};

template<typename L, typename R, typename... Args>
struct isFunctor<R(L::*)(Args...)> : std::true_type {

};

template<typename L>
struct isLambda : isFunctor<decltype(&L::operator())> {
};

template<typename T, typename = std::enable_if<isLambda<T>::value>>
void ssssssss(T fn) {
	T fff = fn;
	fff();
}


template<typename EvtType, typename Fn, typename = std::enable_if_t<std::is_invocable_r<void, Fn, Event<EvtType>>::value, Fn>>
struct AAAAAA {
	using type = Event<EvtType>;
	using ffff = Fn;
};


template<typename EvtType, typename Fn, typename = std::enable_if_t<std::is_invocable_r<void, Fn, Event<EvtType>>::value, Fn>>
class AE_TEMPLATE_DLL EventListener11 : public IEventListener<EvtType> {
public:
	EventListener11(EvtType e, const Fn& fn) :
		_fn(fn) {}

	virtual void AE_CALL onEvent(Event<EvtType>& e) override {
		int a = 1;
		_fn(e);
	}
private:
	Fn _fn;
};



template<typename EvtType, typename Fn, typename = std::enable_if_t<std::is_invocable_r<void, Fn, Event<EvtType>>::value, Fn>>
EventListener11(EvtType, Fn) -> EventListener11<EvtType, Fn>;

template<typename EvtType, typename Fn>
using EventListener22 = EventListener11<EvtType, Fn>;

//template<typename Evt>
//using EventListener11 = EventListener11<Evt, int>;

template<typename Evt, typename Fn>
constexpr bool HHH = std::is_invocable_r<void, Fn, Event<Evt>>::value;


template<size_t Bytes>
decltype(1) testttt() {
	bool b = std::integral_constant<size_t, Bytes>::value >= std::integral_constant<size_t, 4>::value;
	return 0;
}

template<size_t Bytes>
inline constexpr f64 floatMax() {
	static_assert(false, "");
}
template<>
inline constexpr f64 floatMax<4>() {
	return (std::numeric_limits<f32>::max)();
}
template<>
inline constexpr f64 floatMax<8>() {
	return (std::numeric_limits<f64>::max)();
}

class CAAAAA {
public:
	void calllll(Event<ApplicationEvent>& e) {
	}
};


std::unordered_map<std::string, std::any> mappp;

template<typename T>
void adddd(const std::string& name, const T& val) {
	if constexpr (std::is_convertible_v<T, char const*> || std::is_same_v<T, std::string_view>) {
		adddd(name, std::string(val));
	} else {
		auto itr = mappp.find(name);
		if (itr == mappp.end()) {
			mappp.emplace(name, val);
		} else {
			itr->second = val;
		}
	}
}

#include <optional>

template<typename T>
std::optional<T> gettttt(const std::string& name) {
	auto itr = mappp.find(name);
	if (itr == mappp.end()) {
		return std::optional<T>();
	} else {
		if (itr->second.type() == typeid(T)) {
			return std::optional<T>(std::any_cast<T>(itr->second));
		} else {
			return std::optional<T>();
		}
	}
}



//int main(int argc, char* argv[]) {
//	HMODULE HIn = GetModuleHandle(NULL);
//	FreeConsole();
int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow) {
	/*
	abc
	*/

	auto pppp = new int*(new int(123));
	auto ppp2 = pppp;
	(*ppp2) = nullptr;

	//new EventListener11(1);

	//el->onEvent(Event<ApplicationEvent>(ApplicationEvent::CLOSING));
	

	/*
	__declspec(align(16)) float a[] = { 1.5, 2.5, 3.5, 4.5 };
	__declspec(align(16)) float b[] = { 1.2, 2.3, 3.4, 4.5 };
	__declspec(align(16)) float c[] = { 0.0, 0.0, 0.0, 0.0 };


	__m128 m128_a = _mm_load_ps(a);
	__m128 m128_b = _mm_load_ps(b);
	__m128 m128_c = _mm_add_ps(m128_a, m128_b);

	_mm_store_ps(c, m128_c);

	for (int i = 0; i < 4; i++) {
		printf("%f ", c[i]);
	}
	printf("\n");
	*/

	auto v1 = Vec2i32(1, 2, 3, 4, 5.0f);
	
	auto& vvv = v1.slice();

	///*
	auto t0 = Time::now();

	auto eeee = std::endian::native == std::endian::big;

	std::vector<std::thread> threads;
	for (int i = 1; i <= 200; ++i) threads.push_back(std::thread(count1m, i));
	for (int i = 1; i <= 200; ++i) threads.push_back(std::thread(count2m, i));
	ready = true;

	for (auto & th : threads) th.join();

	println(value, "  ", Time::now() - t0, "  ", sizeof(std::atomic<std::thread::id>));

	//std::unordered_map<int, C1> map;
	//map.emplace(1, C1());
	//map.emplace(std::piecewise_construct,
		//std::forward_as_tuple(1),
		//std::forward_as_tuple());
	Matrix44 m1;
	mmmmmmm(m1);


	//CB1 < ApplicationEvent, CBBBBBB> wegwergwerr;
	//fnnn.onEvent(Event<ApplicationEvent>(ApplicationEvent::CLOSING));

	//CB1 < ApplicationEvent, CBBBBBB222> fnnn22;
	//fnnn22.onEvent(Event<ApplicationEvent>(ApplicationEvent::CLOSING));

	auto monitors = Monitor::getMonitors();
	auto vms = monitors[0].getVideoModes();

	auto app = new Application(u8"TestApp", 1000. / 60.);
	RefPtr<Application> appGuard(app);

	SetDllDirectory(String::Utf8ToUnicode(app->getAppPath() + u8"extensions/").c_str());
	//LoadLibrary(String::Utf8ToUnicode(pathh + u8"extensions/PNG.dll").c_str());
	
	auto ttttt = u8"";
	Application::Style wndStype;
	wndStype.thickFrame = true;
	if (app->createWindow(wndStype, u8"", Box2i32(Vec2i32({ 100, 100 }), Vec2i32({ 800, 600 })), false)) {
		RefPtr<GraphicsModuleLoader> gml = new GraphicsModuleLoader();

		/*
		auto d3d11Loader = new GraphicsModuleLoader();
		d3d11Loader->load(u8"./modules/WinD3D11.dll");
		auto d3d11 = d3d11Loader->create(app);
		d3d11->createDevice();
		*/


		RefPtr<InputModuleLoader> directInputModuleLoader = new InputModuleLoader();
		directInputModuleLoader->load(u8"./modules/WinDirectInput.dll");
		RefPtr<InputModuleLoader> xInputModuleLoader = new InputModuleLoader();
		xInputModuleLoader->load(u8"./modules/WinXInput.dll");

		//if (gml->load(u8"./modules/WinGlew.dll")) {
		if (gml->load(u8"./modules/WinD3D11.dll")) {
			auto gpstml = new ModuleLoader<IProgramSourceTranslator>();
			gpstml->load(u8"./modules/ProgramSourceTranslator.dll");
			auto gpst = gpstml->create(&Args().add("dxc", "dxcompiler.dll"));
			//auto kkk = Args().add("a", std::string("bcd")).get<std::string>("a", nullptr);

			/*
			GraphicsProgramSource source;
			source.language = GraphicsProgramSourceLanguage::HLSL;
			source.type = GraphicsProgramType::VERTEX_PROGRAM;
			source.version = "6.0";
			source.data = readFile(u8"../Resources/vert.hlsl");
			auto s = gpst->translate(source, GraphicsProgramSourceLanguage::MSL, "");
			println("%s", s.data.getBytes());
			*/

			RefPtr<IGraphicsModule> graphics = gml->create(&Args().add("app", app).add("trans", gpst));
			std::vector<RefPtr<IInputModule>> inputModules;
			if (auto im = xInputModuleLoader->create(&Args().add("app", app)); im) inputModules.emplace_back(im);
			if (auto im = directInputModuleLoader->create(&Args().add("app", app)); im) inputModules.emplace_back(im);
			
			if (graphics) {
				println("Graphics Version : ", graphics->getVersion().c_str());

				std::vector<RefPtr<IInputDevice>> inputDevices;

				for (auto& im : inputModules) {
					im->getEventDispatcher().addEventListener(ModuleEvent::CONNECTED, new EventListener(std::function([&inputDevices, app](Event<ModuleEvent>& e) {
						auto getNumInputeDevice = [&inputDevices](DeviceType type) {
							uint32_t n = 0;
							for (auto& dev : inputDevices) {
								if (dev->getInfo().type == type) ++n;
							}
							return n;
						};

						auto info = e.getData<DeviceInfo>();
						if ((info->type & (DeviceType::KEYBOARD | DeviceType::GAMEPAD)) != DeviceType::UNKNOWN) {
							auto im = e.getTarget<IInputModule>();
							if (getNumInputeDevice(DeviceType::GAMEPAD) > 0) return;
							println("create device : ", (uint32_t)info->type, " guid size = ", info->guid.getSize());
							auto device = im->createDevice(info->guid);
							if (device) {
								device->getEventDispatcher().addEventListener(DeviceEvent::DOWN, new EventListener(std::function([app](Event<DeviceEvent>& e) {
									auto device = e.getTarget<IInputDevice>();
									switch (device->getInfo().type) {
									case DeviceType::KEYBOARD:
									{
										auto key = e.getData<Key>();
										if (key->code == (uint32_t)KeyboardVirtualKeyCode::KEY_ENTER) {
											f32 status;
											if (device->getKeyState((uint8_t)KeyboardVirtualKeyCode::KEY_RCTRL, &status, 1) && status != 0.f) {
												app->toggleFullscreen();
											}
										}

										break;
									}
									case DeviceType::GAMEPAD:
									{
										auto key = e.getData<Key>();
										println("gamepad down : ", printGamepadKey((GamepadKeyCode)key->code), "  ", key->value[0]);
										if (key->code == (uint32_t)GamepadKeyCode::CROSS) {
											device->setVibration(0.5f, 0.5f);
										}

										break;
									}
									}
									//println("input device down : %d  %f", key->code, key->value[0]);
								})));

								device->getEventDispatcher().addEventListener(DeviceEvent::UP, new EventListener(std::function([](Event<DeviceEvent>& e) {
									auto device = e.getTarget<IInputDevice>();
									switch (device->getInfo().type) {
									case DeviceType::KEYBOARD:
									{
										break;
									}
									case DeviceType::GAMEPAD:
									{
										auto key = e.getData<Key>();
										println("gamepad up : ", printGamepadKey((GamepadKeyCode)key->code), "  ", key->value[0]);
										if (key->code == (uint32_t)GamepadKeyCode::CROSS) {
											device->setVibration(0.0f, 0.0f);
										}

										break;
									}
									}
								})));

								device->getEventDispatcher().addEventListener(DeviceEvent::MOVE, new EventListener(std::function([](Event<DeviceEvent>& e) {
									switch (e.getTarget<IInputDevice>()->getInfo().type) {
									case DeviceType::MOUSE:
									{
										auto key = e.getData<Key>();
										if (key->code == 4) {
											//f32 curPos[2];
											//(e.getTarget<InputDevice>())->getKeyState(key->code, curPos, 2);
											println("input device move : ", key->code, key->value[0], key->value[1], key->value[2]);
										} else if (key->code == 1) {
											//println("input device wheel : %d   %f", key->code, *(f32*)key->value);
										}

										break;
									}
									case DeviceType::GAMEPAD:
									{
										auto key = e.getData<Key>();
										print("gamepad move : ", printGamepadKey((GamepadKeyCode)key->code), " ", key->value[0]);
										if (key->count > 1) print("  ", key->value[1]);
										println();

										break;
									}
									}

								})));

								inputDevices.emplace_back(device);
							}
						}
						println("input device connected : ", info->type);
					})));

					im->getEventDispatcher().addEventListener(ModuleEvent::DISCONNECTED, new EventListener(std::function([&inputDevices](Event<ModuleEvent>& e) {
						auto info = e.getData<DeviceInfo>();
						for (uint32_t i = 0, n = inputDevices.size(); i < n; ++i) {
							if (inputDevices[i]->getInfo().guid == info->guid) {
								inputDevices.erase(inputDevices.begin() + i);
								break;
							}
						}
						println("input device disconnected : ", info->type);
					})));
				}

				//auto vertexBuffer = graphics->createVertexBuffer();
				auto vertexBuffer = new MultipleVertexBuffer(*graphics, 3);
				if (vertexBuffer) {
					/*
					f32 vertices[] = {
						0.0f, 0.0f,
						0.0f, 0.5f,
						0.8f, 0.0f };
					vb->stroage(sizeof(vertices), vertices);
					vb->setFormat(VertexSize::TWO, VertexType::F32);
					*/
					///*
					f32 vertices[] = {
						0.f, 0.f,
						0.f, 1.f,
						1.f, 1.f,
						1.f, 0.f };
					vertexBuffer->create(sizeof(vertices), Usage::MAP_WRITE | Usage::UPDATE | Usage::PERSISTENT_MAP, vertices, sizeof(vertices));
					vertexBuffer->setFormat(VertexSize::TWO, VertexType::F32);
				}

				auto uvBuffer = graphics->createVertexBuffer();
				if (uvBuffer) {
					f32 uvs[] = {
						0.f, 1.f,
						0.f, 0.f,
						1.f, 0.f,
						1.f, 1.f };
					uvBuffer->create(sizeof(uvs), Usage::NONE, uvs, sizeof(uvs));
					uvBuffer->setFormat(VertexSize::TWO, VertexType::F32);
				}

				auto vbf = new VertexBufferFactory();
				vbf->add("POSITION0", vertexBuffer);
				vbf->add("TEXCOORD0", uvBuffer);

				auto cf = new ShaderParameterFactory();
				cf->add("red", new ShaderParameter(ShaderParameterUsage::EXCLUSIVE))->set(Vec4f32::ONE).setUpdated();
				cf->add("green", new ShaderParameter(ShaderParameterUsage::EXCLUSIVE))->set(Vec4f32::ONE).setUpdated();
				//cf->add("blue", new ShaderParameter())->set(Vec4f32::ONE).setUpdated();

				auto aabbccStruct = new ShaderParameterFactory();
				aabbccStruct->add("val1", new ShaderParameter(ShaderParameterUsage::EXCLUSIVE))->set(Vec4f32::ONE).setUpdated();
				f32 val2[] = { 1.0f, 1.0f };
				aabbccStruct->add("val2", new ShaderParameter(ShaderParameterUsage::EXCLUSIVE))->set<f32>(val2, sizeof(val2), sizeof(f32), true).setUpdated();
				aabbccStruct->add("val3", new ShaderParameter())->set(Vec4f32::ONE).setUpdated();
				cf->add("blue", new ShaderParameter())->set(aabbccStruct);

				auto texRes = graphics->createTexture2DResource();
				if (texRes) {
					auto texView = graphics->createTextureView();
					if (texView) texView->create(texRes, 0, -1, 0, -1);

					auto img0 = file::PNGConverter::parse(readFile(app->getAppPath() + u8"../../Resources/c4.png"));
					auto mipLevels = Image::calcMipLevels(img0->size);
					ByteArray mipsData0;
					std::vector<void*> mipsData0Ptr;
					img0->generateMips(img0->format, mipLevels, mipsData0, mipsData0Ptr);

					auto img1 = file::PNGConverter::parse(readFile(app->getAppPath() + u8"../../Resources/red.png"));
					ByteArray mipsData1;
					std::vector<void*> mipsData1Ptr;
					img1->generateMips(img1->format, mipLevels, mipsData1, mipsData1Ptr);

					mipsData0Ptr.insert(mipsData0Ptr.end(), mipsData1Ptr.begin(), mipsData1Ptr.end());

					texRes->create(img0->size, 0, 1, img0->format, Usage::UPDATE, mipsData0Ptr.data());

					auto pb = graphics->createPixelBuffer();
					if (pb) {
						//pb->create(img0->size.getMultiplies() * 4, Usage::MAP_WRITE | Usage::PERSISTENT_MAP, mipsData0Ptr.data()[0], img0->size.getMultiplies() * 4);
						//texRes->copyFrom(0, 0, Box2ui32(Vec2ui32(0, 0), Vec2ui32(4, 4)), pb);
					}

					//auto mapped = tex->map(Usage::CPU_WRITE);
					uint8_t texData[] = { 255, 0, 0, 255, 0, 255, 0, 255, 0, 0, 255, 255, 0, 0, 0, 255 };
					//texRes->write(0, Rect<uint32_t>(2, 3, 3, 4), texData);
					//tex->unmap();
					//texRes->map(0, 0, Usage::MAP_WRITE);
					//texRes->update(0, 0, Box2ui32(Vec2ui32(0, 0), Vec2ui32(2, 2)), texData);

					cf->add("texDiffuse", new ShaderParameter(ShaderParameterUsage::AUTO))->set(texView ? (ITextureViewBase*)texView : (ITextureViewBase*)texRes).setUpdated();
				}

				auto sam = graphics->createSampler();
				if (sam) {
					//sam->setMipLOD(0, 0);
					//sam->setAddress(SamplerAddressMode::WRAP, SamplerAddressMode::WRAP, SamplerAddressMode::WRAP);
					sam->setFilter(SamplerFilterOperation::NORMAL, SamplerFilterMode::POINT, SamplerFilterMode::POINT, SamplerFilterMode::POINT);
					cf->add("samLiner", new ShaderParameter(ShaderParameterUsage::AUTO))->set(sam).setUpdated();
				}

				auto ib = graphics->createIndexBuffer();
				if (ib) {
					uint16_t indices[] = {
						0, 1, 3,
						3, 1, 2 };
					ib->create(sizeof(indices), Usage::NONE, indices, sizeof(indices));
					ib->setFormat(IndexType::UI16);
				}

				auto p = graphics->createProgram();
				if (p) {
					p->upload(readProgramSourcee(app->getAppPath() + u8"../../Resources/vert.hlsl", ProgramStage::VS), readProgramSourcee(app->getAppPath() + u8"../../Resources/frag.hlsl", ProgramStage::PS));
				}

				RefPtr<IEventListener<ApplicationEvent>> appClosingListener = new EventListener(std::function([](Event<ApplicationEvent>& e) {
					//*e.getData<bool>() = true;

				}));

				auto& evtDispatcher = app->getEventDispatcher();

				std::atomic_uint32_t frameCount = 0;

				evtDispatcher.addEventListener(ApplicationEvent::TICKING, new EventListener(std::function([&frameCount, graphics, &inputModules, &inputDevices, vbf, cf, p, ib](Event<ApplicationEvent>& e) {
					++frameCount;
					auto app = e.getTarget<Application>();

					auto dt = f64(*e.getData<int64_t>());
					//println(dt);

					for (auto& im : inputModules) im->poll();
					//for (auto& dev : inputDevices) dev->poll(true);

					//app->setWindowTitle(String::toString(GetKeyboardType(0)) + "  " + String::toString(GetKeyboardType(1)) + "  " + String::toString(GetKeyboardType(2)));

					//println("%lf", dt);
					//app->setWindowTitle(String::toString(dt));

					if (++tttt >= 120) {
						tttt = 0;
						//app->toggleFullscreen();
					}

					graphics->beginRender();
					graphics->clear();

					{
						cf->get("red")->set(Vec4f32((f32)rand() / (f32)RAND_MAX, (f32)rand() / (f32)RAND_MAX)).setUpdated();

						auto vb = vbf->get("POSITION0");
						if (vb) {
							auto cycle = 20000;
							auto halfCycyle = f32(cycle / 2);
							auto t = Time::now<std::chrono::milliseconds>() % cycle;
							f32 vertices[] = {
								0.f, 0.f,
								0.f, 1.f,
								1.f, 1.f,
								1.f, 0.f };
							f32 v = t <= halfCycyle ? 1.f - f32(t) / halfCycyle : (f32(t) - halfCycyle) / halfCycyle;
							vertices[3] = v;
							if ((vb->map(Usage::MAP_WRITE | Usage::MAP_SWAP) & Usage::DISCARD) == Usage::DISCARD) {
								vb->write(0, vertices, sizeof(vertices));
							} else {
								vb->write(12, &v, 4);
							}
							//vb->update(12, &v, 4);
							vb->unmap();
						}
					}

					graphics->draw(vbf, p, cf, ib);

					graphics->endRender();
					graphics->present();
				})));

				evtDispatcher.addEventListener(ApplicationEvent::CLOSING, *appClosingListener);

				{
					std::thread([app, &frameCount]() {
						auto tw = std::make_shared<TimeWheel>(100, 100);
						tw->getEventDispatcher().addEventListener(TimeWheel::TimeWheelEvent::TRIGGER, new EventListener(std::function([](Event<TimeWheel::TimeWheelEvent>& e) {
							auto trigger = e.getData<TimeWheel::Trigger>();
							trigger->timer.doTick(trigger->tickID);
						})));

						auto frameTime = Time::now();
						TimeWheel::Timer timer;
						timer.getEventDispatcher().addEventListener(TimeWheel::TimerEvent::TICK, new EventListener(std::function([app, &frameCount, &frameTime](Event<TimeWheel::TimerEvent>& e) {
							auto t = Time::now();

							auto fps = frameCount / ((t - frameTime) * 0.001);
							println("fps : ", round(fps));

							frameCount = 0;
							frameTime = t;
						})));
						tw->startTimer(timer, 1000000, 0, false);

						auto t0 = Time::now<std::chrono::microseconds>();
						while (true) {
							auto t = Time::now<std::chrono::microseconds>();
							uint64_t d = t - t0;
							while (d) {
								uint64_t e;
								if (d > tw->getInterval()) {
									e = tw->getInterval();
									d -= tw->getInterval();
								} else {
									e = d;
									d = 0;
								}

								tw->tick(e);
							}
							t0 = t;

							std::this_thread::sleep_for(std::chrono::microseconds(100));
						}
					}).detach();
				}

				app->setVisible(true);
				app->run(true);
			}
		}
	}

	return 0;
}