#pragma once

#include "srk/math/Matrix.h"
#include "srk/render/RenderPass.h"
#include "srk/render/RenderPriority.h"

namespace srk {
	class Material;
	class Mesh;
	class MeshResource;
}

namespace srk::components::renderables {
	class IRenderable;
}

namespace srk::render {
	class IRenderer;
	class RenderState;


	class SRK_FW_DLL RenderData {
	public:
		RenderData() {
			reset();
		}

		template<typename T>
		class Getter {
		public:
			using Fn = const T*(*)(void*);

			Getter(Fn fn = nullptr, void* target = nullptr) :
				_fn(fn),
				_target(target) {
			}

			Getter(const Getter& value) :
				_fn(value._fn),
				_target(value._target) {
			}

			Getter(Getter&& value) noexcept :
				_fn(value._fn),
				_target(value._target) {
			}

			inline SRK_CALL operator bool() const {
				return _fn;
			}

			inline Getter& SRK_CALL operator=(const Getter& value) {
				set(value._fn, value._target);
				return *this;
			}

			inline Getter& SRK_CALL operator=(const std::nullptr_t) {
				set(nullptr, nullptr);
				return *this;
			}

			inline const T* SRK_CALL operator()() {
				return _fn(_target);
			}

			inline void SRK_CALL set(Fn fn, void* target) {
				this->_fn = fn;
				this->_target = target;
			}

			inline void SRK_CALL reset() {
				set(nullptr, nullptr);
			}

		private:
			Fn _fn;
			void* _target;
		};

		const components::renderables::IRenderable* renderable;
		RenderPriority priority;
		RenderState* state;
		Material* material;
		Getter<Mesh> mesh;
		IRenderer* renderer;
		std::vector<IntrusivePtr<RenderPass>>* subPasses;

		struct {
			Matrix3x4f32 l2w;
			Matrix3x4f32 l2v;
			Matrix4x4f32 l2p;
		} matrix;

		inline void SRK_CALL set(const RenderData& data) {
			renderable = data.renderable;
			state = data.state;
			priority = data.priority;
			material = data.material;
			mesh = data.mesh;
			renderer = data.renderer;
			subPasses = data.subPasses;
		}

		inline void SRK_CALL reset() {
			renderable = nullptr;
			state = nullptr;
			priority.reset();
			material = nullptr;
			mesh = nullptr;
			renderer = nullptr;
			subPasses = nullptr;
		}
	};
}