#pragma once

#include "../BaseTester.h"
#include <random>

class RenderTargetTester : public BaseTester {
public:
	virtual int32_t SRK_CALL run() override {
		auto monitors = Monitor::getMonitors();
		auto vms = monitors[0].getVideoModes();

		IntrusivePtr wml = new WindowModuleLoader();
		if (!wml->load(getWindowDllPath())) return 0;

		auto wm = wml->create();
		if (!wm) return 0;

		CreateWindowDescriptor desc;
		desc.style.resizable = true;
		desc.contentSize.set(800, 600);
		auto win = wm->crerate(desc);
		if (!win) return 0;

		IntrusivePtr gml = new GraphicsModuleLoader();

		if (!gml->load(getDllPath("srk-module-graphics-gl"))) return 0;
		//if (!gml->load(getDllPath("srk-module-graphics-d3d11"))) return 0;
			
		CreateGrahpicsModuleDescriptor createGrahpicsModuleDesc;
		createGrahpicsModuleDesc.window = win;
		createGrahpicsModuleDesc.sampleCount = 4;
		createGrahpicsModuleDesc.debug = Environment::IS_DEBUG;

		auto graphics = gml->create(createGrahpicsModuleDesc);
		if (!graphics) return 0;

		printaln(L"Graphics Version : "sv, graphics->getVersion());

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
			IntrusivePtr<IRenderTarget> rt;

			struct {
				IntrusivePtr<VertexAttributeCollection> vbf;
				IntrusivePtr<IProgram> p;
				IntrusivePtr<IIndexBuffer> ib;
			} pp;
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
			graphics->setRasterizerState(rs);
		}

		renderData.dss = graphics->createDepthStencilState();
		renderData.vbf = new VertexAttributeCollection();
		renderData.spc = new ShaderParameterCollection();

		{
			//auto vertexBuffer = graphics->createVertexBuffer();
			IntrusivePtr vertexBuffer = new MultipleVertexBuffer(*graphics, 3);
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
				vertexBuffer->create(sizeof(vertices), Usage::NONE, Usage::NONE, vertices, sizeof(vertices));
				vertexBuffer->setStride(3 * sizeof(float32_t));
			}

			auto uvBuffer = graphics->createVertexBuffer();
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
				uvBuffer->create(sizeof(uvs), Usage::NONE, Usage::NONE, uvs, sizeof(uvs));
				uvBuffer->setStride(2 * sizeof(float32_t));
			}

			renderData.vbf->set("POSITION0", VertexAttribute<IVertexBuffer>(vertexBuffer, 3, VertexType::F32, 0));
			renderData.vbf->set("TEXCOORD0", VertexAttribute<IVertexBuffer>(uvBuffer, 2, VertexType::F32, 0));
		}

		{
			renderData.spc->set("red", new ShaderParameter(ShaderParameterUsage::EXCLUSIVE))->set(Vec4f32::ONE).setUpdated();
			renderData.spc->set("green", new ShaderParameter(ShaderParameterUsage::EXCLUSIVE))->set(Vec4f32::ONE).setUpdated();
			//cf->add("blue", new ShaderParameter())->set(Vec4f32::ONE).setUpdated();

			IntrusivePtr aabbccStruct = new ShaderParameterCollection();
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
			auto rts = graphics->createTexture2DResource();
			rts->create(Vec2uz(800 * 2, 600 * 2), 0, 1, 1, TextureFormat::R8G8B8A8_UNORM, Usage::RENDERABLE, Usage::NONE);

			{
				auto rv = graphics->createRenderView();
				rv->create(rts, 0, 0, 0);

				auto ds = graphics->createDepthStencil();
				ds->create(rts->getDimensions(), DepthStencilFormat::D24S8, rts->getSampleCount());

				renderData.rt = graphics->createRenderTarget();
				renderData.rt->setRenderView(0, rv);
				renderData.rt->setDepthStencil(ds);
			}

			{
				auto tv = graphics->createTextureView();
				tv->create(rts, 0, 1, 0, 0);

				auto s = graphics->createSampler();

				renderData.spc->set("ppTex", new ShaderParameter(ShaderParameterUsage::AUTO))->set(tv).setUpdated();
				renderData.spc->set("ppTexSampler", new ShaderParameter(ShaderParameterUsage::AUTO))->set(s).setUpdated();
			}
		}

		{
			IntrusivePtr texRes = graphics->createTexture2DResource();
			if (texRes) {
				TextureFormat fmt;

				auto img0 = extensions::PNGConverter::decode(readFile(Application::getAppPath().parent_path().u8string() + "/Resources/c4.png"));
				img0->format = textureFormatTypeSwitch(img0->format, false);
				fmt = img0->format;
				auto mipLevels = TextureUtils::getMipLevels(img0->dimensions);
				ByteArray mipsData0;
				std::vector<void*> mipsData0Ptr;
				img0->generateMips(img0->format, mipLevels, mipsData0, 0, mipsData0Ptr);

				auto img1 = extensions::PNGConverter::decode(readFile(Application::getAppPath().parent_path().u8string() + "/Resources/red.png"));
				img1->format = textureFormatTypeSwitch(img1->format, false);
				ByteArray mipsData1;
				std::vector<void*> mipsData1Ptr;
				img1->generateMips(img1->format, mipLevels, mipsData1, 0, mipsData1Ptr);

				mipsData0Ptr.insert(mipsData0Ptr.end(), mipsData1Ptr.begin(), mipsData1Ptr.end());

				if (isSupportTextureFormat(graphics, TextureFormat::BC7_UNORM)) {
					auto bc7 = extensions::BC7Converter::encode(*img0, 2, 64, extensions::BC7Converter::Flags::NONE, 12);
					if (bc7) {
						img0->format = TextureFormat::BC7_UNORM;
						img0->source = std::move(bc7);
						fmt = TextureFormat::BC7_UNORM;
						mipsData0Ptr[0] = img0->source.getSource();
					}
				}

				auto hr = texRes->create(img0->dimensions, 0, 1, 1, fmt, Usage::NONE, Usage::UPDATE);//, mipsData0Ptr.data());
				auto bbb = texRes->update(0, 0, Box2uz(Vec2uz::ZERO, img0->dimensions), mipsData0Ptr.data()[0]);

				auto texView = graphics->createTextureView();
				texView->create(texRes, 0, -1, 0, -1);

				uint8_t texData[] = { 255, 0, 0, 255, 0, 255, 0, 255, 0, 0, 255, 255, 0, 0, 0, 255 };
				//auto mapped = texRes->map(0, 0, Usage::MAP_WRITE);
				//texRes->write(0, 0, 4, texData, sizeof(texData));
				//texRes->unmap(0, 0);
				//texRes->update(0, 0, Box2ui32(Vec2ui32(1, 1), Vec2ui32(2, 2)), texData);

				renderData.spc->set("texDiffuse", new ShaderParameter(ShaderParameterUsage::AUTO))->set(texView).setUpdated();
			}

			auto sam = graphics->createSampler();
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
			renderData.ib->create(sizeof(indices), Usage::NONE, Usage::NONE, indices, sizeof(indices));
			renderData.ib->setFormat(IndexType::UI16);
		}

		//pp
		renderData.pp.ib = graphics->createIndexBuffer();
		{
			uint16_t data[] = { 0, 1, 2, 0, 2, 3 };
			renderData.pp.ib->create(sizeof(data), Usage::NONE, Usage::NONE, data, sizeof(data));
			renderData.pp.ib->setFormat(IndexType::UI16);
		}

		renderData.pp.vbf = new VertexAttributeCollection();
		{
			IntrusivePtr ppVertexBuffer = graphics->createVertexBuffer();
			{
				float32_t data[] = { -1.0f, 1.0f, 0.8f, 1.0f, 0.8f, -0.9f, -1.0f, -0.9f };
				ppVertexBuffer->create(sizeof(data), Usage::NONE, Usage::NONE, data, sizeof(data));
				ppVertexBuffer->setStride(2 * sizeof(float32_t));
			}

			IntrusivePtr ppUVBuffer = graphics->createVertexBuffer();
			{
				float32_t data[] = { 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f };
				ppUVBuffer->create(sizeof(data), Usage::NONE, Usage::NONE, data, sizeof(data));
				ppUVBuffer->setStride(2 * sizeof(float32_t));
			}

			renderData.pp.vbf->set("POSITION0", VertexAttribute<IVertexBuffer>(ppVertexBuffer, 2, VertexType::F32, 0));
			renderData.pp.vbf->set("TEXCOORD0", VertexAttribute<IVertexBuffer>(ppUVBuffer, 2, VertexType::F32, 0));
		}
		//

		renderData.p = graphics->createProgram();
		createProgram(*renderData.p, "vert.hlsl", "frag.hlsl");

		renderData.pp.p = graphics->createProgram();
		createProgram(*renderData.pp.p, "pp_draw_tex_vert.hlsl", "pp_draw_tex_frag.hlsl");

		win->getEventDispatcher()->addEventListener(WindowEvent::CLOSING, new EventListener(std::function([](Event<WindowEvent>& e) {
			//*e.getData<bool>() = true;
			})));

		win->getEventDispatcher()->addEventListener(WindowEvent::CLOSED, new EventListener(std::function([renderData](Event<WindowEvent>& e) {
			renderData.looper->stop();
			})));

		renderData.looper->getEventDispatcher()->addEventListener(LooperEvent::TICKING, new EventListener(std::function([renderData](Event<LooperEvent>& e) {
			auto dt = float64_t(*e.getData<int64_t>());

			while (renderData.winModule->processEvent()) {};

			renderData.g->setRenderTarget(renderData.rt);
			renderData.g->setViewport(Box2i32ui32(Vec2i32::ZERO, renderData.rt->getDimensions()));
			renderData.g->beginRender();
			renderData.g->clear(ClearFlag::COLOR | ClearFlag::DEPTH | ClearFlag::STENCIL, Vec4f32(0.0f, 0.0f, 0.25f, 1.0f), 1.f, 0);

			renderData.g->setBlendState(renderData.bs);
			renderData.g->setDepthStencilState(renderData.dss);
			renderData.g->draw(renderData.p, renderData.vbf, renderData.spc, renderData.ib);

			renderData.g->endRender();

			/*
			auto tr = (ITexture2DResource*)renderData.rt->getRenderView(0)->getResource();
			auto& trSize = tr->getSize();
			auto pixelsSize = trSize.getMultiplies() * 4;
			auto& features = renderData.g->getDeviceFeatures();
			if (features.textureMap) {
				RefPtr tex = renderData.g->createTexture2DResource();
				if (tex->create(trSize, 0, 1, 1, tr->getFormat(), Usage::MAP_READ)) {
					if (tex->copyFrom(Vec3ui32::ZERO, 0, 0, tr, 0, 0, Box3ui32(Vec3ui32::ZERO, Vec3ui32(trSize[0], trSize[1], 1)))) {
						if (tex->map(0, 0, Usage::MAP_READ) != Usage::NONE) {
							ByteArray pixels(pixelsSize, pixelsSize);
							tex->read(0, 0, 0, pixels.getSource(), pixels.getLength());

							tex->unmap(0, 0);

							Image png;
							png.format = TextureFormat::R8G8B8A8;
							png.size.set(trSize);
							png.source = pixels.slice();
							if (auto bin = extensions::PNGConverter::encode(png); bin.isValid()) writeFile("D:/Users/Sephiroth/Desktop/test.png", bin);
						}
					}
				}
			} else if (features.pixelBuffer) {
				RefPtr pb = renderData.g->createPixelBuffer();
				if (pb->create(pixelsSize, Usage::MAP_READ)) {
					if (tr->copyTo(0, pb)) {
						if (pb->map(Usage::MAP_READ) != Usage::NONE) {
							ByteArray pixels(pixelsSize, pixelsSize);
							pb->read(0, pixels.getSource(), pixels.getLength());

							pb->unmap();

							Image png;
							png.format = TextureFormat::R8G8B8A8;
							png.size.set(trSize);
							png.source = pixels.slice();
							if (auto bin = extensions::PNGConverter::encode(png); bin.isValid()) writeFile("D:/Users/Sephiroth/Desktop/test2.png", bin);
						}
					}
				}
			}
			*/

			//===================================
			renderData.g->setRenderTarget(nullptr);
			renderData.g->setViewport(Box2i32ui32(Vec2i32::ZERO, renderData.win->getContentSize()));
			renderData.g->beginRender();
			renderData.g->clear(ClearFlag::COLOR | ClearFlag::DEPTH | ClearFlag::STENCIL, Vec4f32(0.0f, 0.0f, 0.0f, 1.0f), 1.f, 0);

			renderData.g->setBlendState(renderData.bs);
			renderData.g->setDepthStencilState(renderData.dss);
			renderData.g->draw(renderData.pp.p, renderData.pp.vbf, renderData.spc, renderData.pp.ib);

			renderData.g->endRender();

			renderData.g->present();
			})));

		(new Stats())->run(renderData.looper);
		win->setVisible(true);
		renderData.looper->run(true);

		return 0;
	}
};