#include <fstream>       // ifstream, ofstream
#include <string>        // string
#include <cstring>       // memmove
#include <cstdint>       // size_t, uint32_t
#include <sys/types.h>   // ssize_t
#include <cstdarg>       // __VA_ARGS__
#include <cstdio>        // fprintf
#include <iostream>      // cout, cerr
#include <vector>        // vector<>
#include <type_traits>   // is_same<>
#include <unordered_set> // unordered_set<>
#include <filesystem>    // std::filesystem

#include "preprocessor.hpp"

using std::size_t;
using std::uint32_t;

/* must be power of two. */
static constexpr size_t MAX_BUFF_SIZE = 2048;
static_assert((MAX_BUFF_SIZE & (MAX_BUFF_SIZE - 1)) == 0);

#define Check_BuffLen(buff_length) \
    Check_BuffLen_Reserve(buff_length, 0)

#define Check_BuffLen_Reserve(buff_length, additional_reserve)\
    static_assert(std::is_same<decltype(buff_length), size_t>::value); \
    if ((buff_length + (size_t)additional_reserve) & (~(MAX_BUFF_SIZE - 1))) {\
        if ((preprocessor_flags & PreprocessorFlags::verbose) != PreprocessorFlags::no_flags) {\
            std::clog << "Max buffer size is reached at line " << lines_count << '\n';\
        }\
        return current_state |= ErrorCodes::preprocessor_line_buffer_overflow;\
    }

#define Assert(expression, message, error_code) \
    static_assert(std::is_same<decltype((message)[0]), const char&>::value);\
    if (!(expression)) {\
        if (is_verbose_mode) {\
            std::clog << "Assertion error. Error message:\n" << message << '\n';\
        }\
        current_state |= error_code;\
        if (is_continue_on_error) {\
            return current_state;\
        }\
    }

#define AssertWithArgs(expression, error_code, ...) \
    if (!(expression)) {\
        if (is_verbose_mode) {\
            fprintf(stderr, __VA_ARGS__);\
        }\
        current_state |= error_code;\
        if (is_continue_on_error) {\
            return current_state;\
        }\
    }\

#define check_newline_char if (curr_char == '\n' || curr_char == '\r') { ++late_line_increase_counter; }

static inline bool count_symbols(const char *buf, const size_t length, std::vector<size_t> *symbols_indexes, size_t &equal_operator_index) {
    bool contains_lambda = false;
    bool is_string_opened = false;
    bool is_comment_opened = false;
    bool is_long_string_opened = false;
    char string_opening_char = '\0'; // will be either '\'' or '\"'
    int opened_curly_brackets = 0;
    int opened_square_brackets = 0;

    for (size_t i = 0; i != length; ++i) {
        const char curr_char = buf[i];
        if (is_comment_opened) {
            if (curr_char == '\n' || curr_char == '\r') {
                is_comment_opened = false;
            }
            continue;
        }
        
        if (is_string_opened)
        {// This string is part of the type hint.
            if (curr_char == '\'' || curr_char == '\"')
            {// Only '\'' or '\"' chars can close / open string
                if (curr_char != string_opening_char)
                {// Context is "data'... or 'data"... or """data'... or '''data"...
                    continue;
                }

                // Current char is equal to the char that opened the string.
                // Context is "data"... or 'data'... or """data"... or '''data'...

                if (!is_long_string_opened || (length + 2 < length && buf[i + 1] == curr_char && buf[i + 2] == curr_char))
                {// Context is in the string like 'data'... or "data"...
                 // or
                 // '''data'''... or """data"""... and long string is closed.
                    is_string_opened = false;
                    is_long_string_opened = false;
                    string_opening_char = '\0';
                }
            }
            continue;
        }

        switch (curr_char) {
        case '#':
            is_comment_opened = true;
            continue;
        case '\'':
        case '\"':
            is_string_opened = true;
            if (i + 2 < length && buf[i + 1] == curr_char && buf[i + 2] == curr_char) {
                is_long_string_opened = true;
            }
            string_opening_char = curr_char;
            continue;
        case ':':
            if (opened_square_brackets <= 0 && opened_curly_brackets <= 0 && (i + 1 == length || buf[i + 1] != '='))
            {// walrus operator a := 10
                symbols_indexes[0].push_back(i);
            }
            continue;
        case '{':
            ++opened_curly_brackets;
            symbols_indexes[1].push_back(i);
            continue;
        case '}':
            --opened_curly_brackets;
            symbols_indexes[2].push_back(i);
            continue;
        case '[':
            ++opened_square_brackets;
            symbols_indexes[3].push_back(i);
            continue;
        case ']':
            --opened_square_brackets;
            symbols_indexes[4].push_back(i);
            continue;
        case '=':
            if ((i + 6 < length) && buf[i + 1] == 'l') {
                if ((buf[i + 2] == 'a') & (buf[i + 3] == 'm') & (buf[i + 4] == 'b') & (buf[i + 5] == 'd') & (buf[i + 6] == 'a')) {
                    if (i + 7 == length || (i + 8 == length && buf[i + 7] == ':'))
                    {// ...=lambda or ...=lambda:
                        contains_lambda = true;
                    }
                }
            }
            if (i == 0 || buf[i - 1] != ':') {
                equal_operator_index = i;
            }

            continue;
        }
    }

    return contains_lambda;
}

static inline constexpr void clear_symbols_vects(std::vector<size_t> *symbols_indexes) {
    symbols_indexes[0].clear();
    symbols_indexes[1].clear();
    symbols_indexes[2].clear();
    symbols_indexes[3].clear();
    symbols_indexes[4].clear();
}

static inline constexpr bool is_colon_operator(const char *buf, const size_t length) noexcept {
    switch (buf[0])
    {
    case 'i':
        return ((length == 2) && (buf[1] == 'f'));
    case 'f':
        if (length < 7) {
            return (length == 3) && (buf[1] == 'o') && (buf[2] == 'r');
        }
        
        return (buf[1] == 'i' && buf[2] == 'n' && buf[3] == 'a' && buf[4] == 'l' && buf[5] == 'l' && buf[6] == 'y') &&
                ((length == 7) || (length == 8 && buf[7] == ':'));
    case 't':
        return ((length == 3) || (length == 4 && buf[3] == ':')) && (buf[1] == 'r' && buf[2] == 'y');
    case 'e':
        if ((length == 4) || (length == 5)) {
            if (buf[1] != 'l') {
                return false;
            }

            const char c2 = buf[2];
            const char c3 = buf[3];
            if (c2 == 's' && c3 == 'e') {
                return (length == 4) || (buf[4] == ':');
            }

            return (length == 4) && (c2 == 'i' && c3 == 'f');
        }
        return ((length == 6) || (length == 7 && buf[6] == ':')) &&
            (buf[1] == 'x' && buf[2] == 'c' && buf[3] == 'e' && buf[4] == 'p' && buf[5] == 't');
    case 'l':
        return ((length == 6) || (length == 7 && buf[6] == ':')) &&
            (buf[1] == 'a' && buf[2] == 'm' && buf[3] == 'b' && buf[4] == 'd' && buf[5] == 'a');
    case 'w':
        if (length == 4) {
            return (buf[1] == 'i' && buf[2] == 't' && buf[3] == 'h');
        }

        return ((length == 5) &&
            (buf[1] == 'h' && buf[2] == 'i' && buf[3] == 'l' && buf[4] == 'e'));
    case 'c':
        if (length == 4) {
            return  (buf[1] == 'a' && buf[2] == 's' && buf[3] == 'e');
        }

        return ((length == 5) &&
            (buf[1] == 'l' && buf[2] == 'a' && buf[3] == 's' && buf[4] == 's'));
    case 'm':
        return  ((length == 5) &&
            (buf[1] == 'a' && buf[2] == 't' && buf[3] == 'c' && buf[4] == 'h'));
    default:
        return false;
    }
}

static inline constexpr bool is_not_delim(const char c) noexcept {
    switch (c) {
    case ' ':
    case '\\':
    case ';':
    case '\t':
    case '\n':
    case '\r':
        return false;
    default:
        return true;
    }
}

static inline constexpr bool is_function_defenition(const char *buf, const size_t length) noexcept {
    return (length == 3) && (buf[0] == 'd' && buf[1] == 'e' && buf[2] == 'f');
}

static inline constexpr bool is_function_accepted_space(const char c) noexcept {
    switch (c) {
    case ' ':
    case '\\':
    case '\t':
    case '\n':
    case '\r':
        return true;
    default:
        return false;
    }
}

static inline ssize_t bin_search_elem_index_less_then_elem(const std::vector<size_t> &vec, size_t elem) {
    size_t l = 0;
    size_t r = vec.size();
    if (r-- == 0 || elem <= vec[0]) {
        return (ssize_t)(-1);
    }
    
    if (elem > vec[r]) {
        return (ssize_t)r;
    }
    
    while (r > l) {
        const size_t m_index = (l + r + 1) >> 1;
        const size_t m_elem = vec[m_index];
        if (m_elem > elem) {
            r = m_index - 1;
        } else if (m_elem == elem) {
            return m_index - 1;
        } else {
            l = m_index;
        }
    }

    return (ssize_t)r;
}

static ErrorCodes
process_file_internal(
    std::ifstream &fin,
    std::ofstream &fout,
    const std::unordered_set<std::string> &ignored_functions,
    const PreprocessorFlags preprocessor_flags
) {
    const bool is_debug_mode = (preprocessor_flags & PreprocessorFlags::debug) != PreprocessorFlags::no_flags;
    const bool is_verbose_mode = (preprocessor_flags & PreprocessorFlags::verbose) != PreprocessorFlags::no_flags;
    const bool is_continue_on_error = (preprocessor_flags & PreprocessorFlags::continue_on_error) == PreprocessorFlags::no_flags;
    ErrorCodes current_state = ErrorCodes::no_errors;
    
    /* Indexes of ':' , '{', '}', '[', ']' in term.*/
    std::vector<size_t> symbols_indexes[5];
    symbols_indexes[0].reserve(8);
    symbols_indexes[1].reserve(8);
    symbols_indexes[2].reserve(8);
    symbols_indexes[3].reserve(8);
    symbols_indexes[4].reserve(8);

    size_t buff_length = 0;
    char buf[MAX_BUFF_SIZE] {};
    char fallback_buffer[MAX_BUFF_SIZE] {};

    uint32_t colon_operators_starts = 0;
    int dict_or_set_init_starts = 0;
    int list_or_index_init_starts = 0;
    uint32_t lines_count = 1;

    int curr_char = 0;
    bool is_comment_opened = false;
    bool is_string_opened = false;
    bool is_long_string_opened = false;
    char string_opening_char = '\0';
    uint32_t late_line_increase_counter = 0;

    while ((curr_char = fin.get()) != -1) {
        if (is_not_delim(curr_char) || is_string_opened || is_comment_opened) {
            Check_BuffLen(buff_length)
            buf[buff_length++] = (char)curr_char;

            if (is_comment_opened) {
                if (curr_char == '\n' || curr_char == '\r') {
                    ++late_line_increase_counter;
                    is_comment_opened = false;
                }
                continue;
            }

            if (is_string_opened)
            {// This string is part of the type hint.
                if (curr_char == '\'' || curr_char == '\"')
                {// Only '\'' or '\"' chars can close / open string
                    if (curr_char != string_opening_char)
                    {// Context is "'... or '"...
                        continue;
                    }

                    // Current char is equal to the char that opened the string.

                    if (!is_long_string_opened)
                    {// Context is in the string like 'data' or "data"
                        is_string_opened = false;
                        string_opening_char = '\0';
                    } else
                    {// Context is like """data"... or '''data'...
                        curr_char = fin.get();
                        AssertWithArgs(
                            curr_char != -1,
                            ErrorCodes::function_return_type_hint_parse_error,
                            "Got EOF while reading string at line %u. String was not closed.\n",
                            lines_count
                        )
                        Check_BuffLen_Reserve(buff_length, 3)
                        buf[buff_length++] = (char)curr_char;

                        if (curr_char != string_opening_char)
                        {// Context is """data"some_char... or '''data'some_char...
                            check_newline_char
                            continue;
                        }

                        // Context is """data""... or '''data''...

                        curr_char = fin.get();
                        AssertWithArgs(
                            curr_char != -1,
                            ErrorCodes::function_return_type_hint_parse_error,
                            "Got EOF while reading string at line %u. String was not closed.\n",
                            lines_count
                        )
                        buf[buff_length++] = (char)curr_char;

                        if (curr_char != string_opening_char)
                        {// Context is """data""some_char... or '''data''some_char...
                            check_newline_char
                            continue;
                        }

                        // Context is """data"""... or '''data'''...
                        is_string_opened = false;
                        is_long_string_opened = false;
                        string_opening_char = '\0';
                    }
                }
                continue;
            }

            switch (curr_char) {
            case '\'':
            case '\"':
                // Context is "... or '...
                is_string_opened = true;
                string_opening_char = curr_char;

                curr_char = fin.get();
                AssertWithArgs(
                    curr_char != -1,
                    ErrorCodes::function_return_type_hint_parse_error,
                    "Got EOF while reading opened string at line %u\nCurrent term is '%s'\n",
                    lines_count,
                    buf
                )
                Check_BuffLen(buff_length)
                buf[buff_length++] = (char)curr_char;

                if (curr_char != string_opening_char)
                {// Context is "some_char... or 'some_char...
                    check_newline_char
                    continue;
                }

                // Context is ""... or ''...

                curr_char = fin.get();
                if (curr_char == -1)
                {// File ended.
                    break;
                }
                Check_BuffLen(buff_length)
                buf[buff_length++] = (char)curr_char;

                if (curr_char == string_opening_char)
                {// Context is """... or '''...
                    is_long_string_opened = true;
                    is_string_opened = true;
                } else
                {// Context is ""some_char... or ''some_char...
                    is_long_string_opened = false;
                    is_string_opened = false;
                    check_newline_char
                }
                continue;
            case '\n':
            case '\r':
                ++late_line_increase_counter;
                continue;
            case '#':
                is_comment_opened = true;
                continue;
            default:
                continue;
            }
        }

        check_newline_char
        is_long_string_opened = false;
        is_string_opened = false;

#ifdef _MSC_VER
#pragma region Special_symbols_counting
#endif
        size_t equal_operator_index = buff_length;
        const bool contains_lambda = count_symbols(buf, buff_length, symbols_indexes, equal_operator_index);

        const auto &colon_symbols_indexes = symbols_indexes[0];
        const auto &ds_open_symbols_indexes = symbols_indexes[1];
        const auto &ds_close_symbols_indexes = symbols_indexes[2];
        const auto &li_open_symbols_indexes = symbols_indexes[3];
        const auto &li_close_symbols_indexes = symbols_indexes[4];

        const size_t colon_indexes_count = colon_symbols_indexes.size();
        AssertWithArgs(
            colon_indexes_count <= 1,
            ErrorCodes::too_much_colon_symbols,
            "More then one ':' symbol (not walrus operator ':=') in one term is not supported\nCurrent term is '%s' at line %u\n",
            buf,
            lines_count
        )
        const bool contains_colon_symbol = colon_indexes_count != 0;
        const size_t colon_index = contains_colon_symbol ? colon_symbols_indexes[0] : buff_length;

        const bool is_in_initialization_context = (dict_or_set_init_starts > 0) || (list_or_index_init_starts > 0);
        const bool brackets_can_be_after_colon_symbol = contains_colon_symbol && !is_in_initialization_context;

        const int dict_or_set_open_symbols_before_colon_count =
            brackets_can_be_after_colon_symbol
            ? (int)(bin_search_elem_index_less_then_elem(ds_open_symbols_indexes, colon_index) + 1)
            : (int)ds_open_symbols_indexes.size();
        
        const int dict_or_set_close_symbols_before_colon_count =
            brackets_can_be_after_colon_symbol
            ? (int)(bin_search_elem_index_less_then_elem(ds_close_symbols_indexes, colon_index) + 1)
            : (int)ds_close_symbols_indexes.size();

        const int dict_or_set_open_minus_close_symbols_before_colon_count = 
            dict_or_set_open_symbols_before_colon_count - dict_or_set_close_symbols_before_colon_count;

        const size_t total_list_or_index_open_symbols = li_open_symbols_indexes.size();
        const int list_or_index_open_symbols_before_colon_count = 
            brackets_can_be_after_colon_symbol
            ? (int)(bin_search_elem_index_less_then_elem(li_open_symbols_indexes, colon_index) + 1)
            : (int)total_list_or_index_open_symbols;

        const size_t total_list_or_index_close_symbols = li_close_symbols_indexes.size();
        const int list_or_index_close_symbols_before_colon_count =
            brackets_can_be_after_colon_symbol
            ? (int)(bin_search_elem_index_less_then_elem(li_close_symbols_indexes, colon_index) + 1)
            : (int)li_close_symbols_indexes.size();
        
        const int list_or_index_open_minus_close_symbols_before_colon_count = 
            list_or_index_open_symbols_before_colon_count - list_or_index_close_symbols_before_colon_count;

        clear_symbols_vects(symbols_indexes);
#ifdef _MSC_VER
#pragma endregion Special_symbols_counting
#endif

#ifdef _MSC_VER
#pragma region Function_parsing
#endif
        if (is_function_defenition(buf, buff_length)) {
            Check_BuffLen(buff_length)
            buf[buff_length++] = curr_char; // add delimeter char after 'def'. Probably a ' ' or '\\'
            while ((curr_char = fin.get()) != -1) {
                Check_BuffLen(buff_length)
                buf[buff_length++] = (char)curr_char;
                if (!is_function_accepted_space(curr_char)) {
                    break;
                }
                check_newline_char
            }
            AssertWithArgs(
                curr_char != -1,
                ErrorCodes::function_parse_error,
                "Got EOF instead of function name at line %u\nCurrent term is '%s'\n",
                lines_count,
                buf
            )

            std::string function_name;
            function_name.reserve(79);
            function_name += curr_char;

            // Read function name.
            while ((curr_char = fin.get()) != -1) {
                Check_BuffLen(buff_length)
                buf[buff_length++] = (char)curr_char;

                if (is_comment_opened) {
                    if (curr_char == '\n' || curr_char == '\r') {
                        ++late_line_increase_counter;
                        is_comment_opened = false;
                    }
                    continue;
                }

                switch (curr_char) {
                case '(':
                    goto function_name_ended;
                case '\n':
                case '\r':
                    ++late_line_increase_counter;
                case '\\':
                    continue;
                case '#':
                    is_comment_opened = true;
                    continue;
                case '\"':
                case '\'':
                    AssertWithArgs(
                        false,
                        ErrorCodes::function_name_parse_error,
                        "String opening chars like ' and \" at line %u are not allowed for function name\n",
                        lines_count
                    )
                    continue;
                default:
                    function_name += curr_char;
                    continue;
                }
            }
            AssertWithArgs(
                curr_char != -1,
                ErrorCodes::function_name_parse_error,
                "Got EOF instead of function name at line %u\nCurrent term is '%s'\n",
                lines_count,
                buf
            )

            function_name_ended:
            const bool ignore_function = ignored_functions.contains(function_name);

            // Go through function params.
            uint32_t opened_round_brackets = 1;
            uint32_t opened_square_brackets = 0;
            is_comment_opened = false;
            is_string_opened = false;
            is_long_string_opened = false;
            string_opening_char = '\0'; // will be either '\'' or '\"'
            bool default_value_initialization_started = false;
            bool should_write_to_buf = true;

            function_arg_ended:
            default_value_initialization_started = false;
            should_write_to_buf = true;
            AssertWithArgs(
                !is_string_opened,
                ErrorCodes::string_not_closed_error | ErrorCodes::function_argument_parse_error,
                "Function argument ended but opened string was not closed %u",
                lines_count
            )

            while ((curr_char = fin.get()) != -1) {
                if (is_comment_opened) {
                    if (curr_char == '\n' || curr_char == '\r') {
                        ++late_line_increase_counter;
                        is_comment_opened = false;
                    }

                    if (should_write_to_buf) {
                        Check_BuffLen(buff_length)
                        buf[buff_length++] = (char)curr_char;
                    }

                    continue;
                }

                if (is_string_opened)
                {// This string is part of the type hint.
                    if (should_write_to_buf) {
                        Check_BuffLen_Reserve(buff_length, 3)
                        buf[buff_length++] = (char)curr_char;
                    }

                    if (curr_char == '\'' || curr_char == '\"')
                    {// Only '\'' or '\"' chars can close / open string
                        if (curr_char != string_opening_char)
                        {// Context is "'... or '"...
                            continue;
                        }

                        // Current char is equal to the char that opened the string.

                        if (!is_long_string_opened)
                        {// Context is in the string like 'data' or "data"
                            is_string_opened = false;
                            string_opening_char = '\0';
                        } else
                        {// Context is like """data"... or '''data'...
                            curr_char = fin.get();
                            AssertWithArgs(
                                curr_char != -1,
                                ErrorCodes::function_return_type_hint_parse_error,
                                "Got EOF while reading string at line %u. String was not closed.\n",
                                lines_count
                            )
                            
                            if (should_write_to_buf) {
                                buf[buff_length++] = (char)curr_char;
                            }

                            if (curr_char != string_opening_char)
                            {// Context is """data"some_char... or '''data'some_char...
                                check_newline_char
                                continue;
                            }

                            // Context is """data""... or '''data''...

                            curr_char = fin.get();
                            AssertWithArgs(
                                curr_char != -1,
                                ErrorCodes::function_return_type_hint_parse_error,
                                "Got EOF while reading string at line %u. String was not closed.\n",
                                lines_count
                            )
                            if (should_write_to_buf) {
                                buf[buff_length++] = (char)curr_char;
                            }

                            if (curr_char != string_opening_char)
                            {// Context is """data""some_char... or '''data''some_char...
                                check_newline_char
                                continue;
                            }

                            // Context is """data"""... or '''data'''...
                            is_string_opened = false;
                            is_long_string_opened = false;
                            string_opening_char = '\0';
                        }
                    }
                    continue;
                }

                switch (curr_char) {
                case '\'':
                case '\"':
                    // Context is "... or '...
                    is_string_opened = true;
                    string_opening_char = curr_char;
                    if (should_write_to_buf) {
                        buf[buff_length++] = (char)curr_char;
                    }

                    curr_char = fin.get();
                    AssertWithArgs(
                        curr_char != -1,
                        ErrorCodes::function_return_type_hint_parse_error,
                        "Got EOF while reading opened string at line %u\nCurrent term is '%s'\n",
                        lines_count,
                        buf
                    )
                    
                    if (should_write_to_buf) {
                        Check_BuffLen_Reserve(buff_length, 2)
                        buf[buff_length++] = (char)curr_char;
                    }

                    if (curr_char != string_opening_char)
                    {// Context is "some_char... or 'some_char...
                        check_newline_char
                        continue;
                    }

                    // Context is ""... or ''...

                    curr_char = fin.get();
                    AssertWithArgs(
                        curr_char != -1,
                        ErrorCodes::function_return_type_hint_parse_error,
                        "Got EOF while reading opened string in the type hint at line %u\nCurrent term is '%s'\n",
                        lines_count,
                        buf
                    )

                    if (should_write_to_buf) {
                        buf[buff_length++] = (char)curr_char;
                    }

                    if (curr_char == string_opening_char)
                    {// Context is """... or '''...
                        is_long_string_opened = true;
                        is_string_opened = true;
                    } else
                    {// Context is ""some_char... or ''some_char...
                        is_long_string_opened = false;
                        is_string_opened = false;

                        switch (curr_char)
                        {
                        case '\n':
                        case '\r':
                            ++late_line_increase_counter;
                            continue;
                        case '#':
                            is_comment_opened = true;
                            continue;
                        }
                    }

                    continue;
                case ',':
                    if (opened_square_brackets == 0 && opened_round_brackets == 1)
                    {// Current function arg ended.
                     // Check if we are not in the type hint like dict[int, dict] and not in default arg initialization ctor.
                        Check_BuffLen(buff_length)
                        buf[buff_length++] = ',';
                        goto function_arg_ended;
                    }
                    break;
                case '[':
                    ++opened_square_brackets;
                    break;
                case ']':
                    AssertWithArgs(
                        opened_square_brackets != 0,
                        ErrorCodes::function_argument_type_hint_parse_error, 
                        "Too much closing square brackets in the function argument type hint at line %u\nCurrent term is '%s'\n",
                        lines_count,
                        buf
                    );
                    --opened_square_brackets;
                    break;
                case '(':
                    ++opened_round_brackets;
                    break;
                case ')':
                    AssertWithArgs(
                        opened_round_brackets != 0,
                        ErrorCodes::too_much_closing_round_brackets,
                        "Too much closing round brackets in the function arguments initialization at line %u",
                        lines_count
                    )
                    if (--opened_round_brackets == 0) {
                        goto function_params_initialization_end;
                    }
                    break;
                case ':':
                    if (!default_value_initialization_started)
                    {// Typehint started.
                        should_write_to_buf = ignore_function;
                    }
                    break;
                case '=':
                    default_value_initialization_started = true;
                    should_write_to_buf = true;
                    break;
                case '\n':
                case '\r':
                    ++late_line_increase_counter;
                    break;
                case '#':
                    is_comment_opened = true;
                    break;
                }

                if (should_write_to_buf) {
                    Check_BuffLen(buff_length)
                    buf[buff_length++] = (char)curr_char;
                }
            }
            AssertWithArgs(
                curr_char != -1,
                ErrorCodes::function_parse_error,
                "Got EOF instead of function args, body or return type at line %u\nCurrent term is '%s'\n",
                lines_count,
                buf
            )

            function_params_initialization_end:           
            Check_BuffLen_Reserve(buff_length, 2)
            AssertWithArgs(
                curr_char == ')',
                ErrorCodes::function_parse_error,
                "Expected ')' symbol after function params, got '%c' at line %u\nCurrent term is '%s'\n",
                curr_char,
                lines_count,
                buf
            )
            buf[buff_length++] = ')';

            while ((curr_char = fin.get()) != -1) {
                if (!is_function_accepted_space(curr_char)) {
                    break;
                }

                check_newline_char

                Check_BuffLen(buff_length);
                buf[buff_length++] = (char)curr_char;
            }
            AssertWithArgs(
                curr_char != -1,
                ErrorCodes::function_return_type_hint_parse_error,
                "Got EOF instead of function body or return type at line %u\nCurrent term is '%s'\n",
                lines_count,
                buf
            )

            if (curr_char == ':') {
                goto write_buf;
            }
            AssertWithArgs(
                curr_char == '-',
                ErrorCodes::function_return_type_hint_parse_error,
                "Expected '-' symbol for function return type hint construction 'def foo() -> return_type:', got '%c' at line %u\nCurrent term is '%s'\n",
                curr_char,
                lines_count,
                buf
            )
            curr_char = fin.get();
            AssertWithArgs(
                curr_char != -1,
                ErrorCodes::function_return_type_hint_parse_error,
                "Got EOF instead of function return type hint at line %u\n",
                lines_count
            )
            AssertWithArgs(
                curr_char == '>',
                ErrorCodes::function_return_type_hint_parse_error,
                "Expected '>' symbol for function return type hint construction 'def foo() -> return_type:', got '%c' at line %u\nCurrent term is '%s'\n",
                curr_char,
                lines_count,
                buf
            )

            is_comment_opened = false;
            is_string_opened = false;
            is_long_string_opened = false;
            string_opening_char = '\0';

            if (ignore_function) {
                buf[buff_length++] = '-';
                buf[buff_length++] = '>';
            }

            // Go through function type hint.
            while ((curr_char = fin.get()) != -1) {
                if (is_comment_opened) {
                    if (curr_char == '\n' || curr_char == '\r') {
                        ++late_line_increase_counter;
                        is_comment_opened = false;
                    }

                    if (should_write_to_buf) {
                        Check_BuffLen(buff_length)
                        buf[buff_length++] = (char)curr_char;
                    }

                    continue;
                }

                if (is_string_opened)
                {// This string is part of the type hint.
                    if (ignore_function) {
                        Check_BuffLen_Reserve(buff_length, 2)
                        buf[buff_length++] = (char)curr_char;
                    }

                    if (curr_char == '\'' || curr_char == '\"')
                    {// Only '\'' or '\"' chars can close / open string
                        if (curr_char != string_opening_char)
                        {// Context is "'... or '"...
                            continue;
                        }

                        // Current char is equal to the char that opened the string.
                        // Context is "data"... or 'data'... or """data"... or '''data'...

                        if (!is_long_string_opened)
                        {// Context is in the string like 'data' or "data"
                            is_string_opened = false;
                            string_opening_char = '\0';
                        } else
                        {// Context is like """data"... or '''data'...
                            curr_char = fin.get();
                            AssertWithArgs(
                                curr_char != -1,
                                ErrorCodes::function_return_type_hint_parse_error,
                                "Got EOF instead of function return type hint at line %u. String was not closed.\n",
                                lines_count
                            )
                            if (ignore_function) {
                                buf[buff_length++] = (char)curr_char;
                            }

                            if (curr_char != string_opening_char)
                            {// Context is """data"some_char... or '''data'some_char...
                                check_newline_char
                                continue;
                            }

                            // Context is """data""... or '''data''...

                            curr_char = fin.get();
                            AssertWithArgs(
                                curr_char != -1,
                                ErrorCodes::function_return_type_hint_parse_error,
                                "Got EOF instead of function return type hint at line %u. String was not closed.\n",
                                lines_count
                            )
                            if (ignore_function) {
                                buf[buff_length++] = (char)curr_char;
                            }

                            if (curr_char != string_opening_char)
                            {// Context is """data""some_char... or '''data''some_char...
                                check_newline_char
                                continue;
                            }

                            // Context is """data"""... or '''data'''...
                            is_string_opened = false;
                            is_long_string_opened = false;
                            string_opening_char = '\0';
                        }
                    }
                    continue;
                }

                switch (curr_char) {
                case '\'':
                case '\"':
                    {// Context is '... or "...
                        if (ignore_function) {
                            Check_BuffLen(buff_length);
                            buf[buff_length++] = (char)curr_char;
                        }

                        is_string_opened = true;
                        string_opening_char = curr_char;

                        curr_char = fin.get();
                        AssertWithArgs(
                            curr_char != -1,
                            ErrorCodes::function_return_type_hint_parse_error,
                            "Got EOF instead of function return type hint at line %u. String was not closed.\n",
                            lines_count
                        )

                        if (ignore_function) {
                            Check_BuffLen_Reserve(buff_length, 2)
                            buf[buff_length++] = (char)curr_char;
                        }

                        if (curr_char != string_opening_char)
                        {// Context is "some_char... or 'some_char...
                            check_newline_char
                            is_string_opened = true;
                            continue;
                        }

                        // Context is ""... or ''...

                        curr_char = fin.get();
                        AssertWithArgs(
                            curr_char != -1,
                            ErrorCodes::function_return_type_hint_parse_error,
                            "Got EOF instead of function return type hint at line %u\nCurrent term is '%s'\n",
                            lines_count,
                            buf
                        )

                        if (curr_char == string_opening_char)
                        {// Context is """... or '''...
                            is_long_string_opened = true;
                            is_string_opened = true;
                        } else
                        {// Context is ""some_char... or ''some_char...
                            is_long_string_opened = false;
                            is_string_opened = false;
                            string_opening_char = '\0';
                            check_newline_char
                        }

                        if (ignore_function) {
                            buf[buff_length++] = (char)curr_char;
                        }
                        continue;
                    }
                case '#':
                    is_comment_opened = true;
                    break;
                case ':':
                    goto write_buf;
                case '\n':
                case '\r':
                    ++late_line_increase_counter;
                    [[fallthrough]];
                case ' ':
                case '\\':
                case '\t':
                    Check_BuffLen(buff_length);
                    buf[buff_length++] = curr_char;
                    continue;
                }

                if (ignore_function) {
                    Check_BuffLen(buff_length);
                    buf[buff_length++] = (char)curr_char;
                }
            }

            if ((preprocessor_flags & PreprocessorFlags::verbose) != PreprocessorFlags::no_flags) {
                std::clog << "Got EOF instead of function initialization end symbol ':' at line " << lines_count << "\nFile processing cant be continued\n";
            }

            return current_state |= ErrorCodes::function_return_type_hint_parse_error;
        }
#ifdef _MSC_VER
#pragma endregion Function_parsing
#endif
        if (is_colon_operator(buf, buff_length) || contains_lambda) {
            if (!contains_colon_symbol) {
                ++colon_operators_starts;
            }
            goto write_buf;
        }
        
        if (contains_colon_symbol)
        {// Probably type hint started.
            if (colon_operators_starts != 0) {
                --colon_operators_starts;
                goto write_buf;
            }
            if (is_in_initialization_context
            || (dict_or_set_open_minus_close_symbols_before_colon_count > 0)
            || (list_or_index_open_minus_close_symbols_before_colon_count > 0)) {
                goto write_buf;
            }

            if (equal_operator_index != buff_length) {
                memmove(&buf[colon_index], &buf[equal_operator_index], buff_length - equal_operator_index);
                buf[colon_index + buff_length - equal_operator_index] = '\0';
                goto write_buf;
            }

            uint32_t opened_square_brackets =
                (total_list_or_index_open_symbols - list_or_index_open_symbols_before_colon_count)
                - (total_list_or_index_close_symbols - list_or_index_close_symbols_before_colon_count);
            
            fallback_buffer[0] = ':'; // Buffer is used in case variable has typehint without initialization.
            fallback_buffer[1] = curr_char;
            size_t fallback_buffer_size = 2;
            
            buff_length = colon_index;
            while ((curr_char = fin.get()) != -1) {
                fallback_buffer[fallback_buffer_size++] = (char)curr_char;

                if (is_comment_opened) {
                    if (curr_char == '\n' || curr_char == '\r') {
                        ++late_line_increase_counter;
                        is_comment_opened = false;
                    }
                    continue;
                }

                if (is_string_opened)
                {// This string is part of the type hint.
                    if (curr_char == '\'' || curr_char == '\"')
                    {// Only '\'' or '\"' chars can close / open string
                        if (curr_char != string_opening_char)
                        {// Context is "'... or '"...
                            continue;
                        }

                        // Current char is equal to the char that opened the string.
                        // Context is "data"... or 'data'... or """data"... or '''data'...

                        if (!is_long_string_opened)
                        {// Context is in the string like 'data' or "data"
                            is_string_opened = false;
                            string_opening_char = '\0';
                        } else
                        {// Context is like """data"... or '''data'...
                            curr_char = fin.get();
                            AssertWithArgs(
                                curr_char != -1,
                                ErrorCodes::function_return_type_hint_parse_error,
                                "Got EOF instead of type hint end at line %u. String was not closed.\n",
                                lines_count
                            )
                            fallback_buffer[fallback_buffer_size++] = (char)curr_char;

                            if (curr_char != string_opening_char)
                            {// Context is """data"some_char... or '''data'some_char...
                                check_newline_char
                                continue;
                            }

                            // Context is """data""... or '''data''...

                            curr_char = fin.get();
                            AssertWithArgs(
                                curr_char != -1,
                                ErrorCodes::unexpected_eof,
                                "Got EOF instead of type hint end at line %u. String was not closed.\n",
                                lines_count
                            )
                            fallback_buffer[fallback_buffer_size++] = (char)curr_char;

                            if (curr_char != string_opening_char)
                            {// Context is """data""some_char... or '''data''some_char...
                                check_newline_char
                                continue;
                            }

                            // Context is """data"""... or '''data'''...
                            is_string_opened = false;
                            is_long_string_opened = false;
                            string_opening_char = '\0';
                        }
                    }
                    continue;
                }

                switch (curr_char) {
                case '\'':
                case '\"':
                    {// Context is '... or "...
                        is_string_opened = true;
                        string_opening_char = curr_char;

                        curr_char = fin.get();
                        AssertWithArgs(
                            curr_char != -1,
                            ErrorCodes::unexpected_eof,
                            "Got EOF instead of type hint end at line %u\nCurrent term is '%s'\n",
                            lines_count,
                            buf
                        )
                        fallback_buffer[fallback_buffer_size++] = (char)curr_char;

                        if (curr_char != string_opening_char)
                        {// Context is "some_char... or 'some_char...
                            check_newline_char
                            is_string_opened = true;
                            continue;
                        }

                        // Context is ""... or ''...

                        curr_char = fin.get();
                        AssertWithArgs(
                            curr_char != -1,
                            ErrorCodes::unexpected_eof,
                            "Got EOF instead of type hint end at line %u\nCurrent term is '%s'\n",
                            lines_count,
                            buf
                        )
                        fallback_buffer[fallback_buffer_size++] = (char)curr_char;

                        if (curr_char == string_opening_char)
                        {// Context is """... or '''...
                            is_long_string_opened = true;
                            is_string_opened = true;
                        } else
                        {// Context is ""some_char... or ''some_char...
                            is_long_string_opened = false;
                            is_string_opened = false;
                            string_opening_char = '\0';
                            check_newline_char
                        }

                        continue;
                    }
                case '#':
                    is_comment_opened = true;
                    continue;
                case '=':
                    goto write_buf;
                case '\n':
                case '\r':
                    ++late_line_increase_counter;
                    if (opened_square_brackets == 0 && buf[buff_length - 1] != '\\')
                    {// variable: type (without initialization)
                        memmove(&buf[colon_index], &fallback_buffer, fallback_buffer_size);
                        buff_length = colon_index + fallback_buffer_size;
                        buf[buff_length] = '\0';
                        fout << buf;
                        goto update_counters_and_buffer;
                    }
                    [[fallthrough]];
                case ' ':
                case '\\':
                case '\t':
                    Check_BuffLen(buff_length);
                    buf[buff_length++] = curr_char;
                    continue;
                case '[':
                    ++opened_square_brackets;
                    continue;
                case ']':
                    AssertWithArgs(
                        opened_square_brackets != 0,
                        ErrorCodes::too_much_closing_square_brackets,
                        "Closing bracket ']' without opened one at line %u\nCurrent term is '%s'\n",
                        lines_count,
                        buf
                    )
                    --opened_square_brackets;
                    continue;
                }
            }

            if (opened_square_brackets == 0 && buf[buff_length - 1] != '\\')
            {// variable: type (without initialization)
                memmove(&buf[colon_index], &fallback_buffer, fallback_buffer_size);
                buff_length = colon_index + fallback_buffer_size;
                buf[buff_length] = '\0';
                fout << buf;
                goto update_counters_and_buffer;
            }
            
            AssertWithArgs(
                false,
                ErrorCodes::unexpected_eof,
                "Got EOF instead of type hint end at line %u\nCurrent term is '%s'\n",
                lines_count,
                buf
            )
        }

        write_buf:
            if (buff_length != 0) {
                buf[buff_length] = '\0';
                fout << buf;
            }
            fout << (char)curr_char;
            goto update_counters_and_buffer;
        
        update_counters_and_buffer:
            dict_or_set_init_starts += dict_or_set_open_minus_close_symbols_before_colon_count;
            list_or_index_init_starts += list_or_index_open_minus_close_symbols_before_colon_count;

            if (is_debug_mode) {
                printf(
                    "Line: %u;\nTerm: '%s'; Buff length: %llu;\nColon colon operators starts: %u\n'{' - '}' on line count: %d;\n'[' - ']' on line count: %d;\n'{' counts: %d\n'[' counts: %d\n\n",
                    lines_count,
                    buf,
                    buff_length,
                    colon_operators_starts,
                    dict_or_set_open_minus_close_symbols_before_colon_count,
                    list_or_index_open_minus_close_symbols_before_colon_count,
                    dict_or_set_init_starts,
                    list_or_index_init_starts
                );
            }
            
            AssertWithArgs(
                dict_or_set_init_starts >= 0,
                ErrorCodes::too_much_closing_curly_brackets,
                "Closing bracket '}' without opened one at line %u\nCurrent term is '%s'\n",
                lines_count,
                buf
            )
            AssertWithArgs(
                list_or_index_init_starts >= 0,
                ErrorCodes::too_much_closing_square_brackets,
                "Closing bracket ']' without opened one at line %u\nCurrent term is '%s'\n",
                lines_count,
                buf
            )

            lines_count += late_line_increase_counter;
            is_string_opened = false;
            buff_length = 0;
            late_line_increase_counter = 0;

            continue;
    }

    /* Flush buffer */
    if (buff_length != 0) {
        buf[buff_length] = '\0';
        fout << buf;
    }

    fout.flush();
    return current_state;
}

std::string generate_tmp_filename(const std::string &filename) {
    const size_t slash_index = filename.rfind('/');
    if (slash_index != filename.npos) {
        return "tmp_" + filename.substr(slash_index + 1);
    }

    const size_t backslash_index = filename.rfind('\\');
    if (backslash_index != filename.npos) {
        return "tmp_" + filename.substr(backslash_index + 1);
    }
    
    return "tmp_" + filename;
}

ErrorCodes process_file(
    const std::string &input_filename,
    const std::unordered_set<std::string> &ignored_functions,
    const PreprocessorFlags preprocessor_flags
) {
    const bool is_verbose_mode = (preprocessor_flags & PreprocessorFlags::verbose) != PreprocessorFlags::no_flags;

    std::ifstream fin(input_filename);
    if (!fin.is_open()) {
        if (is_verbose_mode) {
            std::clog << "Was not able to open " << input_filename << '\n';
        }

        return ErrorCodes::src_file_open_error;
    }

    const std::string &tmp_file_name = generate_tmp_filename(input_filename);
    std::ofstream tmp_fout(tmp_file_name, std::ios::out | std::ios::trunc);
    if (!tmp_fout.is_open()) {
        fin.close();
        if (is_verbose_mode) {
            std::clog << "Was not able to open temporary file " << tmp_file_name << '\n';
        };

        return ErrorCodes::tmp_file_open_error;
    }

    ErrorCodes ret_code = process_file_internal(fin, tmp_fout, ignored_functions, preprocessor_flags);
    fin.close();
    tmp_fout.close();

    if (fin.bad()) {
        ret_code |= ErrorCodes::src_file_io_error;
        if (is_verbose_mode) {
            std::clog << "Input file stream (src code) got bad bit: 'Error on stream (such as when this function catches an exception thrown by an internal operation).'\n";
        }
        return ret_code;
    }

    if (ret_code != ErrorCodes::no_errors) {
        if (is_verbose_mode) {
            std::clog << "An error occured while processing src file " << input_filename << '\n';
        }
        return ret_code;
    }

    if (is_verbose_mode) {
        std::cout << "Successfully processed src file " << input_filename << '\n';
    }

    if ((preprocessor_flags & PreprocessorFlags::overwrite_file) != PreprocessorFlags::no_flags) {
        std::ofstream re_fin(input_filename, std::ios::binary | std::ios::trunc);
        std::ifstream re_tmp_fout(tmp_file_name, std::ios::binary);
        
        re_fin << re_tmp_fout.rdbuf();
        re_fin.close();
        re_tmp_fout.close();

        if (re_fin.bad() || re_tmp_fout.bad()) {
            ret_code |= ErrorCodes::overwrite_error;
            if (is_verbose_mode) {
                std::clog << "An error occured while overwriting tmp file " << tmp_file_name << " to source file " << input_filename << '\n';
            }
        }
        else if (is_verbose_mode) {
            std::cout << "Overwrote source file " << input_filename << '\n';
        }
        
        if (std::remove(tmp_file_name.data()) == 0) {
            if (is_verbose_mode) {
                std::cout << "Successfully deleted tmp file " << tmp_file_name << '\n';
            }
        }
        else {
            ret_code |= ErrorCodes::tmp_file_delete_error;
            if (is_verbose_mode) {
                std::clog << "An error occured while deleting tmp file " << tmp_file_name << '\n';
            }
        }
    }
    else if (is_verbose_mode) {
        std::cout << "Processed version is copied to the " << tmp_file_name << '\n';
    }

    return ret_code;
}

ErrorCodes process_files(
    const std::unordered_set<std::string> &filenames,
    const std::unordered_set<std::string> &ignored_functions,
    const PreprocessorFlags preprocessor_flags
) {
    const bool is_verbose_mode = (preprocessor_flags & PreprocessorFlags::verbose) != PreprocessorFlags::no_flags;
    const bool is_continue_on_error = (preprocessor_flags & PreprocessorFlags::continue_on_error) == PreprocessorFlags::no_flags;
    size_t processed_files = 0;
    const size_t total_files = filenames.size();
    ErrorCodes current_state = ErrorCodes::no_errors;
    
    for (const auto& filename : filenames) {
        const bool exists = std::filesystem::exists(filename);
        AssertWithArgs(
            exists,
            ErrorCodes::src_file_open_error,
            "Could not open file '%s'\n",
            filename.c_str()
        )
        if (!exists) {
            continue;
        }

        const ErrorCodes file_process_ret_code = process_file(filename, ignored_functions, preprocessor_flags);
        ++processed_files;
        current_state |= file_process_ret_code;
        const bool no_errors = file_process_ret_code == ErrorCodes::no_errors;
        AssertWithArgs(
            no_errors,
            ErrorCodes::single_file_process_error,
            "An error occured while processing %llu / %llu file '%s'\n",
            processed_files,
            total_files,
            filename.c_str()
        )

        if (is_verbose_mode) {
            std::cout << processed_files << " / " << total_files << (no_errors ? " file processed successfully\n" : "  file processed with failure\n");
        }
    }

    return current_state;
}
