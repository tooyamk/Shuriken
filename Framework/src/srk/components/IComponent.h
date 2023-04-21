#pragma once

#include "srk/Framework.h"
#include "srk/Intrusive.h"
#include "srk/RTTI.h"

namespace srk {
	class SceneNode;
}

namespace srk::components {
	class SRK_FW_DLL ClassInfo : public rtti::ClassInfo {
	public:
		ClassInfo(const ClassInfo* const base) : rtti::ClassInfo(base) {}
	};


	class SRK_FW_DLL IComponent : public Ref {
	SRK_RTTI_DECLARE_BASE(srk::components::ClassInfo);

	public:
		virtual ~IComponent();

		uint32_t layer;

		inline bool SRK_CALL isEnalbed() const {
			return _enabled;
		}
		inline void SRK_CALL setEnabled(bool b) {
			if (_enabled != b) {
				_enabled = b;
				_enabledChanged();
			}
		}

		inline SceneNode* SRK_CALL getSceneNode() const {
			return _node;
		}
		inline void SRK_CALL attachSceneNode(SceneNode* node) {
			auto old = _node;
			_node = node;
			_sceneNodeChanged(old);
		}

	protected:
		bool _enabled;
		SceneNode* _node;

		IComponent();

		virtual void SRK_CALL _sceneNodeChanged(SceneNode* old) {};
		virtual void SRK_CALL _enabledChanged() {};
	};
}


#define	SRK_COMPONENT_INHERIT(__BASE__) SRK_RTTI_INHERIT(srk::components::ClassInfo, __BASE__)