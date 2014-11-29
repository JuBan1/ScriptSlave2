#pragma once

#include <string>

class TypeInfo{
public:
	std::string name;
	size_t size;

	TypeInfo(std::string name, size_t size) : name(name), size(size){}
};