#pragma once

#include "../BaseTester.h"
#include "srk/SerializableObject.h"

class GraphicsTester : public BaseTester {
public:
	virtual int32_t SRK_CALL run() override {
		auto monitors = Monitor::getMonitors();
		auto vms = monitors[0].getVideoModes();
		
		IntrusivePtr wml = new WindowModuleLoader();
		if (!wml->load(getWindowDllPath())) return 0;

		auto wm = wml->create(nullptr);
		if (!wm) return 0;

		CreateWindowDesc desc;
		desc.style.resizable = true;
		desc.contentSize.set(800, 600);
		auto win = wm->crerateWindow(desc);
		if (!win) return 0;

		IntrusivePtr gml = new GraphicsModuleLoader();

		//if (!gml->load(getDllPath("srk-module-graphics-d3d11"))) return 0;
		//if (!gml->load(getDllPath("srk-module-graphics-d3d12"))) return 0;
		//if (!gml->load(getDllPath("srk-module-graphics-gl"))) return 0;
		if (!gml->load(getDllPath("srk-module-graphics-vulkan"))) return 0;

		SerializableObject args;

		IntrusivePtr stml = new ModuleLoader<IShaderTranspiler>();
		stml->load(getDllPath("srk-module-graphics-shader-transpiler"));

		args.insert("dxc", getDllPath("dxcompiler"));
		auto st = stml->create(&args);

		args.insert("win", win.uintptr());
		args.insert("sampleCount", 4);
		args.insert("transpiler", st.uintptr());
		//args.insert("driverType", "SOFTWARE");
		args.insert("debug", Environment::IS_DEBUG);

		auto graphics = gml->create(&args);
		if (!graphics) return 0;

		IntrusivePtr looper = new Looper(1000.0 / 60.0);

		printaln("Graphics Version : ", graphics->getVersion());

		{
			auto bs = graphics->createBlendState();
			printaln("BlendState : ", bs == nullptr ? "failed" : "succeed");

			auto dss = graphics->createDepthStencilState();
			printaln("DepthStencilState : ", dss == nullptr ? "failed" : "succeed");

			auto rs = graphics->createRasterizerState();
			printaln("RasterizerState : ", rs == nullptr ? "failed" : "succeed");

			IntrusivePtr shader = new Shader();
			auto shaderResourcesFolder = getAppPath().parent_path().u8string() + "/Resources/shaders/";
			extensions::ShaderScript::set(shader, graphics, readFile(getAppPath().parent_path().u8string() + "/Resources/shaders/lighting.shader"),
				[shaderResourcesFolder](const Shader& shader, ProgramStage stage, const std::string_view& name) {
					return readFile(shaderResourcesFolder + name);
				},
				[](const Shader& shader, const std::string_view& name) {
					return modules::graphics::IProgram::InputDescriptor();
				});

			auto program = shader->select(nullptr);
			printaln("Program : ", program == nullptr ? "failed" : "succeed");
			if (program) {
				auto& info = program->getInfo();
				graphics->draw(nullptr, program, nullptr, nullptr);
			}
		}

		graphics->getEventDispatcher()->addEventListener(GraphicsEvent::ERR, createEventListener<GraphicsEvent>([](Event<GraphicsEvent>& e) {
			printaln(*(std::string_view*)e.getData());
			int a = 1;
			}));

		win->getEventDispatcher()->addEventListener(WindowEvent::RESIZED, createEventListener<WindowEvent>([graphics](Event<WindowEvent>& e) {
			graphics->setBackBufferSize(((IWindow*)e.getTarget())->getContentSize());
			}));

		win->getEventDispatcher()->addEventListener(WindowEvent::CLOSING, createEventListener<WindowEvent>([](Event<WindowEvent>& e) {
			//*e.getData<bool>() = true;
			}));

		win->getEventDispatcher()->addEventListener(WindowEvent::CLOSED, createEventListener<WindowEvent>([looper](Event<WindowEvent>& e) {
			looper->stop();
			}));

		win->getEventDispatcher()->addEventListener(WindowEvent::RESIZED, createEventListener<WindowEvent>([graphics](Event<WindowEvent>& e) {
			}));

		looper->getEventDispatcher()->addEventListener(LooperEvent::TICKING, createEventListener<LooperEvent>([graphics](Event<LooperEvent>& e) {
			}));

		//(new Stats())->run(looper);
		win->setVisible(true);
		looper->run(true);

		return 0;
	}
};