#pragma once

#include "../BaseTester.h"
#include "srk/SerializableObject.h"

class RenderPipelineTester : public BaseTester {
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

		if (!gml->load(getDllPath("srk-module-graphics-d3d11"))) return 0;
		//if (!gml->load(getDllPath("srk-module-graphics-d3d12"))) return 0;
		//if (!gml->load(getDllPath("srk-module-graphics-gl"))) return 0;
		//if (!gml->load(getDllPath("srk-module-graphics-vulkan"))) return 0;

		CreateGrahpicsModuleDesc createGrahpicsModuleDesc;
		createGrahpicsModuleDesc.window = win;
		createGrahpicsModuleDesc.sampleCount = 4;
		createGrahpicsModuleDesc.debug = Environment::IS_DEBUG;

		/*std::vector<GraphicsAdapter> adapters;
		GraphicsAdapter::query(adapters);
		for (auto& ga : adapters) {
			if (ga.vendorId == 32902) {
				args.insert("adapter", (uintptr_t)&ga);
				break;
			}
		}*/

		auto graphics = gml->create(createGrahpicsModuleDesc);
		if (!graphics) return 0;

		printaln(L"Graphics Version : "sv, graphics->getVersion());

		graphics->getEventDispatcher()->addEventListener(GraphicsEvent::ERR, createEventListener<GraphicsEvent>([](Event<GraphicsEvent>& e) {
			printaln(*(std::string_view*)e.getData());
			int a = 1;
			}));

		win->getEventDispatcher()->addEventListener(WindowEvent::RESIZED, createEventListener<WindowEvent>([graphics](Event<WindowEvent>& e) {
			graphics->setBackBufferSize(((IWindow*)e.getTarget())->getContentSize());
			}));

		//graphics->getEventDispatcher().addEventListener(GraphicsEvent::ERR, new EventListener([](Event<GraphicsEvent>& e) {
		//	println(*(std::string_view*)e.getData());
		//	int a = 1;
		//}));

		struct {
			IntrusivePtr<IWindowModule> winModule;
			IntrusivePtr<IWindow> win;
			IntrusivePtr<Looper> looper;
			IntrusivePtr<IGraphicsModule> g;
			IntrusivePtr<SceneNode> wrold;
			IntrusivePtr<SceneNode> model;
			IntrusivePtr<Camera> camera;
			IntrusivePtr<Material> material;
			IntrusivePtr<Material> material2;
			IntrusivePtr<StandardRenderPipeline> renderPipeline;
			std::vector<IntrusivePtr<ILight>> lights;
			std::vector<IntrusivePtr<IRenderable>> renderables;
		} renderData;
		renderData.winModule = wm;
		renderData.win = win;
		renderData.looper = new Looper(1.0 / 60.0);
		renderData.g = graphics;

		{
			IntrusivePtr worldNode = new SceneNode();
			IntrusivePtr modelNode = worldNode->addChild<SceneNode>();
			//modelNode = new Node();
			//modelNode->setLocalScale(Vec3f32(4));
			//modelNode->parentTranslate(Vec3f32(0, 3000.f, 0));
			//modelNode->localRotate(Quaternion::createFromEulerY(Math::PI<f32> * 0.8f));
			//modelNode->localRotate(Quaternion::createFromEulerY(Math::PI<f32>* 0.15f));
			//auto lm = modelNode->getLocalMatrix();
			//auto wm = modelNode->getWorldMatrix();
			renderData.model = modelNode;
			if (1) {
				IntrusivePtr lightNode = worldNode->addChild<SceneNode>();
				lightNode->setLocalPosition(Vec3f32(-100, 0, -100));
				auto light = renderData.lights.emplace_back(new PointLight());
				light->attachSceneNode(lightNode);
				//light->setRadius(200);
				lightNode->localRotate(Quaternion<float32_t>(nullptr).rotationY(Math::PI_4<float32_t>));
				renderData.lights.emplace_back(light);
			}
			if (1) {
				IntrusivePtr lightNode = worldNode->addChild<SceneNode>();
				lightNode->setLocalPosition(Vec3f32(100, 0, -100));
				auto light = renderData.lights.emplace_back(new PointLight());
				light->attachSceneNode(lightNode);
				//light->setRadius(1000);
				lightNode->localRotate(Quaternion<float32_t>(nullptr).rotationY(Math::PI_4<float32_t>));
				renderData.lights.emplace_back(light);
			}
			IntrusivePtr cameraNode = worldNode->addChild<SceneNode>();
			//auto camera = new Camera();
			//cameraNode->addComponent(camera);
			renderData.camera = new Camera();
			renderData.camera->attachSceneNode(cameraNode);
			//renderData.camera->clearColor.set(1.0f, 0, 0, 1.0f);
			auto mat = new Material();
			renderData.material = mat;
			auto mat2 = new Material();
			renderData.material2 = mat2;
			IntrusivePtr renderer = new ForwardRenderer(*graphics);

			{
				renderData.camera->getSceneNode()->localTranslate(Vec3f32(0.f, 0.f, -200.f));
			}

			RenderTag forwardBaseTag("forward_base");
			RenderTag forwardAddTag("forward_add");

			IntrusivePtr tag1 = new RenderTagCollection();
			tag1->add(forwardBaseTag);

			IntrusivePtr tag2 = new RenderTagCollection();
			tag2->add(forwardAddTag);

			auto parsed = extensions::FBXConverter::decode(readFile(Application::getAppPath().parent_path().u8string() + "/Resources/teapot.fbx"));
			for (auto& mr : parsed.meshes) {
				if (mr) {
					auto rs = graphics->createRasterizerState();
					rs->setFillMode(FillMode::SOLID);
					rs->setFrontFace(FrontFace::CW);
					rs->setCullMode(CullMode::BACK);
					rs->setScissorEnabled(true);

					auto renderableMesh = new RenderableMesh();
					renderData.renderables.emplace_back(renderableMesh);
					renderableMesh->attachSceneNode(modelNode);
					auto pass = new RenderPass();
					pass->state = new RenderState();
					pass->state->rasterizer.state = rs;
					pass->material = mat;
					pass->tags = tag1;
					renderableMesh->renderPasses.emplace_back(pass);
					{
						auto subPass = new RenderPass();
						subPass->state = new RenderState();
						subPass->state->rasterizer.state = rs;
						subPass->material = mat;
						subPass->tags = tag2;

						pass->subPasses.emplace_back(subPass);
					}
					auto mesh = new Mesh();
					mesh->setBuffer(new MeshBuffer());
					renderableMesh->setMesh(mesh);
					renderableMesh->setRenderer(renderer);

					for (auto& itr : mr->getVerteices()) {
						auto& vs = itr.second;
						auto vb = graphics->createVertexBuffer();
						vb->create(vs.resource->data.getLength(), Usage::NONE, Usage::NONE, vs.resource->data.getSource(), vs.resource->data.getLength());
						vb->setStride(vs.resource->stride);
						mesh->getBuffer()->getVertices()->set(itr.first, VertexAttribute<IVertexBuffer>(vb, vs.desc));
					}

					if (auto is = mr->index; is) {
						auto ib = graphics->createIndexBuffer();
						ib->create(is->data.getLength(), Usage::NONE, Usage::NONE, is->data.getSource(), is->data.getLength());
						ib->setFormat(is->type);
						mesh->getBuffer()->setIndex(ib);
					}
				}
			}

			{
				IntrusivePtr s = new Shader();

				mat->setShader(s);
				mat->setParameters(new ShaderParameterCollection());
				auto shaderResourcesFolder = Application::getAppPath().parent_path().u8string() + "/Resources/shaders/";
				//s->upload(std::filesystem::path(app->getAppPath().parent_path().u8string() + "/Resources/shaders/test.shader"));
				extensions::ShaderScript::set(s, graphics, readFile(Application::getAppPath().parent_path().u8string() + "/Resources/shaders/lighting.shader"),
					[shaderResourcesFolder](const ProgramIncludeInfo& info) {
						return readFile(shaderResourcesFolder + info.file);
					},
					[](const ProgramInputInfo& info) {
						return ProgramInputDescriptor();
					}, programTranspileHandler);

				mat2->setShader(s);
				mat2->setParameters(new ShaderParameterCollection());
				{
					mat->getParameters()->set(ShaderPredefine::DIFFUSE_COLOR, new ShaderParameter(ShaderParameterUsage::EXCLUSIVE))->set(Vec3f32::ONE);
					//mat->getParameters()->set(ShaderPredefine::DIFFUSE_COLOR, new ShaderParameter(ShaderParameterUsage::EXCLUSIVE))->set(Vec3f32(1, 0, 0));
					//mat2->getParameters()->set(ShaderPredefine::DIFFUSE_COLOR, new ShaderParameter(ShaderParameterUsage::EXCLUSIVE))->set(Vec3f32(0, 1, 0));
				}
			}

			renderData.wrold = worldNode;
			renderData.renderPipeline = new StandardRenderPipeline();
			renderData.renderPipeline->getShaderParameters().set(ShaderPredefine::AMBIENT_COLOR, new ShaderParameter())->set(Vec3f32());
			renderData.renderPipeline->getShaderParameters().set(ShaderPredefine::DIFFUSE_COLOR, new ShaderParameter())->set(Vec3f32::ONE);
			renderData.renderPipeline->getShaderParameters().set(ShaderPredefine::SPECULAR_COLOR, new ShaderParameter())->set(Vec3f32::ONE);

			auto aabbccStruct = new ShaderParameterCollection();
			aabbccStruct->set("val1", new ShaderParameter(ShaderParameterUsage::EXCLUSIVE))->set<float32_t>(0.8f).setUpdated();
			float32_t val2[] = { 1.0f, 1.0f };
			aabbccStruct->set("val2", new ShaderParameter(ShaderParameterUsage::EXCLUSIVE))->set<float32_t>(val2, sizeof(val2), sizeof(float32_t), true).setUpdated();
			aabbccStruct->set("val3", new ShaderParameter())->set(Vec4f32::ONE).setUpdated();

			renderData.renderPipeline->getShaderParameters().set("blue", new ShaderParameter())->set(aabbccStruct);
		}

		{
			auto texRes = graphics->createTexture2DResource();
			if (texRes) {
				auto img0 = extensions::PNGConverter::decode(readFile(Application::getAppPath().parent_path().u8string() + "/Resources/white.png"));
				img0->format = textureFormatTypeSwitch(img0->format, false);
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

				auto hr = texRes->create(img0->dimensions, 0, 1, 1, img0->format, Usage::NONE, Usage::MAP_WRITE, mipsData0Ptr.data());

				auto texView = graphics->createTextureView();
				texView->create(texRes, 0, -1, 0, -1);

				renderData.material->getParameters()->set(ShaderPredefine::DIFFUSE_TEXTURE, new ShaderParameter(ShaderParameterUsage::AUTO))->set(texView);
				renderData.material2->getParameters()->set(ShaderPredefine::DIFFUSE_TEXTURE, new ShaderParameter(ShaderParameterUsage::AUTO))->set(texView);
			}

			auto sam = graphics->createSampler();
			if (sam) {
				//sam->setMipLOD(0, 0);
				//sam->setAddress(SamplerAddressMode::WRAP, SamplerAddressMode::WRAP, SamplerAddressMode::WRAP);
				sam->setFilter(SamplerFilterOperation::NORMAL, SamplerFilterMode::POINT, SamplerFilterMode::POINT, SamplerFilterMode::POINT);
				renderData.material->getParameters()->set(ShaderPredefine::DIFFUSE_TEXTURE + "Sampler", new ShaderParameter(ShaderParameterUsage::AUTO))->set(sam);
				renderData.material2->getParameters()->set(ShaderPredefine::DIFFUSE_TEXTURE + "Sampler", new ShaderParameter(ShaderParameterUsage::AUTO))->set(sam);
			}
		}

		win->getEventDispatcher()->addEventListener(WindowEvent::CLOSING, new EventListener(std::function([](Event<WindowEvent>& e) {
			//*e.getData<bool>() = true;
			})));

		win->getEventDispatcher()->addEventListener(WindowEvent::CLOSED, new EventListener(std::function([renderData](Event<WindowEvent>& e) {
			renderData.looper->stop();
			})));

		win->getEventDispatcher()->addEventListener(WindowEvent::RESIZED, new EventListener(std::function([this, renderData](Event<WindowEvent>& e) {
			auto win = (IWindow*)e.getTarget();
			_resize(*renderData.camera, win->getContentSize());
			})));

		renderData.looper->getEventDispatcher()->addEventListener(LooperEvent::TICKING, new EventListener(std::function([renderData](Event<LooperEvent>& e) {
			auto dt = float32_t(*e.getData<float64_t>());

			while (renderData.winModule->processEvent()) {};

			renderData.model->localRotate(Quaternion<float32_t>(nullptr).rotationY(Math::PI<float32_t>* dt * 0.5f));

			renderData.g->setViewport(Box2i32ui32(Vec2i32::ZERO, renderData.win->getContentSize()));
			renderData.g->setScissor(Box2i32ui32(Vec2i32::ZERO, renderData.win->getContentSize() / 2));
			renderData.renderPipeline->render(renderData.g, [renderData](render::IRenderCollector& collector) {
				collector.addCamera(renderData.camera);
				for (auto& r : renderData.renderables) collector.addRenderable(r);
				for (auto& l : renderData.lights) collector.addLight(l);
				});
			renderData.g->present();
			})));

		(new Stats())->run(renderData.looper);
		_resize(*renderData.camera, win->getContentSize());
		win->setVisible(true);
		renderData.looper->run(true);

		return 0;
	}

private:
	IntrusivePtr<SceneNode> _worldRoot;

	void _resize(Camera& cam, const Vec2ui32& size) {
		constexpr auto& zero = Math::ZERO<std::remove_cvref_t<decltype(size)>::ElementType>;
		if (size[0] == zero || size[1] == zero) return;
		cam.setProjectionMatrix(Matrix4x4f32(nullptr).perspectiveFov(Math::PI<float32_t> / 6.f, (float32_t)size[0] / (float32_t)size[1], 10.f, 10000.f));
	}
};