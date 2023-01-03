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

		graphics->getEventDispatcher()->addEventListener(GraphicsEvent::ERR, createEventListener<GraphicsEvent>([](Event<GraphicsEvent>& e) {
			printaln(*(std::string_view*)e.getData());
		int a = 1;
			}));

		IntrusivePtr looper = new Looper(1000.0 / 60.0);

		printaln("Graphics Version : ", graphics->getVersion());

		{
			auto bs = graphics->createBlendState();
			printaln("createBlendState : ", bs ? "succeed" : "failed");

			auto dss = graphics->createDepthStencilState();
			printaln("createDepthStencilState : ", dss ? "succeed" : "failed");

			auto rs = graphics->createRasterizerState();
			printaln("createRasterizerState : ", rs ? "succeed" : "failed");

			auto vb = graphics->createVertexBuffer();
			printaln("createVertexBuffer : ", vb ? "succeed" : "failed");
			if (vb) {
				auto rst = vb->create(12, Usage::MAP_READ_WRITE);
				printaln("VertexBuffer::create : ", rst ? "succeed" : "failed");
				if (rst) {
					auto usage = vb->map(Usage::MAP_WRITE);
					printaln("VertexBuffer::map : ", usage != Usage::NONE ? "succeed" : "failed");
					float32_t wbuf[] = { 1.0f, 2.0f, 3.0f };
					constexpr auto n = std::extent_v<decltype(wbuf)>;
					auto wsize = vb->write(wbuf, sizeof(wbuf), 0);
					printaln("VertexBuffer::write : ", wsize != -1 ? "succeed" : "failed");
					vb->unmap();

					float32_t rbuf[n];
					vb->map(Usage::MAP_READ);
					auto rsize = vb->read(rbuf, sizeof(rbuf), 0);
					vb->unmap();
					auto isSame = rsize != -1 && wsize == rsize;
					for (size_t i = 0; i < n; ++i) {
						if (wbuf[i] != rbuf[i]) {
							isSame = false;
							break;
						}
					}
					printaln("VertexBuffer::read : ", isSame ? "succeed" : "failed");

					vb->create(12, Usage::UPDATE);
					auto upsize = vb->update(wbuf, sizeof(wbuf), 0);
					printaln("VertexBuffer::update : ", upsize != -1 && upsize == sizeof(wbuf) ? "succeed" : "failed");

					vb->create(12, Usage::COPY_SRC, wbuf, sizeof(wbuf));

					auto vb2 = graphics->createVertexBuffer();
					vb2->create(12, Usage::COPY_DST | Usage::MAP_READ);
					auto cpysize = vb2->copyFrom(0, vb, Box1uz(Vec1uz(0), Vec1uz(vb->getSize())));
					vb2->map(Usage::MAP_READ);
					rsize = vb2->read(rbuf, sizeof(rbuf), 0);
					vb2->unmap();
					isSame = cpysize != -1 && rsize != -1 && wsize == cpysize && wsize == rsize;
					for (size_t i = 0; i < n; ++i) {
						if (wbuf[i] != rbuf[i]) {
							isSame = false;
							break;
						}
					}
					printaln("VertexBuffer::copyFrom : ", isSame ? "succeed" : "failed");
				}
			}

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
			printaln("Program : ", program ? "succeed" : "failed");
			if (program) {
				auto& info = program->getInfo();
				IntrusivePtr vac = new VertexAttributeCollection();

				auto pos = graphics->createVertexBuffer();
				float32_t posData[] = { 1.0f, 2.0f, 3.0f, 1.0f, 2.0f, 3.0f, 1.0f, 2.0f,
					1.0f, 2.0f, 3.0f, 1.0f, 2.0f, 3.0f, 1.0f, 2.0f,
					1.0f, 2.0f, 3.0f, 1.0f, 2.0f, 3.0f, 1.0f, 2.0f };
				pos->create(96, Usage::NONE, posData, 96);
				pos->setStride(32);
				vac->set(ShaderPredefine::POSITION0, VertexAttribute<IVertexBuffer>(pos, 3, VertexType::F32, 0));
				vac->set(ShaderPredefine::NORMAL0, VertexAttribute<IVertexBuffer>(pos, 3, VertexType::F32, 12));
				vac->set(ShaderPredefine::UV0, VertexAttribute<IVertexBuffer>(pos, 2, VertexType::F32, 24));

				IntrusivePtr spc = new ShaderParameterCollection();
				spc->set(ShaderPredefine::MATRIX_LW, new ShaderParameter())->set(Matrix3x4f32());
				spc->set(ShaderPredefine::MATRIX_LP, new ShaderParameter())->set(Matrix4x4f32());
				spc->set(ShaderPredefine::AMBIENT_COLOR, new ShaderParameter())->set(Vec3f32());
				spc->set(ShaderPredefine::DIFFUSE_COLOR, new ShaderParameter())->set(Vec3f32::ONE);
				spc->set(ShaderPredefine::SPECULAR_COLOR, new ShaderParameter())->set(Vec3f32::ONE);

				graphics->draw(program, vac, spc, nullptr);
			}
		}

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