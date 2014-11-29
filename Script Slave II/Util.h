#ifndef _MSC_VER //make_unique already in MSVC stdlib. Include only for GCC & others.
#ifndef UTIL_H
#define UTIL_H

#include <memory>

namespace std{

template<typename T, typename... Args>
std::unique_ptr<T> make_unique( Args && ... args )
{
	return std::unique_ptr<T>(new T( std::forward<Args>(args)... ));
}

}

#endif
#endif