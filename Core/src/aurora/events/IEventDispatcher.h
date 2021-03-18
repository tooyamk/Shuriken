#pragma once

#include "aurora/Intrusive.h"
#include <functional>

namespace aurora::events {
	template<typename EvtType>
	class Event {
	public:
		Event(void* target, const EvtType& type, void* data = nullptr) :
			_type(type),
			_data(data),
			_target(target) {
		}

		Event(const EvtType& type, void* data = nullptr) : Event(nullptr, type, data) {
		}

		Event(void* target, const Event& e) : Event(target, e._type, e._data) {
		}

		template<typename T = void>
		inline T* AE_CALL getTarget() const {
			return (T*)_target;
		}

		inline const EvtType& AE_CALL getType() const {
			return _type;
		}

		template<typename T = void>
		inline T* AE_CALL getData() const {
			return (T*)_data;
		}

	protected:
		EvtType _type;
		void* _data;
		void* _target;
	};


	template<typename EvtType>
	using EvtFn = void(*)(Event<EvtType>&);

	template<typename EvtType, typename Class>
	using EvtMethod = void(Class::*)(Event<EvtType>&);

	template<typename EvtType>
	using EvtFunc = std::function<void(Event<EvtType>&)>;


	template<typename EvtType>
	class IEventListener : public Ref {
	public:
		virtual void AE_CALL operator()(Event<EvtType>& e) const = 0;
	};


	template<typename EvtType, typename...>
	class EventListener : public IEventListener<EvtType> {
	};


	template<typename EvtType, typename Class>
	class EventListener<EvtType, EvtMethod<EvtType, Class>> : public IEventListener<EvtType> {
	public:
		EventListener(EvtMethod<EvtType, Class> method, Class* target) :
			_method(target ? method : nullptr),
			_target(target) {
		}

		virtual void AE_CALL operator()(Event<EvtType>& e) const override {
			if (_target) (_target->*_method)(e);
		}
	private:
		EvtMethod<EvtType, Class> _method;
		Class* _target;
	};
	template<typename EvtType, typename Class>
	requires MemberFunctionPointer<EvtMethod<EvtType, Class>>
	EventListener(EvtMethod<EvtType, Class>, Class*)->EventListener<EvtType, EvtMethod<EvtType, Class>>;


	template<typename EvtType>
	class EventListener<EvtType, EvtFn<EvtType>> : public IEventListener<EvtType> {
	public:
		EventListener(EvtFn<EvtType>&& fn) :
			_fn(std::forward<EvtFn<EvtType>>(fn)) {
		}

		virtual void AE_CALL operator()(Event<EvtType>& e) const override {
			if (_fn) _fn(e);
		}
	private:
		EvtFn<EvtType> _fn;
	};
	template<typename EvtType>
	EventListener(EvtFn<EvtType>)->EventListener<EvtType, EvtFn<EvtType>>;

	template<typename EvtType>
	class EventListener<EvtType, EvtFunc<EvtType>> : public IEventListener<EvtType> {
	public:
		EventListener(EvtFunc<EvtType>&& fn) :
			_fn(std::forward<EvtFunc<EvtType>>(fn)) {
		}

		virtual void AE_CALL operator()(Event<EvtType>& e) const override {
			if (_fn) _fn(e);
		}
	private:
		EvtFunc<EvtType> _fn;
	};
	template<typename EvtType>
	EventListener(EvtFunc<EvtType>)->EventListener<EvtType, EvtFunc<EvtType>>;


	template<typename EvtType, typename Fn>
	class EventListener<EvtType, Fn> : public IEventListener<EvtType> {
	public:
		EventListener(Fn&& fn) :
			_fn(fn) {
		}

		virtual void AE_CALL operator()(Event<EvtType>& e) const override {
			_fn(e);
		}
	private:
		Fn _fn;
	};


	template<typename EvtType, typename Class>
	inline IntrusivePtr<EventListener<EvtType, EvtMethod<EvtType, Class>>> AE_CALL createEventListener(EvtMethod<EvtType, Class> fn, Class* target) {
		return new EventListener<EvtType, EvtMethod<EvtType, Class>>(fn, target);
	}

	template<typename EvtType>
	inline IntrusivePtr<EventListener<EvtType, EvtFn<EvtType>>> AE_CALL createEventListener(EvtFn<EvtType>&& fn) {
		return new EventListener<EvtType, EvtFn<EvtType>>(std::forward<EvtFn<EvtType>>(fn));
	}

	template<typename EvtType>
	inline IntrusivePtr<EventListener<EvtType, EvtFunc<EvtType>>> AE_CALL createEventListener(EvtFunc<EvtType>&& fn) {
		return new EventListener<EvtType, EvtFunc<EvtType>>(std::forward<EvtFunc<EvtType>>(fn));
	}
	
	template<typename EvtType, typename Fn>
	requires std::invocable<Fn, Event<EvtType>&>
	inline IntrusivePtr<EventListener<EvtType, Fn>> AE_CALL createEventListener(Fn&& fn) {
		return new EventListener<EvtType, Fn>(std::forward<Fn>(fn));
	}


	template<typename EvtType>
	class IEventDispatcher : public Ref {
	public:
		inline bool AE_CALL addEventListener(const EvtType& type, IEventListener<EvtType>* listener) {
			return listener ? addEventListener(type, *listener) : false;
		}
		virtual bool AE_CALL addEventListener(const EvtType& type, IEventListener<EvtType>& listener) = 0;

		virtual uint32_t AE_CALL getNumEventListeners() const = 0;
		virtual uint32_t AE_CALL getNumEventListeners(const EvtType& type) const = 0;
		virtual bool AE_CALL hasEventListener(const EvtType& type, const IEventListener<EvtType>& listener) const = 0;

		inline bool AE_CALL removeEventListener(const EvtType& type, const IEventListener<EvtType>* listener) {
			return listener ? removeEventListener(type, *listener) : false;
		}
		virtual bool AE_CALL removeEventListener(const EvtType& type, const IEventListener<EvtType>& listener) = 0;

		virtual uint32_t AE_CALL removeEventListeners(const EvtType& type) = 0;
		virtual uint32_t AE_CALL removeEventListeners() = 0;

		inline void AE_CALL dispatchEvent(const Event<EvtType>& e) const {
			dispatchEvent(e.getTarget(), e.getType(), e.getData());
		}
		inline void AE_CALL dispatchEvent(void* target, const Event<EvtType>& e) const {
			dispatchEvent(target, e.getType(), e.getData());
		}
		virtual void AE_CALL dispatchEvent(void* target, const EvtType& type, void* data = nullptr) const = 0;
	};
}