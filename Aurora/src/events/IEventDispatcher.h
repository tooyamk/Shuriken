#pragma once

#include "base/Ref.h"
#include <functional>

namespace aurora::event {
	template<typename T>
	class AE_TEMPLATE_DLL Event {
	public:
		Event(T name, void* data = nullptr) :
			_name(name),
			_data(data),
			_target(nullptr) {
		}

		Event(void* target, T name, void* data = nullptr) :
			_name(name),
			_data(data),
			_target(target) {
		}

		Event(void* target, const Event<T>&e) :
			_name(e.getName()),
			_data(e.getData()),
			_target(target) {
		}

		inline void* getTarget() const {
			return _target;
		}

		inline const T& getName() const {
			return _name;
		}

		inline void* getData() const {
			return _data;
		}

	protected:
		T _name;
		void* _data;
		void* _target;
	};


	template<typename T>
	class AE_TEMPLATE_DLL IEventDispatcher : public Ref {
	public:
		typedef std::function<void(Event<T>&)> Function;
		typedef void(*EVENT_LISTENER)(Event<T>&);

		template<typename C>
		using EVENT_CLASS_LISTENER = void(C::*)(Event<T>&);


		class FunctionListener {
		public:
			FunctionListener(const Function& listener) :
				listener(listener) {
			}

			void operator()(Event<T>& e) {
				return listener(e);
			}

		private:
			Function listener;
		};

		virtual void AE_CALL dispatchEvent(const Event<T>& e) = 0;
		virtual void AE_CALL dispatchEvent(void* target, const Event<T>& e) = 0;
		virtual void AE_CALL dispatchEvent(const T& name, void* data = nullptr) = 0;
		virtual void AE_CALL dispatchEvent(void* target, const T& name, void* data = nullptr) = 0;
	};
}