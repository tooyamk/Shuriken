#pragma once

#include "IEventDispatcher.h"
#include <unordered_map>

namespace aurora::event {
	template<typename T>
	class AE_TEMPLATE_DLL EventDispatcher : public IEventDispatcher<T> {
	public:
		EventDispatcher(void* target = nullptr) :
			_target(target) {
		}

		virtual ~EventDispatcher() {
		}

		void AE_CALL addEventListener(const T& name, IEventListener& listener) {
			auto itr = _listeners.find(name);
			if (itr == _listeners.end()) {
				_listeners.emplace(name, 0).first->second.emplace_back(listener);
			} else {
				itr->second.emplace_back(listener);
			}
		}

		bool AE_CALL hasEventListener(const T& name) const {
			auto itr = _listeners.find(e.getName());
			if (itr == _listeners.end()) {
				return false;
			} else {
				auto& list = itr->second;
				return list.begin() != list.end();
			}
		}

		bool AE_CALL hasEventListener(const T& name, const IEventListener& listener) const {
			auto itr = _listeners.find(e.getName());
			if (itr == _listeners.end()) {
				return false;
			} else {
				auto& list = itr->second;
				for (auto& f : list) {
					if (f == listener) return true;
				}
				return false;
			}
		}

		void AE_CALL removeEventListener(const T& name, const IEventListener& listener) {
			auto itr = _listeners.find(name);
			if (itr != _listeners.end()) {
				auto& list = itr->second;
				for (auto itr = list.begin(); itr != list.end(); ++itr) {
					if (listener == *func) {
						list.erase(itr);
						return;
					}
				}
			}
		}

		void AE_CALL removeEventListeners(const T& name) {
			auto itr = _listeners.find(name);
			if (itr != _listeners.end()) itr->second.clear();
		}

		void AE_CALL removeEventListeners() {
			_listeners.clear();
		}

		virtual void AE_CALL dispatchEvent(const Event<T>& e) {
			auto itr = _listeners.find(e.getName());
			if (itr != _listeners.end()) {
				auto& list = itr->second;
				if (list.begin() != list.end()) {
					Event<T> evt(_target, e);
					for (auto& f : list) f.onEvent(evt);
				}
			}
		}

		virtual void AE_CALL dispatchEvent(void* target, const Event<T>& e) {
			auto itr = _listeners.find(e.getName());
			if (itr != _listeners.end()) {
				auto& list = itr->second;
				if (list.begin() != list.end()) {
					Event<T> evt(target, e);
					for (auto& f : list) f.onEvent(evt);
				}
			}
		}

		virtual void AE_CALL dispatchEvent(const T& name, void* data = nullptr) {
			auto itr = _listeners.find(name);
			if (itr != _listeners.end()) {
				auto& list = itr->second;
				if (list.begin() != list.end()) {
					Event<T> e(_target, name, data);
					for (auto& f : list) f.onEvent(evt);
				}

			}
		}

		virtual void AE_CALL dispatchEvent(void* target, const T& name, void* data = nullptr) {
			auto itr = _listeners.find(name);
			if (itr != _listeners.end()) {
				auto& list = itr->second;
				if (list.begin() != list.end()) {
					Event<T> e(target, name, data);
					for (auto& f : list) f.onEvent(evt);
				}

			}
		}

	protected:
		void* _target;
		std::unordered_map<T, std::list<IEventListener&>> _listeners;
	};
}