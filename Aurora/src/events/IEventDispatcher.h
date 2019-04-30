#pragma once

#include "base/Ref.h"
#include <functional>

namespace aurora::events {
	template<typename EvtType>
	class AE_TEMPLATE_DLL Event {
	public:
		Event(const EvtType& type, void* data = nullptr) :
			_type(type),
			_data(data),
			_target(nullptr) {
		}

		Event(void* target, const EvtType& type, void* data = nullptr) :
			_type(type),
			_data(data),
			_target(target) {
		}

		Event(void* target, const Event<EvtType>& e) :
			_type(e.getType()),
			_data(e.getData()),
			_target(target) {
		}

		inline void* AE_CALL getTarget() const {
			return _target;
		}

		inline const EvtType& AE_CALL getType() const {
			return _type;
		}

		inline void* AE_CALL getData() const {
			return _data;
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
	class AE_TEMPLATE_DLL IEventListener : public Ref {
	public:
		virtual void AE_CALL onEvent(Event<EvtType>& e) = 0;
	};


	template<typename EvtType, typename Class>
	class AE_TEMPLATE_DLL EventListener : public IEventListener<EvtType> {
	public:
		EventListener(Class* target, EvtMethod<EvtType, Class> method) :
			_target(target),
			_method(method) {
		}

		virtual void AE_CALL onEvent(Event<EvtType>& e) override {
			if (_target && _method) (_target->*_method)(e);
		}
	private:
		Class* _target;
		EvtMethod<EvtType, Class> _method;
	};


	template<typename EvtType>
	class AE_TEMPLATE_DLL EventListener<EvtType, EvtFn<EvtType>> : public IEventListener<EvtType> {
	public:
		EventListener(EvtFn<EvtType> fn) :
			_fn(fn) {
		}

		virtual void AE_CALL onEvent(Event<EvtType>& e) override {
			if (_fn) _fn(e);
		}
	private:
		EvtFn<EvtType> _fn;
	};
	template<typename EvtType>
	using EventListenerFn = EventListener<EvtType, EvtFn<EvtType>>;


	template<typename EvtType>
	class AE_TEMPLATE_DLL EventListener<EvtType, EvtFunc<EvtType>> : public IEventListener<EvtType> {
	public:
		EventListener(const EvtFunc<EvtType>& fn) :
			_fn(fn) {
		}

		virtual void AE_CALL onEvent(Event<EvtType>& e) override {
			if (_fn) _fn(e);
		}
	private:
		EvtFunc<EvtType> _fn;
	};
	template<typename EvtType>
	using EventListenerFunc = EventListener<EvtType, EvtFunc<EvtType>>;


	template<typename EvtType>
	class AE_TEMPLATE_DLL IEventDispatcher : public Ref {
	public:
		virtual bool AE_CALL addEventListener(const EvtType& type, IEventListener<EvtType>& listener, bool ref) = 0;
		virtual ui32 AE_CALL hasEventListener(const EvtType& type) const = 0;
		virtual bool AE_CALL hasEventListener(const EvtType& type, const IEventListener<EvtType>& listener) const = 0;
		virtual bool AE_CALL removeEventListener(const EvtType& type, const IEventListener<EvtType>& listener) = 0;
		virtual ui32 AE_CALL removeEventListeners(const EvtType& type) = 0;
		virtual ui32 AE_CALL removeEventListeners() = 0;

		virtual void AE_CALL dispatchEvent(const Event<EvtType>& e) const = 0;
		virtual void AE_CALL dispatchEvent(void* target, const Event<EvtType>& e) const = 0;
		virtual void AE_CALL dispatchEvent(void* target, const EvtType& type, void* data = nullptr) const = 0;
	};
}