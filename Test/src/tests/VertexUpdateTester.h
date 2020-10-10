#pragma once

#include "../BaseTester.h"

class VertexUpdateTester : public BaseTester {
public:
	virtual int32_t AE_CALL run() override {
		auto monitors = Monitor::getMonitors();
		auto vms = monitors[0].getVideoModes();

		RefPtr app = new Application("TestApp");

		ApplicationStyle wndStype;
		wndStype.thickFrame = true;
		if (app->createWindow(wndStype, "", Vec2ui32(800, 600), false)) {
			RefPtr gml = new GraphicsModuleLoader();

			//if (gml->load("libs/" + getDLLName("ae-graphics-gl"))) {
			if (gml->load("libs/" + getDLLName("ae-graphics-d3d11"))) {
				SerializableObject args;

				RefPtr gpstml = new ModuleLoader<IProgramSourceTranslator>();
				gpstml->load("libs/" + getDLLName("ae-program-source-translator"));

				args.insert("dxc", "libs/" + getDLLName("dxcompiler"));
				auto gpst = gpstml->create(&args);

				args.insert("app", (uintptr_t)&*app);
				args.insert("sampleCount", 4);
				args.insert("trans", (uintptr_t)&*gpst);
#ifdef AE_DEBUG
				args.insert("debug", true);
#endif
				auto graphics = gml->create(&args);

				if (graphics) {
					printaln("Graphics Version : ", graphics->getVersion());

					graphics->getEventDispatcher().addEventListener(GraphicsEvent::ERR, createEventListener<GraphicsEvent>([](Event<GraphicsEvent>& e) {
						printaln(*(std::string_view*)e.getData());
						int a = 1;
					}));

					app->getEventDispatcher().addEventListener(ApplicationEvent::RESIZED, createEventListener<ApplicationEvent>([graphics](Event<ApplicationEvent>& e) {
						graphics->setBackBufferSize(((IApplication*)e.getTarget())->getCurrentClientSize());
					}));

					struct {
						RefPtr<Application> app;
						RefPtr<Looper> looper;
						RefPtr<IGraphicsModule> g;
						RefPtr<VertexBufferCollection> vbf;
						RefPtr<ShaderParameterCollection> spc;
						RefPtr<IProgram> p;
						RefPtr<IIndexBuffer> ib;
						RefPtr<IBlendState> bs;
						RefPtr<IDepthStencilState> dss;
					} renderData;
					renderData.app = app;
					renderData.looper = new Looper(1000.0 / 60.0);
					renderData.g = graphics;

					{
						auto rs = graphics->createRasterizerState();
						rs->setFillMode(FillMode::SOLID);
						rs->setFrontFace(FrontFace::CW);
						rs->setCullMode(CullMode::NONE);
						graphics->setRasterizerState(&*rs);
					}

					renderData.dss = graphics->createDepthStencilState();
					renderData.vbf = new VertexBufferCollection();
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
							vertexBuffer->setFormat(VertexFormat(VertexSize::TWO, VertexType::F32));
						}

						auto uvBuffer = graphics->createVertexBuffer();
						if (uvBuffer) {
							float32_t uvs[] = {
								0.f, 1.f,
								0.f, 0.f,
								1.f, 0.f,
								1.f, 1.f };
							uvBuffer->create(sizeof(uvs), Usage::NONE, uvs, sizeof(uvs));
							uvBuffer->setFormat(VertexFormat(VertexSize::TWO, VertexType::F32));
						}

						renderData.vbf->set("POSITION0", vertexBuffer);
						renderData.vbf->set("TEXCOORD0", uvBuffer);
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
							auto img0 = extensions::PNGConverter::parse(readFile(app->getAppPath().parent_path().u8string() + "/Resources/c4.png"));
							auto mipLevels = Image::calcMipLevels(img0->size);
							ByteArray mipsData0;
							std::vector<void*> mipsData0Ptr;
							img0->generateMips(img0->format, mipLevels, mipsData0, mipsData0Ptr);

							auto img1 = extensions::PNGConverter::parse(readFile(app->getAppPath().parent_path().u8string() + "/Resources/red.png"));
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

					app->getEventDispatcher().addEventListener(ApplicationEvent::CLOSING, new EventListener(std::function([](Event<ApplicationEvent>& e) {
						//*e.getData<bool>() = true;
					})));

					app->getEventDispatcher().addEventListener(ApplicationEvent::CLOSED, new EventListener(std::function([renderData](Event<ApplicationEvent>& e) {
						renderData.looper->stop();
					})));

					renderData.looper->getEventDispatcher().addEventListener(LooperEvent::TICKING, new EventListener(std::function([renderData](Event<LooperEvent>& e) {
						auto dt = float64_t(*e.getData<int64_t>());

						renderData.app->pollEvents();
						//println(dt);

						//for (auto& im : inputModules) im->poll();
						//for (auto& dev : inputDevices) dev->poll(true);

						//app->setWindowTitle(String::toString(GetKeyboardType(0)) + "  " + String::toString(GetKeyboardType(1)) + "  " + String::toString(GetKeyboardType(2)));

						//println("%lf", dt);
						//app->setWindowTitle(String::toString(dt));

						renderData.g->setViewport(Box2i32ui32(Vec2i32::ZERO, renderData.app->getCurrentClientSize()));
						renderData.g->beginRender();
						renderData.g->clear(ClearFlag::COLOR | ClearFlag::DEPTH | ClearFlag::STENCIL, Vec4f32(0.0f, 0.0f, 0.25f, 1.0f), 1.f, 0);

						if (true) {
							renderData.spc->get("red")->set(Vec4f32((float32_t)rand() / (float32_t)RAND_MAX, (float32_t)rand() / (float32_t)RAND_MAX)).setUpdated();

							auto vb = renderData.vbf->get("POSITION0");
							if (vb) {
								auto cycle = 20000;
								auto halfCycyle = float32_t(cycle / 2);
								auto t = aurora::Time::now<std::chrono::milliseconds>() % cycle;
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

						renderData.g->setBlendState(renderData.bs.get(), Vec4f32::ZERO);
						renderData.g->draw(renderData.vbf.get(), renderData.p.get(), renderData.spc.get(), renderData.ib.get());

						renderData.g->endRender();
						renderData.g->present();
					})));

					(new Stats())->run(renderData.looper);
					app->setVisible(true);
					renderData.looper->run(true);
				}
			}
		}

		return 0;
	}
};