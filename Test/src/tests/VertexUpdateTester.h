#pragma once

#include "../BaseTester.h"

class VertexUpdateTester : public BaseTester {
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

		//if (!gml->load("libs/" + getDLLName("srk-module-graphics-gl"))) return 0;
		if (!gml->load(getDllPath("srk-module-graphics-d3d11")))  return 0;

		SerializableObject args;

		IntrusivePtr stml = new ModuleLoader<IShaderTranspiler>();
		stml->load(getDllPath("srk-module-graphics-shader-transpiler"));

		args.insert("dxc", getDllPath("dxcompiler"));
		auto st = stml->create(&args);

		args.insert("win", win.uintptr());
		args.insert("sampleCount", 4);
		args.insert("transpiler", st.uintptr());
		args.insert("debug", Environment::IS_DEBUG);

		auto graphics = gml->create(&args);
		if (!graphics) return 0;

		printaln("Graphics Version : ", graphics->getVersion());

		graphics->getEventDispatcher()->addEventListener(GraphicsEvent::ERR, createEventListener<GraphicsEvent>([](Event<GraphicsEvent>& e) {
			printaln(*(std::string_view*)e.getData());
			int a = 1;
			}));

		win->getEventDispatcher()->addEventListener(WindowEvent::CLOSED, createEventListener<WindowEvent>([](Event<WindowEvent>& e) {
			std::exit(0);
			}));

		win->getEventDispatcher()->addEventListener(WindowEvent::RESIZED, createEventListener<WindowEvent>([graphics](Event<WindowEvent>& e) {
			graphics->setBackBufferSize(((IWindow*)e.getTarget())->getContentSize());
			}));

		struct {
			IntrusivePtr<IWindowModule> winModule;
			IntrusivePtr<IWindow> win;
			IntrusivePtr<Looper> looper;
			IntrusivePtr<IGraphicsModule> g;
			IntrusivePtr<VertexAttributeCollection> vbf;
			IntrusivePtr<ShaderParameterCollection> spc;
			IntrusivePtr<IProgram> p;
			IntrusivePtr<IIndexBuffer> ib;
			IntrusivePtr<IBlendState> bs;
			IntrusivePtr<IDepthStencilState> dss;
		} renderData;
		renderData.winModule = wm;
		renderData.win = win;
		renderData.looper = new Looper(1.0 / 60.0);
		renderData.g = graphics;

		{
			auto rs = graphics->createRasterizerState();
			rs->setFillMode(FillMode::SOLID);
			rs->setFrontFace(FrontFace::CW);
			rs->setCullMode(CullMode::NONE);
			graphics->setRasterizerState(&*rs);
		}

		renderData.dss = graphics->createDepthStencilState();
		renderData.vbf = new VertexAttributeCollection();
		renderData.spc = new ShaderParameterCollection();

		{
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
				float32_t vertices[] = {
					0.f, 0.f,
					0.f, 1.f,
					1.f, 1.f,
					1.f, 0.f };
				auto hr = vertexBuffer->create(sizeof(vertices), Usage::MAP_WRITE | Usage::PERSISTENT_MAP, vertices, sizeof(vertices));
				vertexBuffer->setStride(2 * sizeof(float32_t));
			}

			auto uvBuffer = graphics->createVertexBuffer();
			if (uvBuffer) {
				float32_t uvs[] = {
					0.f, 1.f,
					0.f, 0.f,
					1.f, 0.f,
					1.f, 1.f };
				uvBuffer->create(sizeof(uvs), Usage::NONE, uvs, sizeof(uvs));
				uvBuffer->setStride(2 * sizeof(float32_t));
			}

			renderData.vbf->set("POSITION0", VertexAttribute<IVertexBuffer>(vertexBuffer, 2, VertexType::F32, 0));
			renderData.vbf->set("TEXCOORD0", VertexAttribute<IVertexBuffer>(uvBuffer, 2, VertexType::F32, 0));
		}

		{
			renderData.spc->set("red", new ShaderParameter(ShaderParameterUsage::EXCLUSIVE))->set(Vec4f32::ONE).setUpdated();
			renderData.spc->set("green", new ShaderParameter(ShaderParameterUsage::EXCLUSIVE))->set(Vec4f32::ONE).setUpdated();
			//cf->add("blue", new ShaderParameter())->set(Vec4f32::ONE).setUpdated();

			auto aabbccStruct = new ShaderParameterCollection();
			aabbccStruct->set("val1", new ShaderParameter(ShaderParameterUsage::EXCLUSIVE))->set(Vec4f32::ONE).setUpdated();
			float32_t val2[] = { 1.0f, 1.0f };
			aabbccStruct->set("val2", new ShaderParameter(ShaderParameterUsage::EXCLUSIVE))->set<float32_t>(val2, sizeof(val2), sizeof(float32_t), true).setUpdated();
			aabbccStruct->set("val3", new ShaderParameter())->set(Vec4f32::ONE).setUpdated();
			renderData.spc->set("blue", new ShaderParameter())->set(aabbccStruct);
		}

		renderData.bs = graphics->createBlendState();
		{
			//bs->setIndependentBlendEnabled(true);
			RenderTargetBlendState rtbs;
			rtbs.enabled = false;
			//rtbs.func.set(BlendFactor::ZERO, BlendFactor::ONE);
			rtbs.writeMask.set(true, true, true, true);
			renderData.bs->setRenderTargetState(0, rtbs);
		}

		{
			auto texRes = graphics->createTexture2DResource();
			if (texRes) {
				auto img0 = extensions::PNGConverter::decode(readFile(getAppPath().parent_path().u8string() + "/Resources/c4.png"));
				auto mipLevels = Image::calcMipLevels(img0->size);
				ByteArray mipsData0;
				std::vector<void*> mipsData0Ptr;
				img0->generateMips(img0->format, mipLevels, mipsData0, mipsData0Ptr);

				auto img1 = extensions::PNGConverter::decode(readFile(getAppPath().parent_path().u8string() + "/Resources/red.png"));
				ByteArray mipsData1;
				std::vector<void*> mipsData1Ptr;
				img1->generateMips(img1->format, mipLevels, mipsData1, mipsData1Ptr);

				mipsData0Ptr.insert(mipsData0Ptr.end(), mipsData1Ptr.begin(), mipsData1Ptr.end());

				texRes->create(img0->size, 0, 1, 1, img0->format, Usage::UPDATE, mipsData0Ptr.data());

				auto texView = graphics->createTextureView();
				texView->create(texRes, 0, -1, 0, -1);

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

				renderData.spc->set("texDiffuse", new ShaderParameter(ShaderParameterUsage::AUTO))->set(texView).setUpdated();
			}

			auto sam = graphics->createSampler();
			if (sam) {
				//sam->setMipLOD(0, 0);
				//sam->setAddress(SamplerAddressMode::WRAP, SamplerAddressMode::WRAP, SamplerAddressMode::WRAP);
				sam->setFilter(SamplerFilterOperation::NORMAL, SamplerFilterMode::POINT, SamplerFilterMode::POINT, SamplerFilterMode::POINT);
				renderData.spc->set("samLiner", new ShaderParameter(ShaderParameterUsage::AUTO))->set(sam).setUpdated();
			}
		}

		renderData.ib = graphics->createIndexBuffer();
		{
			uint16_t indices[] = {
					0, 1, 3,
					3, 1, 2,

					4, 5, 7,
					7, 5, 6 };
			renderData.ib->create(sizeof(indices), Usage::NONE, indices, sizeof(indices));
			renderData.ib->setFormat(IndexType::UI16);
		}

		renderData.p = graphics->createProgram();
		createProgram(*renderData.p, "vert.hlsl", "frag.hlsl");

		win->getEventDispatcher()->addEventListener(WindowEvent::CLOSING, new EventListener(std::function([](Event<WindowEvent>& e) {
			//*e.getData<bool>() = true;
			})));

		win->getEventDispatcher()->addEventListener(WindowEvent::CLOSED, new EventListener(std::function([renderData](Event<WindowEvent>& e) {
			renderData.looper->stop();
			})));

		renderData.looper->getEventDispatcher()->addEventListener(LooperEvent::TICKING, new EventListener(std::function([renderData](Event<LooperEvent>& e) {
			auto dt = float64_t(*e.getData<int64_t>());

			while (renderData.winModule->processEvent()) {};
			//println(dt);

			//for (auto& im : inputModules) im->poll();
			//for (auto& dev : inputDevices) dev->poll(true);

			//app->setWindowTitle(String::toString(GetKeyboardType(0)) + "  " + String::toString(GetKeyboardType(1)) + "  " + String::toString(GetKeyboardType(2)));

			//println("%lf", dt);
			//app->setWindowTitle(String::toString(dt));

			renderData.g->setViewport(Box2i32ui32(Vec2i32::ZERO, renderData.win->getContentSize()));
			renderData.g->beginRender();
			renderData.g->clear(ClearFlag::COLOR | ClearFlag::DEPTH | ClearFlag::STENCIL, Vec4f32(0.0f, 0.0f, 0.25f, 1.0f), 1.f, 0);

			if (true) {
				renderData.spc->get("red")->set(Vec4f32((float32_t)rand() / (float32_t)RAND_MAX, (float32_t)rand() / (float32_t)RAND_MAX)).setUpdated();

				if (auto va = renderData.vbf->get("POSITION0"); va) {
					if (auto& vb = va->resource; vb) {
						auto cycle = 20000;
						auto halfCycyle = float32_t(cycle / 2);
						auto t = srk::Time::now<std::chrono::milliseconds>() % cycle;
						float32_t vertices[] = {
							0.f, 0.f,
							0.f, 1.f,
							1.f, 1.f,
							1.f, 0.f };
						float32_t v = t <= halfCycyle ? 1.f - float32_t(t) / halfCycyle : (float32_t(t) - halfCycyle) / halfCycyle;
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
			}

			renderData.g->setBlendState(renderData.bs);
			renderData.g->draw(renderData.vbf, renderData.p, renderData.spc, renderData.ib);

			renderData.g->endRender();
			renderData.g->present();
			})));

		(new Stats())->run(renderData.looper);
		win->setVisible(true);
		renderData.looper->run(true);

		return 0;
	}
};