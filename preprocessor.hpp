#ifndef __PY_TYPEHINT_PREPROCESSOR_H__
#define __PY_TYPEHINT_PREPROCESSOR_H__ 1

#include <fstream>       // ifstream, ofstream
#include <string>        // string
#include <cstdint>       // size_t, uint32_t
#include <unordered_set> // unordered_set<>

using std::uint32_t;

enum class PreprocessorFlags : uint32_t {
    no_flags          = 1 >> 1,
    overwrite_file    = 1 << 0,
    verbose           = 1 << 1,
    debug             = 1 << 2,
    continue_on_error = 1 << 3, /* Not recommended to use. */
    all_disabled      = 1 << 4
};

enum class ErrorCodes : uint32_t {
    no_errors                               = 1 >> 1,
    preprocessor_line_buffer_overflow       = 1 << 0,
    function_parse_error                    = 1 << 1,
    function_name_parse_error               = 1 << 2 | function_parse_error,
    function_argument_parse_error           = 1 << 3 | function_parse_error,
    function_argument_type_hint_parse_error = 1 << 4 | function_argument_parse_error,
    function_return_type_hint_parse_error   = 1 << 5 | function_parse_error,
    unexpected_eof                          = 1 << 6,
    too_much_colon_symbols                  = 1 << 7,
    too_much_closing_curly_brackets         = 1 << 8,
    too_much_closing_square_brackets        = 1 << 9,
    too_much_closing_round_brackets         = 1 << 10,
    too_few_closing_square_brackets         = 1 << 11,
    too_few_closing_round_brackets          = 1 << 12,
    string_not_closed_error                 = 1 << 13,
    src_file_open_error                     = 1 << 14,
    src_file_io_error                       = 1 << 15,
    tmp_file_open_error                     = 1 << 16,
    tmp_file_delete_error                   = 1 << 17,
    overwrite_error                         = 1 << 18,
    single_file_process_error               = 1 << 19 /* Can only occur while processing many files at once. */
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

constexpr PreprocessorFlags default_flags = PreprocessorFlags::verbose;

ErrorCodes process_file(
    const std::string &input_filename,
    const std::unordered_set<std::string> &ignored_functions,
    const PreprocessorFlags preprocessor_flags = default_flags
);

ErrorCodes process_files(
    const std::unordered_set<std::string> &filenames,
    const std::unordered_set<std::string> &ignored_functions,
    const PreprocessorFlags preprocessor_flags = default_flags
);

#endif
