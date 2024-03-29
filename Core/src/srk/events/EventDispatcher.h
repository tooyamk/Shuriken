#pragma once

#include "IEventDispatcher.h"
#include <mutex>
#include <unordered_map>
#include <vector>

namespace srk::events {
	template<typename EvtType>
	class EventDispatcher : public IEventDispatcher<EvtType> {
	public:
		EventDispatcher() {}
		EventDispatcher(const EventDispatcher&) = delete;
		EventDispatcher(EventDispatcher&&) = delete;
		EventDispatcher& operator=(const EventDispatcher&) = delete;
		EventDispatcher& operator=(EventDispatcher&&) = delete;

		virtual ~EventDispatcher() {
			removeEventListeners();
		}

		using IEventDispatcher<EvtType>::addEventListener;
		virtual bool SRK_CALL addEventListener(const EvtType& type, IEventListener<EvtType>& listener) override {
			bool newAdd = true;

			std::scoped_lock lock(_mutex);

			if (auto pair = _listeners.emplace(std::piecewise_construct, std::forward_as_tuple(type), std::forward_as_tuple(&listener)); pair.second) {
				listener.ref();
			} else {
				auto& tl = pair.first->second;
				if (tl.template remove<false>(listener)) {
					newAdd = false;
				} else {
					listener.ref();
					++tl.numValidListeners;
				}

				tl.listeners.emplace_back(&listener);
				++tl.numTotalListeners;
			}

			return newAdd;
		}

		virtual uint32_t SRK_CALL getNumEventListeners() const override {
			uint32_t n = 0;

			std::scoped_lock lock(_mutex);

			for (auto& itr : _listeners) n += itr.second.numValidListeners;
			return n;
		}

		virtual uint32_t SRK_CALL getNumEventListeners(const EvtType& type) const override {
			std::scoped_lock lock(_mutex);

			auto itr = _listeners.find(type);
			return itr == _listeners.end() ? 0 : itr->second.numValidListeners;
		}

		virtual bool SRK_CALL hasEventListener(const EvtType& type, const IEventListener<EvtType>& listener) const override {
			std::scoped_lock lock(_mutex);

			if (auto itr = _listeners.find(type); itr != _listeners.end()) {
				if (auto& tl = itr->second; tl.numValidListeners) {
					if (auto& list = tl.listeners; tl.dispatching) {
						for (auto& f : list) {
							if (f.valid && f.rawListener == &listener) return true;
						}
					} else {
						for (auto& f : list) {
							if (f.rawListener == &listener) return true;
						}
					}
				}
			}

			return false;
		}

		using IEventDispatcher<EvtType>::removeEventListener;
		virtual bool SRK_CALL removeEventListener(const EvtType& type, const IEventListener<EvtType>& listener) override {
			std::scoped_lock lock(_mutex);

			if (auto itr = _listeners.find(type); itr != _listeners.end()) return itr->second.template remove<true>(listener);

			return false;
		}

		virtual uint32_t SRK_CALL removeEventListeners(const EvtType& type) override {
			std::scoped_lock lock(_mutex);

			auto itr = _listeners.find(type);
			return itr == _listeners.end() ? 0 : itr->second.removeAll();
		}

		virtual uint32_t SRK_CALL removeEventListeners() override {
			uint32_t n = 0;

			std::scoped_lock lock(_mutex);

			for (auto& itr : _listeners)  n += itr.second.removeAll();
			return n;
		}

		using IEventDispatcher<EvtType>::dispatchEvent;
		virtual void SRK_CALL dispatchEvent(void* target, const EvtType& type, void* data = nullptr) const override {
			std::scoped_lock lock(_mutex);

			if (auto itr = _listeners.find(type); itr != _listeners.end()) {
				auto& tl = itr->second;
				if (tl.numValidListeners) {
					++tl.dispatching;

					Event<EvtType> evt(target, type, data);
					auto& list = tl.listeners;
					for (auto& f : list) {
						if (f.valid) (*f.rawListener)(evt);
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
							--tl.numTotalListeners;
						}
					}
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
				dispatching(0),
				numValidListeners(1),
				numTotalListeners(1) {
				listeners.emplace_back(rawListener);
			}

			TypeListeners(TypeListeners&& other) noexcept :
				dispatching(other.dispatching),
				numValidListeners(other.numValidListeners),
				numTotalListeners(other.numTotalListeners),
				listeners(std::move(other.listeners)) {
			}

			uint32_t dispatching;
			uint32_t numValidListeners;
			uint32_t numTotalListeners;
			std::vector<Listener> listeners;

			template<bool Clean>
			bool SRK_CALL remove(const IEventListener<EvtType>& listener) {
				if (!numValidListeners) return false;

				if (dispatching) {
					for (auto& f : listeners) {
						if (f.rawListener == &listener) {
							if (f.valid) {
								f.valid = false;
								if constexpr (Clean) {
									Ref::unref(*f.rawListener);
									--numValidListeners;
								}
								
								return true;
							}
						}
					}
				} else {
					for (auto itr = listeners.begin(); itr != listeners.end(); ++itr) {
						if (&listener == (*itr).rawListener) {
							if constexpr (Clean) {
								Ref::unref(*(*itr).rawListener);
								--numValidListeners;
							}
							listeners.erase(itr);
							--numTotalListeners;

							return true;
						}
					}
				}

				return false;
			}

			uint32_t SRK_CALL removeAll() {
				auto n = numValidListeners;
				if (n) {
					if (dispatching) {
						for (auto& f : listeners) {
							if (f.valid) {
								f.valid = false;
								Ref::unref(*f.rawListener);
							}
						}
					} else {
						for (auto& f : listeners) Ref::unref(*f.rawListener);
						listeners.clear();
						numTotalListeners = 0;
					}

					numValidListeners = 0;
				}

				return n;
			}
		};


		mutable std::recursive_mutex _mutex;
		mutable std::unordered_map<EvtType, TypeListeners> _listeners;
	};
}