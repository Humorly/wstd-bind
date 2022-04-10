#pragma once

#include <tuple>
#include <functional>

// #define placeholder type
#if defined(_MSC_VER)
#define __placeholder_type std::_Ph

#elif defined(__GNUC__)
#define __placeholder_type std::_Placeholder
#endif	

namespace wstd
{
	template <typename __place_type>
	struct params_wrapper
	{
		template <typename ... args>
		static auto get_value(__place_type val, args ... params)
		{
			return val;
		}
	};

	// placeholders special
#define place_holder_wrapper(__index) 		template <>																	\
											struct params_wrapper<__placeholder_type<__index> &>										\
											{																			\
												template <typename ... args>											\
												static auto get_value(__placeholder_type<__index> &, args ... params)				\
												{																		\
													std::tuple<args...> tu_(params ...);							\
													return std::get<__index - 1>(tu_);									\
												}																		\
											};

	// placeholder 1 ~ 20
	place_holder_wrapper(1);
	place_holder_wrapper(2);
	place_holder_wrapper(3);
	place_holder_wrapper(4);
	place_holder_wrapper(5);
	place_holder_wrapper(6);
	place_holder_wrapper(7);
	place_holder_wrapper(8);
	place_holder_wrapper(9);
	place_holder_wrapper(10);
	place_holder_wrapper(11);
	place_holder_wrapper(12);
	place_holder_wrapper(13);
	place_holder_wrapper(14);
	place_holder_wrapper(15);
	place_holder_wrapper(16);
	place_holder_wrapper(17);
	place_holder_wrapper(18);
	place_holder_wrapper(19);
	place_holder_wrapper(20);

	// base binder caller
	template <typename ret, typename ... args>
	class binder_impl_wrapper
	{
	public:
		binder_impl_wrapper() {}
		virtual ~binder_impl_wrapper() {}

		virtual ret call(args ... parmas) { return ret(); };
	};

	// binder caller
	template <typename call_type, typename ret, typename ... args>
	class binder_impl : public binder_impl_wrapper<ret, args ...>
	{
	public:
		binder_impl(call_type cal) : caller_(cal)
		{
		}
		virtual ~binder_impl() {}
		virtual ret call(args ... parmas) 
		{
			return caller_.call(parmas ...);
		};

	protected:
		call_type caller_;
	};

	// function wrapper with none-member funciton
	template <typename Fx, typename ... args>
	class binder
	{
	public:
		binder(Fx f, args ... params) : tuple_(params ...)
		{
			function_ = f;
		}
		virtual ~binder() {}

		// call
		template <typename ... __call_args>
		auto call(__call_args ... params)
		{
			return func(args_seq_, params ...);
		};

		template <std::size_t ... I, typename ... __func_args>
		auto func(std::index_sequence<I ...>, __func_args ... params)
		{
			return function_(params_wrapper< decltype(std::get<I>(tuple_)) >::template get_value(std::get<I>(tuple_), params ...) ...);
		}
	protected:
		Fx function_;
		std::index_sequence_for<args...> args_seq_{};
		std::tuple<args...> tuple_;
	};

	// function wrapper with member funciton
	template <typename Fx, typename T, typename ... args>
	class binder_t
	{
	public:
		binder_t(Fx f, T ob, args ... params) : tuple_(params ...)
		{
			function_ = f;
			ob_ = ob;
		}
		virtual ~binder_t() {}

		// call
		template <typename ... __call_args>
		auto call(__call_args ... params)
		{
			return func(args_seq_, params ...);
		};

		template <std::size_t... I, typename ... __func_args>
		auto func(std::index_sequence<I...>, __func_args ... params)
		{
			return (ob_->*function_)(params_wrapper< decltype(std::get<I>(tuple_)) >::template get_value(std::get<I>(tuple_), params ...)...);
		}
	protected:
		Fx function_;
		T ob_;
		std::index_sequence_for<args ...> args_seq_{};
		std::tuple<args ...> tuple_;
	};

	// binder selector
	template <bool __type>
	struct make_binder
	{
		template<typename Fx, typename ... args>
		static binder<Fx, args...> bind(Fx f, args... a)
		{
			binder<Fx, args...> binder_(f, a...);
			return binder_;
		}
	};

	template <>
	struct make_binder<true>
	{
		template<typename Fx, typename T, typename ... args>
		static binder_t<Fx, T, args...> bind(Fx f, T ob, args... a)
		{
			binder_t<Fx, T, args...> binder_(f, ob, a...);
			return binder_;
		}
	};

	template<typename Fx, typename T, typename ... args>
	auto bind(Fx f, T ob, args ... params)
	{
		return make_binder<std::is_member_function_pointer<decltype(f)>::value>::template bind(f, ob, params ...);
	}

	template <typename ret, typename ... args>
	class func_impl {};

	template <typename ret, typename ... args>
	class func_impl<ret (args...)>
	{
	public:

		func_impl() {}
		virtual ~func_impl() 
		{
			if (nullptr != impl_)
				delete impl_;
		}

	protected:
		template <typename binder>
		void reset(binder call_impl)
		{
			impl_ = new binder_impl<binder, ret, args...>(call_impl);
		}

		ret call(args ... a)
		{
			return impl_->call(a...);
		}

		binder_impl_wrapper<ret, args ...> * impl_ = nullptr;
	};

	template <typename Fx>
	class function : public func_impl<Fx>
	{
	public:
		template<typename __binder_type>
		function(__binder_type t)
		{
			this->reset(t);
		}
		virtual ~function() {}

		template<typename ... args>
		auto operator ()(args ... params)
		{
			return this->call(params ...);
		}
	};
};
