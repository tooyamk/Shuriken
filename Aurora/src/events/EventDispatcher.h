#pragma once

#include "IEventDispatcher.h"
#include <mutex>
#include <unordered_map>

namespace aurora::events {
	template<typename EvtType, typename Lock>
	class AE_TEMPLATE_DLL EventDispatcher : public IEventDispatcher<EvtType> {
	public:
		EventDispatcher(void* target = nullptr) :
			_target(target) {
		}

		virtual ~EventDispatcher() {
		}

		virtual void AE_CALL addEventListener(const EvtType& type, IEventListener<EvtType>& listener) override {
			std::lock_guard<Lock> lck(_lock);

			auto itr = _listeners.find(type);
			if (itr == _listeners.end()) {
				_listeners.emplace(type, &listener);
			} else {
				auto& tl = itr->second;
				auto& list = tl.listeners;
				if (tl.numValidListeners) {
					if (tl.dispatching) {
						for (auto& f : list) {
							if (f.rawListener == &listener) {
								if (f.valid) {
									f.valid = false;
									--tl.numValidListeners;
								}
								break;
							}
						}
					} else {
						for (auto itr = list.begin(); itr != list.end(); ++itr) {
							if (&listener == (*itr).rawListener) {
								if ((*itr).valid) --tl.numValidListeners;
								list.erase(itr);
								--tl.numTotalListeners;
								break;
							}
						}
					}
				}

				list.emplace_back(&listener);
				++tl.numValidListeners;
				++tl.numTotalListeners;
			}
		}

		virtual bool AE_CALL hasEventListener(const EvtType& type) const  override {
			std::lock_guard<Lock> lck(_lock);

			auto itr = _listeners.find(type);
			if (itr == _listeners.end()) {
				return false;
			} else {
				return itr->second.numValidListeners > 0;
			}
		}

		virtual bool AE_CALL hasEventListener(const EvtType& type, const IEventListener<EvtType>& listener) const override {
			std::lock_guard<Lock> lck(_lock);

			auto itr = _listeners.find(type);
			if (itr == _listeners.end()) {
				return false;
			} else {
				auto& tl = itr->second;
				if (tl.numValidListeners) {
					auto& list = tl.listeners;
					for (auto& f : list) {
						if (f.rawListener == &listener) return f.valid;
					}
				}

				return false;
			}
		}

		virtual void AE_CALL removeEventListener(const EvtType& type, const IEventListener<EvtType>& listener) override {
			std::lock_guard<Lock> lck(_lock);

			auto itr = _listeners.find(type);
			if (itr != _listeners.end()) {
				auto& tl = itr->second;
				if (tl.numValidListeners) {
					auto& list = tl.listeners;
					if (tl.dispatching) {
						for (auto& f : list) {
							if (f.rawListener == &listener) {
								if (f.valid) {
									f.valid = false;
									--tl.numValidListeners;
								}
								return;
							}
						}
					} else {
						for (auto itr = list.begin(); itr != list.end(); ++itr) {
							if (&listener == (*itr).rawListener) {
								list.erase(itr);
								--tl.numValidListeners;
								--tl.numTotalListeners;
								return;
							}
						}
					}
				}
			}
		}

		virtual void AE_CALL removeEventListeners(const EvtType& type) override {
			std::lock_guard<Lock> lck(_lock);

			auto itr = _listeners.find(type);
			if (itr != _listeners.end()) _removeEventListeners(itr->second);
		}

		virtual void AE_CALL removeEventListeners() {
			std::lock_guard<Lock> lck(_lock);

			for (auto& itr : _listeners) _removeEventListeners(itr.second);
		}

		virtual void AE_CALL dispatchEvent(const Event<EvtType>& e) const override {
			dispatchEvent(_target, e.getType(), e.getData());
		}

		virtual void AE_CALL dispatchEvent(void* target, const Event<EvtType>& e) const override {
			dispatchEvent(target, e.getType(), e.getData());
		}

		virtual void AE_CALL dispatchEvent(const EvtType& type, void* data = nullptr) const override {
			dispatchEvent(_target, type, data);
		}

		virtual void AE_CALL dispatchEvent(void* target, const EvtType& type, void* data = nullptr) const override {
			std::lock_guard<Lock> lck(_lock);

			auto itr = _listeners.find(type);
			if (itr != _listeners.end()) {
				auto& tl = itr->second;
				if (tl.numValidListeners) {
					++tl.dispatching;

					Event<EvtType> evt(target, type, data);
					auto& list = tl.listeners;
					for (auto& f : list) {
						if (f.valid) f.rawListener->onEvent(evt);
					}

					--tl.dispatching;
				}

				if (!tl.dispatching && tl.numValidListeners != tl.numTotalListeners) {
					auto& list = tl.listeners;
					for (auto itr = list.begin(); itr != list.end();) {
						if ((*itr).valid) {
							++itr;
						} else {
							itr = list.erase(itr);
						}
					}
					tl.numValidListeners = tl.numTotalListeners;
				}
			}
		}

	protected:
		struct Listener {
			Listener(IEventListener<EvtType>* rawListener) :
				valid(true),
				rawListener(rawListener) {
			}
			bool valid;
			IEventListener<EvtType>* rawListener;
		};


		struct TypeListeners {
			TypeListeners(IEventListener<EvtType>* rawListener) :
				dispatching(false),
				numValidListeners(1),
				numTotalListeners(1) {
				listeners.emplace_back(rawListener);
			}
			ui32 dispatching;
			ui32 numValidListeners;
			ui32 numTotalListeners;
			std::list<Listener> listeners;
		};


		void* _target;
		mutable Lock _lock;
		mutable std::unordered_map<EvtType, TypeListeners> _listeners;

		void AE_CALL _removeEventListeners(TypeListeners& typeListeners) {
			if (typeListeners.numValidListeners) {
				auto& list = typeListeners.listeners;
				if (typeListeners.dispatching) {
					for (auto& f : list) {
						if (f.valid) {
							f.valid = false;
							--typeListeners.numValidListeners;
						}
					}
				} else {
					list.clear();
					typeListeners.numValidListeners = 0;
					typeListeners.numTotalListeners = 0;
				}
			}
		}
	};
}