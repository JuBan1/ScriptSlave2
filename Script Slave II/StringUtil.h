#pragma once

#include <string>
#include <stdarg.h>  // for va_start, etc

std::string string_format(const std::string fmt_str, ...);

std::string va_string_format(const std::string fmt_str, va_list arg_list);
