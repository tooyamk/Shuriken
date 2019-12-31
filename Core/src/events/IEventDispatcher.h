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

		Event(void* target, const Event& e) :
			_type(e.getType()),
			_data(e.getData()),
			_target(target) {
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
	class AE_TEMPLATE_DLL IEventListener : public Ref {
	public:
		virtual void AE_CALL onEvent(Event<EvtType>& e) = 0;
	};


	template<typename EvtType, typename Class>
	class AE_TEMPLATE_DLL EventListener : public IEventListener<EvtType> {
	public:
		EventListener(Class* target, EvtMethod<EvtType, Class> method) :
			_target(method ? target : nullptr),
			_method(method) {
		}

		virtual void AE_CALL onEvent(Event<EvtType>& e) override {
			if (_target) (_target->*_method)(e);
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
	EventListener(EvtFn<EvtType>)->EventListener<EvtType, EvtFn<EvtType>>;


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
	EventListener(EvtFunc<EvtType>)->EventListener<EvtType, EvtFunc<EvtType>>;


	template<typename EvtType, typename Fn>
	class AE_TEMPLATE_DLL EventListener<EvtType, const Fn&> : public IEventListener<EvtType> {
	public:
		EventListener(const Recognitor<EvtType>& recognitor, const Fn& fn) :
			_fn(fn) {
		}

		virtual void AE_CALL onEvent(Event<EvtType>& e) override {
			_fn(e);
		}
	private:
		Fn _fn;
	};
	template<typename EvtType, typename Fn, typename = std::enable_if_t<std::is_invocable_r<void, Fn, Event<EvtType>>::value, Fn>>
	EventListener(Recognitor<EvtType>, Fn)->EventListener<EvtType, const Fn&>;


	template<typename EvtType>
	class AE_TEMPLATE_DLL IEventDispatcher : public Ref {
	public:
		inline bool AE_CALL addEventListener(const EvtType& type, IEventListener<EvtType>* listener) {
			return listener ? addEventListener(type, *listener) : false;
		}
		virtual bool AE_CALL addEventListener(const EvtType& type, IEventListener<EvtType>& listener) = 0;

		virtual uint32_t AE_CALL hasEventListener(const EvtType& type) const = 0;
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