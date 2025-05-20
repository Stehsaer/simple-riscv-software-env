#pragma once

#include <map>
#include <string>
#include <string_view>

namespace filesystem
{
	struct Sv_comp
	{
		using is_transparent = void;  // This allows heterogeneous lookup

		bool operator()(const std::string& lhs, const std::string& rhs) const { return lhs < rhs; }

		bool operator()(const std::string& lhs, std::string_view rhs) const { return lhs < rhs; }

		bool operator()(std::string_view lhs, const std::string& rhs) const { return lhs < rhs; }
	};

	template <typename T>
	using String_view_map = std::map<std::string, T, Sv_comp>;
}