#ifndef _PY_TYPEHINT_PREPROCESSOR_FLAGS_PARSER_H_
#define _PY_TYPEHINT_PREPROCESSOR_FLAGS_PARSER_H_ 1

#include <cstdint> // size_t
#include <string>  // std::string

#include "preprocessor.hpp"

using std::size_t;

PreprocessorFlags parse_flags(const size_t argc, const char **const argv);

std::string from_error(ErrorCodes error_codes);

#endif
