#pragma once

#include "base/Ref.h"
#include <unordered_map>
#include <functional>

AE_EVENT_NS_BEGIN

template<typename T>
class Event {
public:
};


template<typename T>
using AE_EVENT_CALLBACK = std::function<void(Event<T>)>;


template<typename T>
class EventListener {
public:
	EventListener(const AE_EVENT_CALLBACK<T>& callback) :
		_callback(callback) {
	}

protected:
	AE_EVENT_CALLBACK<T> _callback;
};


template<typename T>
class EventDispatcher : public Ref {
public:
	EventDispatcher(void* target) :
		_target(target) {
	}

	virtual ~EventDispatcher() {

	}

	void AE_CALL addEventListener(const T& name, const AE_EVENT_CALLBACK<T>& callback) {
		auto el = new EventListener<T>(callback);

		auto itr = _listeners.find(name);
		auto& vec = itr == _listeners.end() ? _listeners.emplace(1, 0).first->second : itr->second;
		vec.emplace_back(el);
	}

protected:
	void* _target;
	std::unordered_map<T, std::vector<EventListener<T>*>> _listeners;
};

AE_EVENT_NS_END