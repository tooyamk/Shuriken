#pragma once

#include "../BaseTester.h"

class DepthTestTester : public BaseTester {
public:
	virtual int32_t AE_CALL run() override {
		auto monitors = Monitor::getMonitors();
		auto vms = monitors[0].getVideoModes();

		RefPtr<Application> app = new Application(u8"TestApp", 1000. / 60.);

		Application::Style wndStype;
		wndStype.thickFrame = true;
		if (app->createWindow(wndStype, u8"", Box2i32(Vec2i32({ 100, 100 }), Vec2i32({ 800, 600 })), false)) {
			RefPtr<GraphicsModuleLoader> gml = new GraphicsModuleLoader();

			//if (gml->load(getDLLName("ae-win-gl"))) {
			if (gml->load(getDLLName("ae-win-d3d11"))) {
				auto gpstml = new ModuleLoader<IProgramSourceTranslator>();
				gpstml->load(getDLLName("ae-program-source-translator"));
				auto gpst = gpstml->create(&Args().add("dxc", getDLLName("dxcompiler")));

				RefPtr<IGraphicsModule> graphics = gml->create(&Args().add("app", &*app).add("trans", gpst));

				if (graphics) {
					println("Graphics Version : ", graphics->getVersion());

					{
						RefPtr<IRasterizerState> rs = graphics->createRasterizerState();
						rs->setFillMode(FillMode::SOLID);
						rs->setFrontFace(FrontFace::CW);
						rs->setCullMode(CullMode::NONE);
						graphics->setRasterizerState(&*rs);
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
							-0.5f, -0.5f, 0.f,
							-0.5f, 0.5f, 0.f,
							0.5f, 0.5f, 0.f,
							0.5f, -0.5f, 0.f,
						
							0.0f, 0.0f, 0.5f,
							0.0f, 1.0f, 0.5f,
							1.0f, 1.0f, 0.5f,
							1.0f, 0.0f, 0.5f };
						vertexBuffer->create(sizeof(vertices), Usage::MAP_WRITE | Usage::UPDATE | Usage::PERSISTENT_MAP, vertices, sizeof(vertices));
						vertexBuffer->setFormat(VertexSize::THREE, VertexType::F32);
					}

					auto uvBuffer = graphics->createVertexBuffer();
					if (uvBuffer) {
						f32 uvs[] = {
							0.f, 1.f,
							0.f, 0.f,
							1.f, 0.f,
							1.f, 1.f,
						
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

					auto bs = graphics->createBlendState();
					//bs->setIndependentBlendEnabled(true);
					RenderTargetBlendState rtbs;
					rtbs.enabled = false;
					//rtbs.func.set(BlendFactor::ZERO, BlendFactor::ONE);
					rtbs.writeMask.set(true, true, true, true);
					bs->setRenderTargetState(0, rtbs);

					auto dss = graphics->createDepthStencilState();

					auto texRes = graphics->createTexture2DResource();
					if (texRes) {
						auto texView = graphics->createTextureView();
						if (texView) texView->create(texRes, 0, -1, 0, -1);

						auto img0 = file::PNGConverter::parse(readFile(app->getAppPath() + u8"Resources/c4.png"));
						auto mipLevels = Image::calcMipLevels(img0->size);
						ByteArray mipsData0;
						std::vector<void*> mipsData0Ptr;
						img0->generateMips(img0->format, mipLevels, mipsData0, mipsData0Ptr);

						auto img1 = file::PNGConverter::parse(readFile(app->getAppPath() + u8"Resources/red.png"));
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
							3, 1, 2,
						
							4, 5, 7,
							7, 5, 6 };
						ib->create(sizeof(indices), Usage::NONE, indices, sizeof(indices));
						ib->setFormat(IndexType::UI16);
					}

					auto p = graphics->createProgram();
					if (p) {
						if (!p->upload(readProgramSourcee(app->getAppPath() + u8"Resources/vert.hlsl", ProgramStage::VS), readProgramSourcee(app->getAppPath() + u8"Resources/frag.hlsl", ProgramStage::PS))) {
							println(L"program upload error");
						}
					}

					RefPtr<IEventListener<ApplicationEvent>> appClosingListener = new EventListener(std::function([](Event<ApplicationEvent>& e) {
						//*e.getData<bool>() = true;
					}));

					auto& evtDispatcher = app->getEventDispatcher();

					evtDispatcher.addEventListener(ApplicationEvent::TICKING, new EventListener(std::function([graphics, vbf, cf, p, ib, bs, dss](Event<ApplicationEvent>& e) {
						auto app = e.getTarget<Application>();

						auto dt = f64(*e.getData<int64_t>());

						graphics->beginRender();
						graphics->clear(ClearFlag::COLOR | ClearFlag::DEPTH | ClearFlag::STENCIL, Vec4f32(0.0f, 0.0f, 0.25f, 1.0f), 1.f, 0);

						graphics->setBlendState(bs, Vec4f32::ZERO);
						graphics->setDepthStencilState(dss, 0);
						graphics->draw(vbf, p, cf, ib);

						graphics->endRender();
						graphics->present();
					})));

					evtDispatcher.addEventListener(ApplicationEvent::CLOSING, *appClosingListener);

					(new Stats())->run(&*app);
					app->setVisible(true);
					app->run(true);
				}
			}
		}

		return 0;
	}
};