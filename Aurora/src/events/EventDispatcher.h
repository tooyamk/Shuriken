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

		void AE_CALL addEventListener(const T& name, EVENT_LISTENER listener) {
			if (listener) {
				auto itr = _listeners.find(name);
				if (itr == _listeners.end()) {
					_listeners.emplace(name, 0).first->second.emplace_back(listener);
				} else {
					auto& list = itr->second;
					for (auto& f : list) {
						auto func = f.target<EVENT_LISTENER>();
						if (func && listener == *func) return;
					}
					list.emplace_back(listener);
				}
			}
		}

		template<typename C>
		void AE_CALL addEventListener(const T& name, C* c, EVENT_CLASS_LISTENER<C> listener) {
			if (c && listener) {
				auto itr = _listeners.find(name);
				if (itr == _listeners.end()) {
					_listeners.emplace(name, 0).first->second.emplace_back(DelagateMethodListener<C>(c, listener));
				} else {
					auto& list = itr->second;
					for (auto& f : list) {
						auto func = f.target<DelagateMethodListener<C>>();
						if (func && func->c == c && func->listener == listener) return;
					}
					list.emplace_back(DelagateMethodListener<C>(c, listener));
				}
			}
		}

		void AE_CALL addEventListener(const T& name, FunctionListener* listener) {
			if (listener) {
				auto itr = _listeners.find(name);
				if (itr == _listeners.end()) {
					_listeners.emplace(name, 0).first->second.emplace_back(DelagateFunctionListener(listener));
				} else {
					auto& list = itr->second;
					for (auto& f : list) {
						auto func = f.target<DelagateFunctionListener>();
						if (func && func->listener == listener) return;
					}
					list.emplace_back(DelagateFunctionListener(listener));
				}
			}
		}

		void AE_CALL removeEventListeners() {
			_listeners.clear();
		}

		void AE_CALL removeEventListeners(const T& name) {
			auto itr = _listeners.find(name);
			if (itr != _listeners.end()) itr->second.clear();
		}

		void AE_CALL removeEventListener(const T& name, EVENT_LISTENER listener) {
			if (listener) {
				auto itr = _listeners.find(name);
				if (itr != _listeners.end()) {
					auto& list = itr->second;
					for (auto itr = list.begin(); itr != list.end(); ++itr) {
						auto func = (*itr).target<EVENT_LISTENER>();
						if (func && listener == *func) {
							list.erase(itr);
							return;
						}
					}
				}
			}
		}

		template<typename C>
		void AE_CALL removeEventListener(const T& name, C* c, EVENT_CLASS_LISTENER<C> listener) {
			if (c && listener) {
				auto itr = _listeners.find(name);
				if (itr != _listeners.end()) {
					auto& list = itr->second;
					for (auto itr = list.begin(); itr != list.end(); ++itr) {
						auto func = (*itr).target<DelagateMethodListener<C>>();
						if (func && func->c == c && func->listener == listener) {
							list.erase(itr);
							return;
						}
					}
				}
			}
		}

		void AE_CALL removeEventListener(const T& name, FunctionListener* listener) {
			if (listener) {
				auto itr = _listeners.find(name);
				if (itr != _listeners.end()) {
					auto& list = itr->second;
					for (auto itr = list.begin(); itr != list.end(); ++itr) {
						auto func = (*itr).target<DelagateFunctionListener>();
						if (func && func->listener == listener) {
							list.erase(itr);
							return;
						}
					}
				}
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

		virtual void AE_CALL dispatchEvent(const Event<T>& e) {
			auto itr = _listeners.find(e.getName());
			if (itr != _listeners.end()) {
				auto& list = itr->second;
				if (list.begin() != list.end()) {
					Event<T> evt(_target, e);
					for (auto& f : list) f(evt);
				}
			}
		}

		virtual void AE_CALL dispatchEvent(void* target, const Event<T>& e) {
			auto itr = _listeners.find(e.getName());
			if (itr != _listeners.end()) {
				auto& list = itr->second;
				if (list.begin() != list.end()) {
					Event<T> evt(target, e);
					for (auto& f : list) f(evt);
				}
			}
		}

		virtual void AE_CALL dispatchEvent(const T& name, void* data = nullptr) {
			auto itr = _listeners.find(name);
			if (itr != _listeners.end()) {
				auto& list = itr->second;
				if (list.begin() != list.end()) {
					Event<T> e(_target, name, data);
					for (auto& f : list) f(e);
				}

			}
		}

		virtual void AE_CALL dispatchEvent(void* target, const T& name, void* data = nullptr) {
			auto itr = _listeners.find(name);
			if (itr != _listeners.end()) {
				auto& list = itr->second;
				if (list.begin() != list.end()) {
					Event<T> e(target, name, data);
					for (auto& f : list) f(e);
				}

			}
		}

	protected:
		void* _target;
		std::unordered_map<T, std::list<Function>> _listeners;

		template<typename C>
		class DelagateMethodListener {
		public:
			DelagateMethodListener(C* c, EVENT_CLASS_LISTENER<C> listener) :
				c(c),
				listener(listener) {
			}

			C* c;
			EVENT_CLASS_LISTENER<C> listener;

			void operator()(Event<T>& e) {
				return (c->*listener)(e);
			}
		};


		class DelagateFunctionListener {
		public:
			DelagateFunctionListener(FunctionListener* listener) :
				listener(listener) {
			}

			FunctionListener* listener;

			void operator()(Event<T>& e) {
				return (*listener)(e);
			}
		};
	};
}