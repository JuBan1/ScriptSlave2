#pragma once

#include <string>

class TypeInfo{
public:
	std::string name;
	bool isArray;
	size_t size;

	std::string GetSimpleTypeName() const { //TODO: I'd rather have a direct link to the simple type
		if (isArray) 
			return name.substr(0, name.length()-2);
		else
			return name;
	}

	TypeInfo(std::string name, size_t size, bool isArray) : size(size), isArray(isArray) {
		if (isArray){
			this->name = name + "[]";
		}
		else{
			this->name = name;
		}
	}
};