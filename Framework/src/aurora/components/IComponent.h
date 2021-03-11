#pragma once

#include "aurora/Intrusive.h"
#include "aurora/RTTI.h"

namespace aurora {
	class Node;
}

namespace aurora::components {
	class AE_FW_DLL ClassInfo : public rtti::ClassInfo {
	public:
		ClassInfo(const ClassInfo* const base) : rtti::ClassInfo(base) {}
	};


	class AE_FW_DLL IComponent : public Ref {
	AE_RTTI_DECLARE_BASE(aurora::components::ClassInfo);

	public:
		virtual ~IComponent();

		uint32_t layer;

		inline bool AE_CALL isEnalbed() const {
			return _enabled;
		}
		inline void AE_CALL setEnabled(bool b) {
			if (_enabled != b) {
				_enabled = b;
				_enabledChanged();
			}
		}

		inline Node* AE_CALL getNode() const {
			return _node;
		}
		inline void AE_CALL attachNode(Node* node) {
			auto old = _node;
			_node = node;
			_nodeChanged(old);
		}

	protected:
		bool _enabled;
		Node* _node;

		IComponent();

		virtual void AE_CALL _nodeChanged(Node* old) {};
		virtual void AE_CALL _enabledChanged() {};
	};
}


#define	AE_COMPONENT_INHERIT(__BASE__) AE_RTTI_INHERIT(aurora::components::ClassInfo, __BASE__)