#pragma once

class FilePosition{
public:
	unsigned short line;
	unsigned short pos;

	bool operator<(const FilePosition& other) const {
		return (line==other.line && pos < other.pos) || line < other.line;
	}

	bool operator>(const FilePosition& other) const {
		return (line == other.line && pos > other.pos) || line > other.line;
	}

	std::string ToString() const{
		return std::to_string(line) + ":" + std::to_string(pos);
	}

	FilePosition(unsigned short line, unsigned short pos)
		: line(line), pos(pos)
	{}

	FilePosition()
		: line(0), pos(0)
	{}
};