#pragma once

#include "aurora/Ref.h"
#include "aurora/RTTI.h"

namespace aurora {
	class Node;
}

namespace aurora::components {
	class AE_DLL ClassInfo : public rtti::ClassInfo {
	public:
		ClassInfo(const ClassInfo* const base, bool single) : rtti::ClassInfo(base) {
			if (base) {
				if (base->_singleBase) {
					_singleBase = base->_singleBase;
				} else {
					_singleBase = single ? this : nullptr;
				}
			} else {
				_singleBase = single ? this : nullptr;
			}
		}

		inline const ClassInfo* AE_CALL getSingleBase() const {
			return _singleBase;
		}

	private:
		const ClassInfo* _singleBase;
	};


	class AE_DLL IComponent : public Ref {
	AE_RTTI_DECLARE_BASE(aurora::components::ClassInfo, false);

	public:
		virtual ~IComponent();

		uint32_t layer;

		inline bool AE_CALL isEnalbed() const {
			return _enabled;
		}
		void AE_CALL setEnabled(bool b);

		inline Node* AE_CALL getNode() const {
			return _node;
		}

	protected:
		bool _enabled;
		Node* _node;

		friend Node;

		IComponent();

		void AE_CALL __setNode(Node* node);

		virtual void AE_CALL _nodeChanged(Node* old) {};
		virtual void AE_CALL _enabledChanged() {};
	};
}


#define	AE_COMPONENT_INHERIT(__BASE__, __SINGLE__) \
AE_RTTI_INHERIT(aurora::components::ClassInfo, __BASE__, __SINGLE__)