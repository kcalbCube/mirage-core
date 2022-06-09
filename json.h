#pragma once

#include <boost/json.hpp>
#include <string>
#include <fstream>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

namespace mirage::utils::json
{
	inline boost::property_tree::ptree parseFile(std::string_view filename)
	{	
		boost::property_tree::ptree tree;
		boost::property_tree::read_json(filename.data(), tree);

		return tree;
	}

	inline boost::property_tree::ptree parseString(const std::string& stringData)
	{
		std::stringstream ss{stringData};

		boost::property_tree::ptree tree;
		boost::property_tree::read_json(ss, tree);

		return tree;
	}

	template<typename T>
	inline T readObject(boost::property_tree::ptree ptree)
	{

	}
}
