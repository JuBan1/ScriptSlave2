#ifndef OPTIONAL_H
#define OPTIONAL_H

#include <stdexcept>

template <class T>
class Optional{

public:

	T Get() const {
		if( m_valid )
			return m_t;
		throw std::exception();
	}

	bool isValid() const {
		return m_valid;
	}

	Optional<T>()
		: m_valid( false )
	{}

	Optional<T>(T t)
		: m_valid( true ), m_t( t )
	{}

private:
	bool m_valid;
	T m_t;
};

#endif