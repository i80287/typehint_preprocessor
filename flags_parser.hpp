#ifndef _PY_TYPEHINT_PREPROCESSOR_FLAGS_PARSER_H_
#define _PY_TYPEHINT_PREPROCESSOR_FLAGS_PARSER_H_ 1

#include <cstddef> // size_t
#include <string>  // std::string

#include <preprocessor.hpp>

namespace preprocessor_tools {

PreprocessorFlags parse_flags(const size_t argc, const char **const argv) noexcept;

std::string from_error(const ErrorCodes error_codes);

} // namespace preprocessor_tools

#endif
