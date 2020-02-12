#pragma once

#include "aurora/AList.h"
#include "aurora/Application.h"
#include "aurora/ByteArray.h"
#include "aurora/Image.h"
#include "aurora/Lock.h"
#include "aurora/Monitor.h"
#include "aurora/Node.h"
#include "aurora/RTTI.h"
#include "aurora/String.h"
#include "aurora/Time.h"
#include "aurora/TimeWheel.h"

#include "aurora/events/EventDispatcher.h"

#include "aurora/math/Box.h"
#include "aurora/math/Matrix.h"
#include "aurora/math/Quaternion.h"
#include "aurora/math/Vector.h"

#include "aurora/modules/ModuleLoader.h"
#include "aurora/modules/graphics/GraphicsAdapter.h"
#include "aurora/modules/graphics/GraphicsUtils.h"
#include "aurora/modules/graphics/IProgramSourceTranslator.h"
#include "aurora/modules/graphics/ProgramSource.h"
#include "aurora/modules/graphics/ShaderParameter.h"
#include "aurora/modules/graphics/ShaderParameterFactory.h"
#include "aurora/modules/graphics/VertexBufferFactory.h"

#include "aurora/components/Camera.h"

#include "aurora/hash/CRC.h"
#include "aurora/hash/MD5.h"
#include "aurora/hash/xxHash.h"