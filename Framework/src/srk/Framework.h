#pragma once

#include "srk/Core.h"

#include "srk/applications/Application.h"
#include "srk/GraphicsBuffer.h"
#include "srk/Image.h"
#include "srk/Looper.h"
#include "srk/Material.h"
#include "srk/Mesh.h"
#include "srk/Monitor.h"
#include "srk/Node.h"
#include "srk/ProgramSource.h"
#include "srk/Shader.h"
#include "srk/ShaderDefine.h"
#include "srk/ShaderParameter.h"
#include "srk/ShaderPredefine.h"

#include "srk/modules/ModuleLoader.h"
#include "srk/modules/graphics/GraphicsAdapter.h"
#include "srk/modules/graphics/IShaderTranspiler.h"

#include "srk/components/Camera.h"
#include "srk/components/renderables/RenderableMesh.h"
#include "srk/components/lights/DirectionLight.h"
#include "srk/components/lights/PointLight.h"
#include "srk/components/lights/SpotLight.h"

#include "srk/render/ForwardRenderer.h"
#include "srk/render/SpriteRenderer.h"
#include "srk/render/StandardRenderPipeline.h"