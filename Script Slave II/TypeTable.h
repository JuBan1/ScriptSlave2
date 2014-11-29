#pragma once

#include <string>
#include <map>
#include "TypeInfo.h"

class TypeTable{
	std::map<std::string, TypeInfo> types;

public:
	void Add(TypeInfo ti){
		types.emplace(std::make_pair(ti.name, ti));
	}

	TypeInfo const* Get(const std::string& typeName) const {
		auto it = types.find(typeName);
		if (it == types.end())
			return nullptr;
		else
			return &it->second;
	}
};