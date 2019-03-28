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
//using namespace aurora::renderers;
//using namespace aurora::renderers::commands;

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


ui32 value = 0;
RecursiveAtomicLock m;
//std::atomic<ui32> value(0);
std::atomic<bool> ready(false);    // can be checked without being set
std::atomic_flag winner = ATOMIC_FLAG_INIT;    // always set when checked

int iv = 0;

int fn(int a) {
	iv += a;
	return iv;
}

void zzzzzzzz() {
	m.lock();
	for (int i = 0; i < 10000; ++i) {
		++value;
	} // 计数.
	m.unlock();
}

void count1m(int id) {
	while (!ready) {
		std::this_thread::yield();
	}

	m.lock();
	for (int i = 0; i < 100; ++i) {
		zzzzzzzz();
	} // 计数.
	m.unlock();
	// 如果某个线程率先执行完上面的计数过程，则输出自己的 ID.
	// 此后其他线程执行 test_and_set 是 if 语句判断为 false，
	// 因此不会输出自身 ID.
};

template<typename T = void>
class C0 {
public:
	C0(void(*l)()) :
		_target((ui32)l) {
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
		println("T is void : %d", sizeof(T));
		//call<std::is_void_v<T>>();
	}

protected:
	bool bbb = std::is_void_v<T>;
	ui64 _target;
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
	decltype(M::CREATE_PARAMP()) a;
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	///*
	std::vector<std::thread> threads;
	for (int i = 1; i <= 8; ++i)
		threads.push_back(std::thread(count1m, i));
	ready = true;

	for (auto & th : threads) th.join();
	//*/

	//std::unordered_map<int, C1> map;
	//map.emplace(1, C1());
	//map.emplace(std::piecewise_construct,
		//std::forward_as_tuple(1),
		//std::forward_as_tuple());

	Test1Command cmd1(123);
	ICommand* cmd = &cmd1;

	std::list<int> list;
	list.emplace_back(0);

	for (auto itr = list.begin(); itr != list.end(); ++itr) {
		auto& zzz = *itr;
	}

	new Test1Command(111);

	cmd->execute();

	auto monitors = Monitor::getMonitors();
	auto vms = monitors[0].getVideoModes();

	auto app = new Application(u8"TestApp", 1000000000. / 60., EventDispatcher<ApplicationEvent, EmptyLock>::DEFAULT_ALLOCATOR);
	RefGuard appGuard(app);
	
	Application::Style wndStype;
	wndStype.thickFrame = true;
	if (app->createWindow(wndStype, u8"", Rect<i32>(100, 100, 900, 700), false)) {
		auto gml = new GraphicsModuleLoader();
		RefGuard moduleGLGuard(gml);

		auto kbl = new InputModuleLoader();
		RefGuard moduleKBGuard(kbl);
		if (gml->load(u8"./modules/WinGlew.dll") && kbl->load(u8"./modules/WinKeyboard.dll")) {
			GraphicsModule* graphics = gml->create(GraphicsModule::CREATE_PARAMS{ app });
			InputModule* kb = kbl->create(InputModule::CREATE_PARAMS{ app, &EventDispatcher<InputEvent, EmptyLock>::DEFAULT_ALLOCATOR });
			if (graphics && kb) {
				RefGuard graphicsGuard(graphics);
				RefGuard kbGuard(kb);

				kb->getEventDispatcher()->addEventListener(InputEvent::DOWN, *new EventListenerFunc<InputEvent>(std::bind([](Event<InputEvent>& e) {
					auto key = (InputKey*)e.getData();
					println("key down : %d   %f", key->code, key->value);
				}, std::placeholders::_1)), true);

				kb->getEventDispatcher()->addEventListener(InputEvent::UP, *new EventListenerFunc<InputEvent>(std::bind([](Event<InputEvent>& e) {
					auto key = (InputKey*)e.getData();
					println("key up : %d   %f", key->code, key->value);
				}, std::placeholders::_1)), true);

				if (graphics->createDevice()) {
					auto vb = graphics->createVertexBuffer();
					float vertices[6] = {
						0.0f, 0.0f,
						0.0f, 0.5f,
						0.8f, 0.0f};
					vb->stroage(24, vertices);

					auto p = graphics->createProgram();

					auto appUpdateListener = new EventListenerFunc<ApplicationEvent>(std::bind([graphics, kb, vb, p](Event<ApplicationEvent>& e) {
						auto app = (Application*)e.getTarget();

						auto dt = f64(*((i64*)e.getData())) * 0.000001;

						app->pollEvents();
						kb->pollEvents();

						//println("%lf", dt);
						//app->setWindowTitle(String::toString(dt));

						if (++tttt >= 120) {
							tttt = 0;
							//app->toggleFullscreen();
						}

						graphics->beginRender();
						graphics->clear();

						vb->use();
						p->use();

						graphics->endRender();
						graphics->present();
					}, std::placeholders::_1));
					RefGuard appUpdateListenerGuard(appUpdateListener);

					auto appClosingListener = new EventListenerFunc<ApplicationEvent>(std::bind([](Event<ApplicationEvent>& e) {
						//*(bool*)e.getData() = true;
						
					}, std::placeholders::_1));
					RefGuard appClosingListenerGuard(appClosingListener);

					auto evtDispatcher = app->getEventDispatcher();

					evtDispatcher->addEventListener(ApplicationEvent::UPDATE, *appUpdateListener, true);
					evtDispatcher->addEventListener(ApplicationEvent::CLOSING, *appClosingListener, true);
					app->setVisible(true);
					app->run();
				}
			}
		}
	}

	return 0;
}