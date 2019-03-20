#pragma once

#include "IEventDispatcher.h"
#include <unordered_map>

namespace aurora::event {
	template<typename EvtType>
	class AE_TEMPLATE_DLL EventDispatcher : public IEventDispatcher<EvtType> {
	public:
		EventDispatcher(void* target = nullptr) :
			_target(target) {
		}

		virtual ~EventDispatcher() {
		}

		virtual void AE_CALL addEventListener(const EvtType& type, IEventListener<EvtType>& listener) override {
			auto itr = _listeners.find(type);
			if (itr == _listeners.end()) {
				_listeners.emplace(type, 0).first->second.emplace_back(&listener);
			} else {
				itr->second.emplace_back(&listener);
			}
		}

		virtual bool AE_CALL hasEventListener(const EvtType& type) const  override {
			auto itr = _listeners.find(type);
			if (itr == _listeners.end()) {
				return false;
			} else {
				auto& list = itr->second;
				return list.begin() != list.end();
			}
		}

		virtual bool AE_CALL hasEventListener(const EvtType& type, const IEventListener<EvtType>& listener) const override {
			auto itr = _listeners.find(type);
			if (itr == _listeners.end()) {
				return false;
			} else {
				auto& list = itr->second;
				for (auto& f : list) {
					if (f == &listener) return true;
				}
				return false;
			}
		}

		virtual void AE_CALL removeEventListener(const EvtType& type, const IEventListener<EvtType>& listener) override {
			auto itr = _listeners.find(type);
			if (itr != _listeners.end()) {
				auto& list = itr->second;
				for (auto itr = list.begin(); itr != list.end(); ++itr) {
					if (&listener == *itr) {
						list.erase(itr);
						return;
					}
				}
			}
		}

		virtual void AE_CALL removeEventListeners(const EvtType& type) override {
			auto itr = _listeners.find(type);
			if (itr != _listeners.end()) itr->second.clear();
		}

		virtual void AE_CALL removeEventListeners() {
			_listeners.clear();
		}

		virtual void AE_CALL dispatchEvent(const Event<EvtType>& e) override {
			auto itr = _listeners.find(e.getType());
			if (itr != _listeners.end()) {
				auto& list = itr->second;
				if (list.begin() != list.end()) {
					Event<EvtType> evt(_target, e);
					for (auto& f : list) f->onEvent(evt);
				}
			}
		}

		virtual void AE_CALL dispatchEvent(void* target, const Event<EvtType>& e) override {
			auto itr = _listeners.find(e.getType());
			if (itr != _listeners.end()) {
				auto& list = itr->second;
				if (list.begin() != list.end()) {
					Event<EvtType> evt(target, e);
					for (auto& f : list) f->onEvent(evt);
				}
			}
		}

		virtual void AE_CALL dispatchEvent(const EvtType& type, void* data = nullptr) override {
			auto itr = _listeners.find(type);
			if (itr != _listeners.end()) {
				auto& list = itr->second;
				if (list.begin() != list.end()) {
					Event<EvtType> evt(_target, type, data);
					for (auto& f : list) f->onEvent(evt);
				}

			}
		}

		virtual void AE_CALL dispatchEvent(void* target, const EvtType& type, void* data = nullptr) override {
			auto itr = _listeners.find(type);
			if (itr != _listeners.end()) {
				auto& list = itr->second;
				if (list.begin() != list.end()) {
					Event<EvtType> evt(target, type, data);
					for (auto& f : list) f->onEvent(evt);
				}
			}
		}

	protected:
		void* _target;
		std::unordered_map<EvtType, std::list<IEventListener<EvtType>*>> _listeners;
	};
}