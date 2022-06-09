#pragma once
#include <algorithm>
#include <utility>
#include <vector>

namespace mirage::signal
{
	template<typename T>
	class VectorCollector
	{
	public:
		std::vector<T> contents;

		bool operator()(T v)
		{
			contents.emplace_back(std::move(v));
			return true;
		}
	};

	template<typename T, typename... Args>
	bool isAny(auto&& signal, T value, Args&&... args)
	{
		bool result = false;
		signal.collect([&result, &value](auto value1) -> void 
		{
			if(value1 == value)
				result = true;
		}, args...);

		return result;
	}

}
