#pragma once

#include "../BaseTester.h"

class RenderTargetTester : public BaseTester {
public:
	virtual int32_t AE_CALL run() override {
		auto monitors = Monitor::getMonitors();
		auto vms = monitors[0].getVideoModes();

		RefPtr app = new Application(u8"TestApp", 1000. / 60.);

		Application::Style wndStype;
		wndStype.thickFrame = true;
		if (app->createWindow(wndStype, u8"", Box2i32(Vec2i32({ 100, 100 }), Vec2i32({ 800, 600 })), false)) {
			RefPtr gml = new GraphicsModuleLoader();

			//if (gml->load(getDLLName("ae-win-gl"))) {
			if (gml->load(getDLLName("ae-win-d3d11"))) {
				RefPtr gpstml = new ModuleLoader<IProgramSourceTranslator>();
				gpstml->load(getDLLName("ae-program-source-translator"));
				RefPtr gpst = gpstml->create(&Args().add("dxc", getDLLName("dxcompiler")));

				RefPtr graphics = gml->create(&Args().add("app", &*app).add("trans", gpst.get()));

				if (graphics) {
					println("Graphics Version : ", graphics->getVersion());

					graphics->getEventDispatcher().addEventListener(GraphicsEvent::ERR, new EventListener(Recognitor<GraphicsEvent>(),[](Event<GraphicsEvent>& e) {
						println(*(std::string_view*)e.getData());
						int a = 1;
					}));

					{
						RefPtr rs = graphics->createRasterizerState();
						rs->setFillMode(FillMode::SOLID);
						rs->setFrontFace(FrontFace::CW);
						rs->setCullMode(CullMode::NONE);
						graphics->setRasterizerState(&*rs);
					}

					//auto vertexBuffer = graphics->createVertexBuffer();
					RefPtr vertexBuffer = new MultipleVertexBuffer(*graphics, 3);
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
						vertexBuffer->create(sizeof(vertices), Usage::NONE, vertices, sizeof(vertices));
						vertexBuffer->setFormat(VertexSize::THREE, VertexType::F32);
					}

					RefPtr uvBuffer = graphics->createVertexBuffer();
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

					RefPtr vbf = new VertexBufferFactory();
					vbf->add("POSITION0", vertexBuffer.get());
					vbf->add("TEXCOORD0", uvBuffer.get());

					RefPtr cf = new ShaderParameterFactory();
					cf->add("red", new ShaderParameter(ShaderParameterUsage::EXCLUSIVE))->set(Vec4f32::ONE).setUpdated();
					cf->add("green", new ShaderParameter(ShaderParameterUsage::EXCLUSIVE))->set(Vec4f32::ONE).setUpdated();
					//cf->add("blue", new ShaderParameter())->set(Vec4f32::ONE).setUpdated();

					RefPtr aabbccStruct = new ShaderParameterFactory();
					aabbccStruct->add("val1", new ShaderParameter(ShaderParameterUsage::EXCLUSIVE))->set(Vec4f32::ONE).setUpdated();
					f32 val2[] = { 1.0f, 1.0f };
					aabbccStruct->add("val2", new ShaderParameter(ShaderParameterUsage::EXCLUSIVE))->set<f32>(val2, sizeof(val2), sizeof(f32), true).setUpdated();
					aabbccStruct->add("val3", new ShaderParameter())->set(Vec4f32::ONE).setUpdated();
					cf->add("blue", new ShaderParameter())->set(aabbccStruct.get());

					RefPtr bs = graphics->createBlendState();
					//bs->setIndependentBlendEnabled(true);
					RenderTargetBlendState rtbs;
					rtbs.enabled = false;
					//rtbs.func.set(BlendFactor::ZERO, BlendFactor::ONE);
					rtbs.writeMask.set(true, true, true, true);
					bs->setRenderTargetState(0, rtbs);

					RefPtr rts = graphics->createTexture2DResource();
					rts->create(Vec2ui32(800, 600), 0, 1, TextureFormat::R8G8B8A8, Usage::RENDERABLE);

					RefPtr rv = graphics->createRenderView();
					rv->create(rts.get(), 0, 0, 0);

					RefPtr ds = graphics->createDepthStencil();
					ds->create(rts->getSize(), false);

					RefPtr rt = graphics->createRenderTarget();
					rt->setRenderView(0, rv.get());
					rt->setDepthStencil(ds.get());

					RefPtr dss = graphics->createDepthStencilState();

					RefPtr texRes = graphics->createTexture2DResource();
					if (texRes) {
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

						auto hr = texRes->create(img0->size, 0, 1, img0->format, Usage::IGNORE_UNSUPPORTED | Usage::UPDATE, mipsData0Ptr.data());

						RefPtr texView = graphics->createTextureView();
						texView->create(texRes.get(), 0, -1, 0, -1);

						RefPtr pb = graphics->createPixelBuffer();
						if (pb) {
							//pb->create(img0->size.getMultiplies() * 4, Usage::MAP_WRITE | Usage::PERSISTENT_MAP, mipsData0Ptr.data()[0], img0->size.getMultiplies() * 4);
							//texRes->copyFrom(0, 0, Box2ui32(Vec2ui32(0, 0), Vec2ui32(4, 4)), pb);
						}

						uint8_t texData[] = { 255, 0, 0, 255, 0, 255, 0, 255, 0, 0, 255, 255, 0, 0, 0, 255 };
						//auto mapped = texRes->map(0, 0, Usage::MAP_WRITE);
						//texRes->write(0, 0, 4, texData, sizeof(texData));
						//texRes->unmap(0, 0);
						//texRes->update(0, 0, Box2ui32(Vec2ui32(1, 1), Vec2ui32(2, 2)), texData);

						cf->add("texDiffuse", new ShaderParameter(ShaderParameterUsage::AUTO))->set(texView.get()).setUpdated();
					}

					RefPtr sam = graphics->createSampler();
					if (sam) {
						//sam->setMipLOD(0, 0);
						//sam->setAddress(SamplerAddressMode::WRAP, SamplerAddressMode::WRAP, SamplerAddressMode::WRAP);
						sam->setFilter(SamplerFilterOperation::NORMAL, SamplerFilterMode::POINT, SamplerFilterMode::POINT, SamplerFilterMode::POINT);
						cf->add("samLiner", new ShaderParameter(ShaderParameterUsage::AUTO))->set(sam.get()).setUpdated();
					}

					RefPtr ib = graphics->createIndexBuffer();
					if (ib) {
						uint16_t indices[] = {
							0, 1, 3,
							3, 1, 2,
						
							4, 5, 7,
							7, 5, 6 };
						ib->create(sizeof(indices), Usage::NONE, indices, sizeof(indices));
						ib->setFormat(IndexType::UI16);
					}

					//pp
					RefPtr ppIndexBuffer = graphics->createIndexBuffer();
					{
						uint16_t data[] = { 0, 1, 2, 0, 2, 3 };
						ppIndexBuffer->create(sizeof(data), Usage::NONE, data, sizeof(data));
						ppIndexBuffer->setFormat(IndexType::UI16);
					}

					RefPtr ppVBF = new VertexBufferFactory();
					{
						RefPtr ppVertexBuffer = graphics->createVertexBuffer();
						{
							f32 data[] = { -1.0f, 1.0f, 0.8f, 1.0f, 0.8f, -1.0f, -1.0f, -1.0f };
							ppVertexBuffer->create(sizeof(data), Usage::NONE, data, sizeof(data));
							ppVertexBuffer->setFormat(VertexSize::TWO, VertexType::F32);
						}

						RefPtr ppUVBuffer = graphics->createVertexBuffer();
						{
							f32 data[] = { 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f };
							ppUVBuffer->create(sizeof(data), Usage::NONE, data, sizeof(data));
							ppUVBuffer->setFormat(VertexSize::TWO, VertexType::F32);
						}

						ppVBF->add("POSITION0", ppVertexBuffer.get());
						ppVBF->add("TEXCOORD0", ppUVBuffer.get());
					}

					{
						RefPtr tv = graphics->createTextureView();
						tv->create(rts.get(), 0, 1, 0, 0);

						RefPtr s = graphics->createSampler();

						cf->add("ppTex", new ShaderParameter(ShaderParameterUsage::AUTO))->set(tv.get()).setUpdated();
						cf->add("ppTexSampler", new ShaderParameter(ShaderParameterUsage::AUTO))->set(s.get()).setUpdated();
					}
					//

					RefPtr p = graphics->createProgram();
					if (p) {
						if (!p->upload(readProgramSourcee(app->getAppPath() + u8"Resources/vert.hlsl", ProgramStage::VS), readProgramSourcee(app->getAppPath() + u8"Resources/frag.hlsl", ProgramStage::PS))) {
							println(L"program upload error");
						}
					}

					RefPtr ppProgram = graphics->createProgram();
					if (!ppProgram->upload(readProgramSourcee(app->getAppPath() + u8"Resources/pp_draw_tex_vert.hlsl", ProgramStage::VS), readProgramSourcee(app->getAppPath() + u8"Resources/pp_draw_tex_frag.hlsl", ProgramStage::PS))) {
						println(L"program upload error");
					}

					RefPtr appClosingListener = new EventListener(std::function([](Event<ApplicationEvent>& e) {
						//*e.getData<bool>() = true;
					}));

					struct {
						RefPtr<IGraphicsModule> graphics;
						RefPtr<VertexBufferFactory> vbf;
						RefPtr<ShaderParameterFactory> spf;
						RefPtr<IProgram> p;
						RefPtr<IIndexBuffer> ib;
						RefPtr<IBlendState> bs;
						RefPtr<IDepthStencilState> dss;
						RefPtr<IRenderTarget> rt;

						struct {
							RefPtr<VertexBufferFactory> vbf;
							RefPtr<IProgram> p;
							RefPtr<IIndexBuffer> ib;
						} pp;
					} renderData;
					renderData.graphics = graphics;
					renderData.vbf = vbf;
					renderData.spf = cf;
					renderData.p = p;
					renderData.ib = ib;
					renderData.bs = bs;
					renderData.dss = dss;
					renderData.rt = rt;
					renderData.pp.vbf = ppVBF;
					renderData.pp.p = ppProgram;
					renderData.pp.ib = ppIndexBuffer;

					auto& evtDispatcher = app->getEventDispatcher();

					evtDispatcher.addEventListener(ApplicationEvent::TICKING, new EventListener(std::function([renderData](Event<ApplicationEvent>& e) {
						auto app = e.getTarget<Application>();

						auto dt = f64(*e.getData<int64_t>());

						renderData.graphics->setRenderTarget(renderData.rt.get());
						renderData.graphics->beginRender();
						renderData.graphics->clear(ClearFlag::COLOR | ClearFlag::DEPTH | ClearFlag::STENCIL, Vec4f32(0.0f, 0.0f, 0.25f, 1.0f), 1.f, 0);

						renderData.graphics->setBlendState(renderData.bs.get(), Vec4f32::ZERO);
						renderData.graphics->setDepthStencilState(renderData.dss.get(), 0);
						renderData.graphics->draw(renderData.vbf.get(), renderData.p.get(), renderData.spf.get(), renderData.ib.get());

						renderData.graphics->endRender();

						//===================================
						renderData.graphics->setRenderTarget(nullptr);
						renderData.graphics->beginRender();
						renderData.graphics->clear(ClearFlag::COLOR | ClearFlag::DEPTH | ClearFlag::STENCIL, Vec4f32(0.0f, 0.0f, 0.0f, 1.0f), 1.f, 0);

						renderData.graphics->setBlendState(renderData.bs.get(), Vec4f32::ZERO);
						renderData.graphics->setDepthStencilState(renderData.dss.get(), 0);
						renderData.graphics->draw(renderData.pp.vbf.get(), renderData.pp.p.get(), renderData.spf.get(), renderData.pp.ib.get());

						renderData.graphics->endRender();

						renderData.graphics->present();
					})));

					evtDispatcher.addEventListener(ApplicationEvent::CLOSING, *appClosingListener);

					(new Stats())->run(app.get());
					app->setVisible(true);
					app->run(true);
				}
			}
		}

		return 0;
	}
};