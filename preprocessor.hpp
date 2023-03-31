#ifndef __PY_TYPEHINT_PREPROCESSOR_H__
#define __PY_TYPEHINT_PREPROCESSOR_H__ 1

#include <fstream>
#include <string>
#include <cstdint>

using std::uint32_t;

enum class PreprocessorFlags : uint32_t {
    no_flags       = 0b0000,
    overwrite_file = 0b0001,
    verbose        = 0b0010,
    debug          = 0b0100,
    stop_on_error  = 0b1000
};

enum class ErrorCodes : uint32_t {
    no_errors                         = 0b000000000000,
    preprocessor_line_buffer_overflow = 0b000000000001,
    function_parse_error              = 0b000000000010,
    function_argument_type_hint_error = 0b000000000100,
    unexpected_eof                    = 0b000000001000,
    too_much_colon_symbols            = 0b000000010000,
    too_much_closing_curly_brackets   = 0b000000100000,
    too_much_closing_square_brackets  = 0b000001000000,
    src_file_open_error               = 0b000010000000,
    src_file_io_error                 = 0b000100000000,
    tmp_file_open_error               = 0b001000000000,
    tmp_file_delete_error             = 0b010000000000,
    overwrite_error                   = 0b100000000000
};

inline constexpr ErrorCodes
operator&(ErrorCodes __a, ErrorCodes __b)
{ return ErrorCodes(static_cast<uint32_t>(__a) & static_cast<uint32_t>(__b)); }

inline constexpr ErrorCodes
operator|(ErrorCodes __a, ErrorCodes __b)
{ return ErrorCodes(static_cast<uint32_t>(__a) | static_cast<uint32_t>(__b)); }

inline constexpr ErrorCodes
operator^(ErrorCodes __a, ErrorCodes __b)
{ return ErrorCodes(static_cast<uint32_t>(__a) ^ static_cast<uint32_t>(__b)); }

inline constexpr ErrorCodes
operator~(ErrorCodes __a)
{ return ErrorCodes(~static_cast<uint32_t>(__a)); }

inline constexpr ErrorCodes&
operator|=(ErrorCodes& __a, ErrorCodes __b)
{ return __a = __a | __b; }

inline constexpr ErrorCodes&
operator&=(ErrorCodes& __a, ErrorCodes __b)
{ return __a = __a & __b; }

inline constexpr ErrorCodes&
operator^=(ErrorCodes& __a, ErrorCodes __b)
{ return __a = __a ^ __b; }

inline constexpr PreprocessorFlags
operator|(PreprocessorFlags __a, PreprocessorFlags __b)
{ return PreprocessorFlags(static_cast<uint32_t>(__a) | static_cast<uint32_t>(__b)); }

inline constexpr PreprocessorFlags
operator&(PreprocessorFlags __a, PreprocessorFlags __b)
{ return PreprocessorFlags(static_cast<uint32_t>(__a) & static_cast<uint32_t>(__b)); }

inline constexpr PreprocessorFlags
operator^(PreprocessorFlags __a, PreprocessorFlags __b)
{ return PreprocessorFlags(static_cast<uint32_t>(__a) ^ static_cast<uint32_t>(__b)); }

inline constexpr PreprocessorFlags
operator~(PreprocessorFlags __a)
{ return PreprocessorFlags(~static_cast<uint32_t>(__a)); }

inline constexpr PreprocessorFlags&
operator|=(PreprocessorFlags& __a, PreprocessorFlags __b)
{ return __a = __a | __b; }

inline constexpr PreprocessorFlags&
operator&=(PreprocessorFlags& __a, PreprocessorFlags __b)
{ return __a = __a & __b; }

inline constexpr PreprocessorFlags&
operator^=(PreprocessorFlags& __a, PreprocessorFlags __b)
{ return __a = __a ^ __b; }

constexpr PreprocessorFlags default_flags = PreprocessorFlags::verbose | PreprocessorFlags::stop_on_error;

ErrorCodes process_file(const std::string &input_filename, const PreprocessorFlags preprocessor_flags = default_flags);

#endif