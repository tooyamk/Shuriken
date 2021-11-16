#pragma once

#include "../BaseTester.h"
#include "aurora/SerializableObject.h"

class RenderPipelineTester : public BaseTester {
public:
	virtual int32_t AE_CALL run() override {
		auto monitors = Monitor::getMonitors();
		auto vms = monitors[0].getVideoModes();
		
		IntrusivePtr app = new Application("TestApp");

		ApplicationStyle wndStype;
		wndStype.thickFrame = true;
		if (app->createWindow(wndStype, "", Vec2ui32(800, 600), false)) {
			IntrusivePtr gml = new GraphicsModuleLoader();

			//if (gml->load("libs/" + getDLLName("ae-graphics-d3d11"))) {
			if (gml->load("libs/" + getDLLName("ae-graphics-gl"))) {
			//if (gml->load("libs/" + getDLLName("ae-graphics-vulkan"))) {
				SerializableObject args;
				
				IntrusivePtr gpstml = new ModuleLoader<IProgramSourceTranslator>();
				gpstml->load("libs/" + getDLLName("ae-program-source-translator"));

				args.insert("dxc", "libs/" + getDLLName("dxcompiler"));
				auto gpst = gpstml->create(&args);

				args.insert("app", app.uintptr());
				args.insert("sampleCount", 4);
				args.insert("trans", gpst.uintptr());
				//args.insert("driverType", "SOFTWARE");
				args.insert("debug", Environment::IS_DEBUG);

				auto graphics = gml->create(&args);

				if (graphics) {
					printaln("Graphics Version : ", graphics->getVersion());

					graphics->getEventDispatcher()->addEventListener(GraphicsEvent::ERR, createEventListener<GraphicsEvent>([](Event<GraphicsEvent>& e) {
						printaln(*(std::string_view*)e.getData());
						int a = 1;
					}));

					app->getEventDispatcher()->addEventListener(ApplicationEvent::RESIZED, createEventListener<ApplicationEvent>([graphics](Event<ApplicationEvent>& e) {
						graphics->setBackBufferSize(((IApplication*)e.getTarget())->getCurrentClientSize());
					}));

					//graphics->getEventDispatcher().addEventListener(GraphicsEvent::ERR, new EventListener([](Event<GraphicsEvent>& e) {
					//	println(*(std::string_view*)e.getData());
					//	int a = 1;
					//}));

					struct {
						IntrusivePtr<Application> app;
						IntrusivePtr<Looper> looper;
						IntrusivePtr<IGraphicsModule> g;
						IntrusivePtr<Node> wrold;
						IntrusivePtr<Node> model;
						IntrusivePtr<Camera> camera;
						IntrusivePtr<Material> material;
						IntrusivePtr<Material> material2;
						IntrusivePtr<StandardRenderPipeline> renderPipeline;
						std::vector<IntrusivePtr<ILight>> lights;
						std::vector<IntrusivePtr<IRenderable>> renderables;
					} renderData;
					renderData.app = app;
					renderData.looper = new Looper(1.0 / 60.0);
					renderData.g = graphics;

					{
						IntrusivePtr worldNode = new Node();
						IntrusivePtr modelNode = worldNode->addChild<Node>();
						//modelNode = new Node();
						//modelNode->setLocalScale(Vec3f32(4));
						//modelNode->parentTranslate(Vec3f32(0, 3000.f, 0));
						//modelNode->localRotate(Quaternion::createFromEulerY(Math::PI<f32> * 0.8f));
						//modelNode->localRotate(Quaternion::createFromEulerY(Math::PI<f32>* 0.15f));
						//auto lm = modelNode->getLocalMatrix();
						//auto wm = modelNode->getWorldMatrix();
						renderData.model = modelNode;
						if (1) {
							IntrusivePtr lightNode = worldNode->addChild<Node>();
							lightNode->setLocalPosition(Vec3f32(-100, 0, -100));
							auto light = renderData.lights.emplace_back(new PointLight());
							light->attachNode(lightNode);
							//light->setRadius(200);
							lightNode->localRotate(Quaternion::createEulerY(Math::PI_4<float32_t>));
							renderData.lights.emplace_back(light);
						}
						if (1) {
							IntrusivePtr lightNode = worldNode->addChild<Node>();
							lightNode->setLocalPosition(Vec3f32(100, 0, -100));
							auto light = renderData.lights.emplace_back(new PointLight());
							light->attachNode(lightNode);
							//light->setRadius(1000);
							lightNode->localRotate(Quaternion::createEulerY(-Math::PI_4<float32_t>));
							renderData.lights.emplace_back(light);
						}
						IntrusivePtr cameraNode = worldNode->addChild<Node>();
						//auto camera = new Camera();
						//cameraNode->addComponent(camera);
						renderData.camera = new Camera();
						renderData.camera->attachNode(cameraNode);
						auto mat = new Material();
						renderData.material = mat;
						auto mat2 = new Material();
						renderData.material2 = mat2;
						IntrusivePtr renderer = new ForwardRenderer(*graphics);

						{
							renderData.camera->getNode()->localTranslate(Vec3f32(0.f, 0.f, -200.f));
						}

						RenderTag forwardBaseTag("forward_base");
						RenderTag forwardAddTag("forward_add");

						IntrusivePtr tag1 = new RenderTagCollection();
						tag1->add(forwardBaseTag);

						IntrusivePtr tag2 = new RenderTagCollection();
						tag2->add(forwardAddTag);

						auto parsed = extensions::FBXConverter::decode(readFile(app->getAppPath().parent_path().u8string() + "/Resources/teapot.fbx"));
						for (auto& mr : parsed.meshes) {
							if (mr) {
								auto rs = graphics->createRasterizerState();
								rs->setFillMode(FillMode::SOLID);
								rs->setFrontFace(FrontFace::CW);
								rs->setCullMode(CullMode::BACK);

								auto renderableMesh = new RenderableMesh();
								renderData.renderables.emplace_back(renderableMesh);
								renderableMesh->attachNode(modelNode);
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
									auto vs = itr.second;
									auto vb = graphics->createVertexBuffer();
									vb->create(vs->data.getLength(), Usage::NONE, vs->data.getSource(), vs->data.getLength());
									vb->setFormat(vs->format);
									mesh->getBuffer()->getVertices()->set(itr.first, vb);
								}

								if (auto is = mr->index; is) {
									auto ib = graphics->createIndexBuffer();
									ib->create(is->data.getLength(), Usage::NONE, is->data.getSource(), is->data.getLength());
									ib->setFormat(is->type);
									mesh->getBuffer()->setIndex(ib);
								}
							}
						}

						{
							IntrusivePtr s = new Shader();

							mat->setShader(s);
							mat->setParameters(new ShaderParameterCollection());
							auto shaderResourcesFolder = app->getAppPath().parent_path().u8string() + "/Resources/shaders/";
							//s->upload(std::filesystem::path(app->getAppPath().parent_path().u8string() + "/Resources/shaders/test.shader"));
							extensions::ShaderScript::set(s, graphics, readFile(app->getAppPath().parent_path().u8string() + "/Resources/shaders/lighting.shader"),
								[shaderResourcesFolder](const Shader& shader, ProgramStage stage, const std::string_view& name) {
								return readFile(shaderResourcesFolder + name);
							},
								[](const Shader& shader, const std::string_view& name) {
								return modules::graphics::IProgram::InputDescription();
							});

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
						renderData.renderPipeline->getShaderParameters().set(ShaderPredefine::AMBIENT_COLOR, new ShaderParameter())->set(Vec3f32(0.0f));
						renderData.renderPipeline->getShaderParameters().set(ShaderPredefine::DIFFUSE_COLOR, new ShaderParameter())->set(Vec3f32::ONE);
						renderData.renderPipeline->getShaderParameters().set(ShaderPredefine::SPECULAR_COLOR, new ShaderParameter())->set(Vec3f32::ONE);
					}

					{
						auto texRes = graphics->createTexture2DResource();
						if (texRes) {
							auto img0 = extensions::PNGConverter::decode(readFile(app->getAppPath().parent_path().u8string() + "/Resources/white.png"));
							auto mipLevels = Image::calcMipLevels(img0->size);
							ByteArray mipsData0;
							std::vector<void*> mipsData0Ptr;
							img0->generateMips(img0->format, mipLevels, mipsData0, mipsData0Ptr);

							auto img1 = extensions::PNGConverter::decode(readFile(app->getAppPath().parent_path().u8string() + "/Resources/red.png"));
							ByteArray mipsData1;
							std::vector<void*> mipsData1Ptr;
							img1->generateMips(img1->format, mipLevels, mipsData1, mipsData1Ptr);

							mipsData0Ptr.insert(mipsData0Ptr.end(), mipsData1Ptr.begin(), mipsData1Ptr.end());

							auto hr = texRes->create(img0->size, 0, 1, 1, img0->format, Usage::IGNORE_UNSUPPORTED | Usage::MAP_WRITE, mipsData0Ptr.data());

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

					app->getEventDispatcher()->addEventListener(ApplicationEvent::CLOSING, new EventListener(std::function([](Event<ApplicationEvent>& e) {
						//*e.getData<bool>() = true;
					})));

					app->getEventDispatcher()->addEventListener(ApplicationEvent::CLOSED, new EventListener(std::function([renderData](Event<ApplicationEvent>& e) {
						renderData.looper->stop();
					})));

					app->getEventDispatcher()->addEventListener(ApplicationEvent::RESIZED, new EventListener(std::function([this, renderData](Event<ApplicationEvent>& e) {
						auto app = (IApplication*)e.getTarget();
						_resize(*renderData.camera, app->getCurrentClientSize());
					})));

					renderData.looper->getEventDispatcher()->addEventListener(LooperEvent::TICKING, new EventListener(std::function([renderData](Event<LooperEvent>& e) {
						auto dt = float32_t(*e.getData<float64_t>());

						renderData.app->pollEvents();

						renderData.model->localRotate(Quaternion::createEulerY(Math::PI<float32_t> * dt * 0.5f));

						renderData.g->setViewport(Box2i32ui32(Vec2i32::ZERO, renderData.app->getCurrentClientSize()));
						renderData.renderPipeline->render(renderData.g, [renderData](render::IRenderCollector& collector) {
							collector.addCamera(renderData.camera);
							for (auto& r : renderData.renderables) collector.addRenderable(r);
							for (auto& l : renderData.lights) collector.addLight(l);
						});
						renderData.g->present();
					})));

					(new Stats())->run(renderData.looper);
					_resize(*renderData.camera, app->getCurrentClientSize());
					app->setVisible(true);
					renderData.looper->run(true);
				}
			}
		}

		return 0;
	}

private:
	IntrusivePtr<Node> _worldRoot;

	void _resize(Camera& cam, const Vec2ui32& size) {
		constexpr auto& zero = Math::ZERO<std::remove_cvref_t<decltype(size)>::ElementType>;
		if (size[0] == zero || size[1] == zero) return;
		cam.setProjectionMatrix(Matrix44::createPerspectiveFovLH(Math::PI<float32_t> / 6.f, (float32_t)size[0] / (float32_t)size[1], 10, 10000));
	}
};