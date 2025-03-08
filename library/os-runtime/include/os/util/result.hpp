#pragma once

#include <optional>
#include <variant>

namespace os
{
	struct Success_t
	{
	};

	static constexpr Success_t success = {};

	template <typename T, typename E>
		requires(!std::is_same_v<T, E>)
	class Result
	{
		std::variant<T, E> v;

	  public:

		Result(T value) :
			v(std::move(value))
		{
		}

		Result(E error) :
			v(std::move(error))
		{
		}

		T& value() & { return std::get<T>(v); }
		const T& value() const& { return std::get<T>(v); }
		T&& value() && { return std::get<T>(std::move(v)); }
		const T&& value() const&& { return std::get<T>(std::move(v)); }

		E& error() & { return std::get<E>(v); }
		const E& error() const& { return std::get<E>(v); }
		E&& error() && { return std::get<E>(std::move(v)); }
		const E&& error() const&& { return std::get<E>(std::move(v)); }

		operator bool() const { return std::holds_alternative<T>(v); }
	};

	template <typename E>
	class Result<void, E>
	{
		std::optional<E> e;

	  public:

		Result(Success_t) :
			e(std::nullopt)
		{
		}

		Result(E error) :
			e(std::move(error))
		{
		}

		E& error() & { return std::get<E>(e); }
		const E& error() const& { return std::get<E>(e); }
		E&& error() && { return std::get<E>(std::move(e)); }
		const E&& error() const&& { return std::get<E>(std::move(e)); }

		operator bool() const { return !e.has_value(); }
	};

	template <typename E>
	using Err_result = Result<void, E>;
}