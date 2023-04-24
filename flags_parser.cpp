#include <cstdint>  // uint32_t
#include <cstring>  // strcmp
#include <iostream> // std::clog

#include "flags_parser.hpp"

using std::uint32_t;

namespace preprocessor_tools {

static constexpr PreprocessorFlags parse_flag(const char *arg) {
    switch (arg[0]) {
    case 'd':
        if (strcmp(++arg, "ebug") == 0) {
            // -debug turns on both debug and verbose modes
            return PreprocessorFlags::debug | PreprocessorFlags::verbose;
        }
        break;
    case 'o':
        if (strcmp(++arg, "verwrite") == 0) {
            return PreprocessorFlags::overwrite_file;
        }
        break;
    case 'v':
        if (strcmp(++arg, "erbose") == 0) {
            return PreprocessorFlags::verbose;
        }
        break;
    case 'c':
        if (strcmp(++arg, "ontinue_on_error") == 0) {
            std::clog << "War";
            return PreprocessorFlags::continue_on_error;
        }
        break;
    case 'a':
        if (strcmp(++arg, "ll_disabled") == 0) {
            return PreprocessorFlags::all_flags_disabled;
        }
        break;
    }

    return PreprocessorFlags::no_flags;
}

PreprocessorFlags parse_flags(const size_t argc, const char **const argv) {
    PreprocessorFlags flags = PreprocessorFlags::no_flags;

    for (size_t i = 0; i < argc; ++i) {
        const char *arg = argv[i];
        if (arg[0] != '-') {
            continue;
        }

        flags |= parse_flag(++arg);
    }

    return flags;
}

std::string from_error(const ErrorCodes error_codes) {
    std::string error_report("Errors:\n");
    size_t reserve = 0;
    for (uint32_t i = 0; i <= 20; ++i)
        if (error_codes & (1u << i))
            reserve += 32;
    error_report.reserve(reserve);

    if (error_codes & ErrorCodes::preprocessor_line_buffer_overflow) {
        error_report += "Line buffer overflew (too long line occured)\n";
    }

    if (error_codes & ErrorCodes::function_parse_error) {
        error_report += "Could not parse Python function\n";
    }

    if (error_codes & ErrorCodes::function_name_parse_error) {
        error_report += "Could not parse Python function name\n";
    }

    if (error_codes & ErrorCodes::function_argument_parse_error) {
        error_report += "Could not parse Python function argument name\n";
    }

    if (error_codes & ErrorCodes::function_argument_type_hint_parse_error) {
        error_report += "Could not parse Python function argument typehint\n";
    }

    if (error_codes & ErrorCodes::function_return_type_hint_parse_error) {
        error_report += "Could not parse Python function return type typehint\n";
    }

    if (error_codes & ErrorCodes::unexpected_eof) {
        error_report += "Unexpected end of stream (end of file)\n";
    }

    if (error_codes & ErrorCodes::too_much_colon_symbols) {
        error_report += "Too much colon symbols ':'\n";
    }

    if (error_codes & ErrorCodes::too_much_closing_curly_brackets) {
        error_report += "Too much closing curly brackets '}'\n";
    }

    if (error_codes & ErrorCodes::too_much_closing_square_brackets) {
        error_report += "Too much closing square brackets ']'\n";
    }

    if (error_codes & ErrorCodes::too_much_closing_round_brackets) {
        error_report += "Too much closing round brackets ')'\n";
    }

    if (error_codes & ErrorCodes::too_few_closing_square_brackets) {
        error_report += "Too few closing square brackets ']'\n";
    }

    if (error_codes & ErrorCodes::too_few_closing_round_brackets) {
        error_report += "Too few closing round brackets ')'\n";
    }

    if (error_codes & ErrorCodes::string_not_closed_error) {
        error_report += "Python string was not closed\n";
    }

    if (error_codes & ErrorCodes::src_file_open_error) {
        error_report += "An error occured while opening source '.py' file\n";
    }

    if (error_codes & ErrorCodes::src_file_io_error) {
        error_report += "An error occured while reading source '.py' file\n";
    }

    if (error_codes & ErrorCodes::tmp_file_open_error) {
        error_report += "An error occured while opening temporary file\n";
    }

    if (error_codes & ErrorCodes::tmp_file_delete_error) {
        error_report += "An error occured while deleting temporary file\n";
    }

    if (error_codes & ErrorCodes::overwrite_error) {
        error_report += "An error occured while overwriting source '.py' file\n";
    }

    if (error_codes & ErrorCodes::single_file_process_error) {
        error_report += "An error occured while processing single file (multifile mode is turned on)\n";
    }

    if (error_codes & ErrorCodes::memory_allocating_error) {
        error_report += "An error occured while allocationg memory for the preprocessor's buffers\n";
    }

    return error_report;
}

} // namespace preprocessor_tools
