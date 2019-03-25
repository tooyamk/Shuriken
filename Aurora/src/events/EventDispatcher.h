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

		virtual bool AE_CALL addEventListener(const EvtType& type, IEventListener<EvtType>& listener, bool ref) override {
			std::lock_guard<Lock> lck(_lock);

			bool rst = true;
			auto itr = _listeners.find(type);
			if (itr == _listeners.end()) {
				if (ref) listener.ref();
				_listeners.emplace(std::piecewise_construct,
					std::forward_as_tuple(type),
					std::forward_as_tuple(&listener, ref));
			} else {
				auto& tl = itr->second;
				auto& list = tl.listeners;
				bool oldRef = false;
				if (tl.numValidListeners) {
					if (tl.dispatching) {
						for (auto& f : list) {
							if (f.rawListener == &listener) {
								if (f.valid) {
									f.valid = false;
									rst = false;
									oldRef = f.ref;
									--tl.numValidListeners;
								}
								break;
							}
						}
					} else {
						for (auto itr = list.begin(); itr != list.end(); ++itr) {
							if (&listener == (*itr).rawListener) {
								rst = false;
								oldRef = (*itr).ref;
								--tl.numValidListeners;

								list.erase(itr);
								--tl.numTotalListeners;
								break;
							}
						}
					}
				}

				list.emplace_back(&listener, ref);
				++tl.numValidListeners;
				++tl.numTotalListeners;

				if (oldRef) {
					if (!ref) listener.unref();
				} else if (ref) {
					listener.ref();
				}
			}

			return rst;
		}

		virtual ui32 AE_CALL hasEventListener(const EvtType& type) const  override {
			std::lock_guard<Lock> lck(_lock);

			auto itr = _listeners.find(type);
			if (itr == _listeners.end()) {
				return 0;
			} else {
				return itr->second.numValidListeners;
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
					if (tl.dispatching) {
						for (auto& f : list) {
							if (f.valid && f.rawListener == &listener) return true;
						}
					} else {
						for (auto& f : list) {
							if (f.rawListener == &listener) return true;
						}
					}
				}

				return false;
			}
		}

		virtual bool AE_CALL removeEventListener(const EvtType& type, const IEventListener<EvtType>& listener) override {
			std::lock_guard<Lock> lck(_lock);

			bool rst = false;
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
									rst = true;
									if (f.ref) f.rawListener->unref();
									--tl.numValidListeners;
								}
								break;
							}
						}
					} else {
						for (auto itr = list.begin(); itr != list.end(); ++itr) {
							if (&listener == (*itr).rawListener) {
								rst = true;
								if ((*itr).ref) (*itr).rawListener->unref();

								list.erase(itr);
								--tl.numValidListeners;
								--tl.numTotalListeners;
								break;
							}
						}
					}
				}
			}

			return rst;
		}

		virtual ui32 AE_CALL removeEventListeners(const EvtType& type) override {
			std::lock_guard<Lock> lck(_lock);

			auto itr = _listeners.find(type);
			if (itr != _listeners.end()) return _removeEventListeners(itr->second);
			return 0;
		}

		virtual ui32 AE_CALL removeEventListeners() {
			std::lock_guard<Lock> lck(_lock);

			ui32 n = 0;
			for (auto& itr : _listeners)  n += _removeEventListeners(itr.second);
			return n;
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
			Listener(IEventListener<EvtType>* rawListener, bool ref) :
				valid(true),
				ref(ref),
				rawListener(rawListener) {
			}
			bool valid;
			bool ref;
			IEventListener<EvtType>* rawListener;
		};


		struct TypeListeners {
			TypeListeners(IEventListener<EvtType>* rawListener, bool ref) :
				dispatching(false),
				numValidListeners(1),
				numTotalListeners(1) {
				listeners.emplace_back(rawListener, ref);
			}
			ui32 dispatching;
			ui32 numValidListeners;
			ui32 numTotalListeners;
			std::list<Listener> listeners;
		};


		void* _target;
		mutable Lock _lock;
		mutable std::unordered_map<EvtType, TypeListeners> _listeners;

		ui32 AE_CALL _removeEventListeners(TypeListeners& typeListeners) {
			ui32 n = typeListeners.numValidListeners;
			if (typeListeners.numValidListeners) {
				auto& list = typeListeners.listeners;
				if (typeListeners.dispatching) {
					for (auto& f : list) {
						if (f.valid) {
							f.valid = false;
							if (f.ref) f.rawListener->unref();
							--typeListeners.numValidListeners;
						}
					}
				} else {
					for (auto& f : list) {
						if (f.ref) f.rawListener->unref();
					}
					list.clear();
					typeListeners.numValidListeners = 0;
					typeListeners.numTotalListeners = 0;
				}
			}

			return n;
		}
	};
}