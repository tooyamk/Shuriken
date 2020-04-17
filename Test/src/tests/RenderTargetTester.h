#pragma once

#include "../BaseTester.h"

class RenderTargetTester : public BaseTester {
public:
	virtual int32_t AE_CALL run() override {
		auto monitors = Monitor::getMonitors();
		auto vms = monitors[0].getVideoModes();

		RefPtr app = new Application(u8"TestApp");

		Application::Style wndStype;
		wndStype.thickFrame = true;
		if (app->createWindow(wndStype, u8"", Box2i32(Vec2i32({ 100, 100 }), Vec2i32({ 800, 600 })), false)) {
			RefPtr gml = new GraphicsModuleLoader();

			if (gml->load(getDLLName("ae-win-gl"))) {
			//if (gml->load(getDLLName("ae-win-d3d11"))) {
				RefPtr gpstml = new ModuleLoader<IProgramSourceTranslator>();
				gpstml->load(getDLLName("ae-program-source-translator"));
				RefPtr gpst = gpstml->create(&Args().add("dxc", getDLLName("dxcompiler")));

				RefPtr graphics = gml->create(&Args().add("app", &*app).add("sampleCount", SampleCount(1)).add("trans", &*gpst));

				if (graphics) {
					println("Graphics Version : ", graphics->getVersion());

					graphics->getEventDispatcher().addEventListener(GraphicsEvent::ERR, new EventListener(Recognitor<GraphicsEvent>(),[](Event<GraphicsEvent>& e) {
						println(*(std::string_view*)e.getData());
						int a = 1;
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
						RefPtr<IRenderTarget> rt;

						struct {
							RefPtr<VertexBufferCollection> vbf;
							RefPtr<IProgram> p;
							RefPtr<IIndexBuffer> ib;
						} pp;
					} renderData;
					renderData.app = app;
					renderData.looper = new Looper(1000.0 / 60.0);
					renderData.g = graphics;

					{
						RefPtr rs = graphics->createRasterizerState();
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
							float32_t vertices[] = {
								-0.5f, -0.5f, .0f,
								-0.5f, 0.5f, .0f,
								0.45f, 0.45f, 0.2f,
								0.45f, -0.5f, .0f,

								0.0f, 0.0f, 0.5f,
								0.0f, 1.0f, 0.5f,
								1.0f, 1.0f, 0.5f,
								1.0f, 0.0f, 0.5f };
							vertexBuffer->create(sizeof(vertices), Usage::NONE, vertices, sizeof(vertices));
							vertexBuffer->setFormat(VertexFormat(VertexSize::THREE, VertexType::F32));
						}

						RefPtr uvBuffer = graphics->createVertexBuffer();
						if (uvBuffer) {
							float32_t uvs[] = {
								0.f, 1.f,
								0.f, 0.f,
								1.f, 0.f,
								1.f, 1.f,

								0.f, 1.f,
								0.f, 0.f,
								1.f, 0.f,
								1.f, 1.f };
							uvBuffer->create(sizeof(uvs), Usage::NONE, uvs, sizeof(uvs));
							uvBuffer->setFormat(VertexFormat(VertexSize::TWO, VertexType::F32));
						}

						renderData.vbf->set("POSITION0", vertexBuffer.get());
						renderData.vbf->set("TEXCOORD0", uvBuffer.get());
					}

					{
						renderData.spc->set("red", new ShaderParameter(ShaderParameterUsage::EXCLUSIVE))->set(Vec4f32::ONE).setUpdated();
						renderData.spc->set("green", new ShaderParameter(ShaderParameterUsage::EXCLUSIVE))->set(Vec4f32::ONE).setUpdated();
						//cf->add("blue", new ShaderParameter())->set(Vec4f32::ONE).setUpdated();

						RefPtr aabbccStruct = new ShaderParameterCollection();
						aabbccStruct->set("val1", new ShaderParameter(ShaderParameterUsage::EXCLUSIVE))->set(Vec4f32::ONE).setUpdated();
						float32_t val2[] = { 1.0f, 1.0f };
						aabbccStruct->set("val2", new ShaderParameter(ShaderParameterUsage::EXCLUSIVE))->set<float32_t>(val2, sizeof(val2), sizeof(float32_t), true).setUpdated();
						aabbccStruct->set("val3", new ShaderParameter())->set(Vec4f32::ONE).setUpdated();
						renderData.spc->set("blue", new ShaderParameter())->set(aabbccStruct.get());
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
						RefPtr rts = graphics->createTexture2DResource();
						rts->create(Vec2ui32(800, 600), 0, 1, 4, TextureFormat::R8G8B8A8, Usage::RENDERABLE);

						{
							RefPtr rv = graphics->createRenderView();
							rv->create(rts.get(), 0, 0, 0);

							RefPtr ds = graphics->createDepthStencil();
							ds->create(rts->getSize(), DepthStencilFormat::D24S8, rts->getSampleCount());

							renderData.rt = graphics->createRenderTarget();
							renderData.rt->setRenderView(0, rv.get());
							renderData.rt->setDepthStencil(ds.get());
						}

						{
							RefPtr tv = graphics->createTextureView();
							tv->create(rts.get(), 0, 1, 0, 0);

							RefPtr s = graphics->createSampler();

							renderData.spc->set("ppTex", new ShaderParameter(ShaderParameterUsage::AUTO))->set(tv.get()).setUpdated();
							renderData.spc->set("ppTexSampler", new ShaderParameter(ShaderParameterUsage::AUTO))->set(s.get()).setUpdated();
						}
					}

					{
						RefPtr texRes = graphics->createTexture2DResource();
						if (texRes) {
							auto img0 = extensions::PNGConverter::parse(readFile(app->getAppPath() + u8"Resources/c4.png"));
							auto mipLevels = Image::calcMipLevels(img0->size);
							ByteArray mipsData0;
							std::vector<void*> mipsData0Ptr;
							img0->generateMips(img0->format, mipLevels, mipsData0, mipsData0Ptr);

							auto img1 = extensions::PNGConverter::parse(readFile(app->getAppPath() + u8"Resources/red.png"));
							ByteArray mipsData1;
							std::vector<void*> mipsData1Ptr;
							img1->generateMips(img1->format, mipLevels, mipsData1, mipsData1Ptr);

							mipsData0Ptr.insert(mipsData0Ptr.end(), mipsData1Ptr.begin(), mipsData1Ptr.end());

							auto hr = texRes->create(img0->size, 0, 1, 1, img0->format, Usage::IGNORE_UNSUPPORTED | Usage::UPDATE, mipsData0Ptr.data());

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

							renderData.spc->set("texDiffuse", new ShaderParameter(ShaderParameterUsage::AUTO))->set(texView.get()).setUpdated();
						}

						RefPtr sam = graphics->createSampler();
						if (sam) {
							//sam->setMipLOD(0, 0);
							//sam->setAddress(SamplerAddressMode::WRAP, SamplerAddressMode::WRAP, SamplerAddressMode::WRAP);
							sam->setFilter(SamplerFilterOperation::NORMAL, SamplerFilterMode::POINT, SamplerFilterMode::POINT, SamplerFilterMode::POINT);
							renderData.spc->set("samLiner", new ShaderParameter(ShaderParameterUsage::AUTO))->set(&*sam).setUpdated();
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

					//pp
					renderData.pp.ib = graphics->createIndexBuffer();
					{
						uint16_t data[] = { 0, 1, 2, 0, 2, 3 };
						renderData.pp.ib->create(sizeof(data), Usage::NONE, data, sizeof(data));
						renderData.pp.ib->setFormat(IndexType::UI16);
					}

					renderData.pp.vbf = new VertexBufferCollection();
					{
						RefPtr ppVertexBuffer = graphics->createVertexBuffer();
						{
							float32_t data[] = { -1.0f, 1.0f, 0.8f, 1.0f, 0.8f, -0.9f, -1.0f, -0.9f };
							ppVertexBuffer->create(sizeof(data), Usage::NONE, data, sizeof(data));
							ppVertexBuffer->setFormat(VertexFormat(VertexSize::TWO, VertexType::F32));
						}

						RefPtr ppUVBuffer = graphics->createVertexBuffer();
						{
							float32_t data[] = { 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f };
							ppUVBuffer->create(sizeof(data), Usage::NONE, data, sizeof(data));
							ppUVBuffer->setFormat(VertexFormat(VertexSize::TWO, VertexType::F32));
						}

						renderData.pp.vbf->set("POSITION0", ppVertexBuffer.get());
						renderData.pp.vbf->set("TEXCOORD0", ppUVBuffer.get());
					}
					//

					renderData.p = graphics->createProgram();
					programCreate(*renderData.p, "vert.hlsl", "frag.hlsl");

					renderData.pp.p = graphics->createProgram();
					programCreate(*renderData.pp.p, "pp_draw_tex_vert.hlsl", "pp_draw_tex_frag.hlsl");

					app->getEventDispatcher().addEventListener(ApplicationEvent::CLOSING, new EventListener(std::function([](Event<ApplicationEvent>& e) {
						//*e.getData<bool>() = true;
					})));

					app->getEventDispatcher().addEventListener(ApplicationEvent::CLOSED, new EventListener(std::function([renderData](Event<ApplicationEvent>& e) {
						renderData.looper->stop();
					})));

					renderData.looper->getEventDispatcher().addEventListener(LooperEvent::TICKING, new EventListener(std::function([renderData](Event<LooperEvent>& e) {
						auto dt = float64_t(*e.getData<int64_t>());

						renderData.app->pollEvents();

						renderData.g->setRenderTarget(renderData.rt.get());
						renderData.g->beginRender();
						renderData.g->clear(ClearFlag::COLOR | ClearFlag::DEPTH | ClearFlag::STENCIL, Vec4f32(0.0f, 0.0f, 0.25f, 1.0f), 1.f, 0);

						renderData.g->setBlendState(renderData.bs.get(), Vec4f32::ZERO);
						renderData.g->setDepthStencilState(renderData.dss.get(), 0);
						renderData.g->draw(renderData.vbf.get(), renderData.p.get(), renderData.spc.get(), renderData.ib.get());

						renderData.g->endRender();

						//===================================
						renderData.g->setRenderTarget(nullptr);
						renderData.g->beginRender();
						renderData.g->clear(ClearFlag::COLOR | ClearFlag::DEPTH | ClearFlag::STENCIL, Vec4f32(0.0f, 0.0f, 0.0f, 1.0f), 1.f, 0);

						renderData.g->setBlendState(renderData.bs.get(), Vec4f32::ZERO);
						renderData.g->setDepthStencilState(renderData.dss.get(), 0);
						renderData.g->draw(renderData.pp.vbf, renderData.pp.p.get(), renderData.spc.get(), renderData.pp.ib.get());

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