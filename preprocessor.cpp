#include <fstream>     // ifstream, ofstream
#include <string>      // string
#include <cstring>     // memmove
#include <cstdint>     // size_t, uint32_t
#include <sys/types.h> // ssize_t
#include <ctime>       // time
#include <cstdarg>     // __VA_ARGS__
#include <cstdio>      // fprintf
#include <cstdlib>     // rand, srand
#include <iostream>    // cout, cerr
#include <vector>      // vector<T>
#include <type_traits> // is_same<>

#include "preprocessor.hpp"

using std::size_t;
using std::uint32_t;

/* must be power of two. */
static constexpr size_t MAX_BUFF_SIZE = 1024;
static_assert((MAX_BUFF_SIZE & (MAX_BUFF_SIZE - 1)) == 0);

#define Check_BuffLen(buff_length) \
    Check_BuffLen_Reserve(buff_length, (size_t)0)

#define Check_BuffLen_Reserve(buff_length, additional_reserve)\
    static_assert(std::is_same<decltype(buff_length), size_t>::value); \
    static_assert(std::is_same<decltype(additional_reserve), size_t>::value); \
    if ((buff_length + additional_reserve) & (~(MAX_BUFF_SIZE - 1))) {\
        if ((preprocessor_flags & PreprocessorFlags::verbose) != PreprocessorFlags::no_flags) {\
            std::clog << "Max buffer size is reached\n";\
        }\
        if ((preprocessor_flags & PreprocessorFlags::stop_on_error) != PreprocessorFlags::no_flags) {\
            return ErrorCodes::preprocessor_line_buffer_overflow;\
        } else {\
            current_state |= ErrorCodes::preprocessor_line_buffer_overflow;\
        }\
    }

#define Assert(expression, message, error_code) \
    static_assert(std::is_same<decltype((message)[0]), const char&>::value);\
    if (!(expression)) {\
        if ((preprocessor_flags & PreprocessorFlags::verbose) != PreprocessorFlags::no_flags) {\
            std::clog << "Assertion error. Error message:\n" << message << '\n';\
        }\
        if ((preprocessor_flags & PreprocessorFlags::stop_on_error) != PreprocessorFlags::no_flags) {\
            return error_code;\
        } else {\
            current_state |= error_code;\
        }\
    }

#define AssertWithArgs(expression, error_code, ...) \
    if (!(expression)) {\
        if ((preprocessor_flags & PreprocessorFlags::verbose) != PreprocessorFlags::no_flags) {\
            fprintf(stderr, __VA_ARGS__);\
        }\
        if ((preprocessor_flags & PreprocessorFlags::stop_on_error) != PreprocessorFlags::no_flags) {\
            return error_code;\
        } else {\
            current_state |= error_code;\
        }\
    }\

static inline void count_symbols(const char *buf, const size_t length, std::vector<size_t> *symbols_indexes) {
    for (size_t i = 0; i != length; ++i) {
        switch (buf[i]) {
        case ':':
            if (i + 1 == length || buf[i + 1] != '=')
            {// walrus operator a := 10
                symbols_indexes[0].push_back(i);
            }
            continue;
        case '{':
            symbols_indexes[1].push_back(i);
            continue;
        case '}':
            symbols_indexes[2].push_back(i);
            continue;
        case '[':
            symbols_indexes[3].push_back(i);
            continue;
        case ']':
            symbols_indexes[4].push_back(i);
            continue;
        }
    }
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

            char c2 = buf[2];
            char c3 = buf[3];
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
        return ((length == 5) &&
            (buf[1] == 'h' && buf[2] == 'i' && buf[3] == 'l' && buf[4] == 'e'));
    case 'c':
        return ((length == 5) &&
            (buf[1] == 'l' && buf[2] == 'a' && buf[3] == 's' && buf[4] == 's'));
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

static inline constexpr bool is_not_typehint_end(const char c) noexcept {
    return c != '=';
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

static ErrorCodes process_file_internal(std::ifstream &fin, std::ofstream &fout, const PreprocessorFlags preprocessor_flags) {
    const bool is_debug_mode = (preprocessor_flags & PreprocessorFlags::debug) != PreprocessorFlags::no_flags;
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

    uint32_t colon_separators_starts = 0;
    int dict_or_set_init_starts = 0;
    int list_or_index_init_starts = 0;
    uint32_t lines_count = 1;

    int curr_char = 0;
    while ((curr_char = fin.get()) != -1) {
        uint32_t late_line_increase_counter = 0;
        if (is_not_delim(curr_char)) {
            Check_BuffLen(buff_length)
            buf[buff_length++] = (char)curr_char;
            continue;
        }
        if (curr_char == '\n' || curr_char == '\r') {
            ++late_line_increase_counter;
        }

#ifdef _MSC_VER
#pragma region Special_symbols_counting
#endif
        count_symbols(buf, buff_length, symbols_indexes);

        const auto &colon_symbols_indexes = symbols_indexes[0];
        const auto &ds_open_symbols_indexes = symbols_indexes[1];
        const auto &ds_close_symbols_indexes = symbols_indexes[2];
        const auto &li_open_symbols_indexes = symbols_indexes[3];
        const auto &li_close_symbols_indexes = symbols_indexes[4];

        const size_t colon_indexes_count = colon_symbols_indexes.size();
        Assert(colon_indexes_count <= 1, "More then one ':' symbol (not walrus operator ':=') in one term is not supported", ErrorCodes::too_much_colon_symbols);
        const bool contains_colon_symbol = colon_indexes_count != 0;
        const size_t colon_index = contains_colon_symbol ? colon_symbols_indexes[0] : buff_length;

        const bool is_in_initialization_context = (dict_or_set_init_starts > 0) || (list_or_index_init_starts > 0);

        const int dict_or_set_open_symbols_before_colon_count =
            contains_colon_symbol && !is_in_initialization_context
            ? (int)(bin_search_elem_index_less_then_elem(ds_open_symbols_indexes, colon_index) + 1)
            : (int)ds_open_symbols_indexes.size();
        
        const int dict_or_set_close_symbols_before_colon_count =
            contains_colon_symbol && !is_in_initialization_context
            ? (int)(bin_search_elem_index_less_then_elem(ds_close_symbols_indexes, colon_index) + 1)
            : (int)ds_close_symbols_indexes.size();

        const int dict_or_set_open_minus_close_symbols_before_colon_count = 
            dict_or_set_open_symbols_before_colon_count - dict_or_set_close_symbols_before_colon_count;

        const int list_or_index_open_symbols_before_colon_count = 
            contains_colon_symbol && !is_in_initialization_context
            ? (int)(bin_search_elem_index_less_then_elem(li_open_symbols_indexes, colon_index) + 1)
            : (int)li_open_symbols_indexes.size();

        const int list_or_index_close_symbols_before_colon_count =
            contains_colon_symbol && !is_in_initialization_context
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
            buf[buff_length++] = curr_char;
            while ((curr_char = fin.get()) != -1) {
                Check_BuffLen(buff_length)
                buf[buff_length++] = (char)curr_char;
                if (!is_function_accepted_space(curr_char)) {
                    break;
                }
                if (curr_char == '\n' || curr_char == '\r') {
                    ++late_line_increase_counter;
                }
            }
            AssertWithArgs(
                curr_char != -1,
                ErrorCodes::function_parse_error,
                "Got EOF instead of function name initialization at line %u\n",
                lines_count
            )
            
            while ((curr_char = fin.get()) != -1) {
                Check_BuffLen(buff_length)
                buf[buff_length++] = (char)curr_char;
                if (curr_char == '(') {
                    break;
                }
                if (curr_char == '\n' || curr_char == '\r') {
                    ++late_line_increase_counter;
                }
            }
            AssertWithArgs(
                curr_char != -1,
                ErrorCodes::function_parse_error,
                "Got EOF instead of function params initialization at line %u\n",
                lines_count
            )

            while (true)
            {// Go through function pararms.
                bool arg_without_typehint = false;
                while ((curr_char = fin.get()) != -1) {
                    if (curr_char == ':')
                    {// typehint started
                        break;
                    }

                    if (curr_char == ')') {
                        goto function_params_initialization_end;
                    }

                    Check_BuffLen(buff_length)
                    buf[buff_length++] = (char)curr_char;

                    
                    if (curr_char == '=')
                    {// Function argument default value initialization.
                        arg_without_typehint = true;
                        break;
                    } else if (curr_char == '\n' || curr_char == '\r') {
                        ++late_line_increase_counter;
                    }
                }
                AssertWithArgs(
                    curr_char != -1,
                    ErrorCodes::function_parse_error,
                    "Got EOF instead of function argument type hint at line %u\n",
                    lines_count
                )

                while ((curr_char = fin.get()) != -1) {
                    if (curr_char == ',')
                    {// Current function arg ended.
                        Check_BuffLen(buff_length)
                        buf[buff_length++] = ',';
                        break;
                    }

                    if (curr_char == ')') {
                        goto function_params_initialization_end;
                    }

                    if (arg_without_typehint) {
                        buf[buff_length++] = (char)curr_char;
                    }

                    if (curr_char == '\n' || curr_char == '\r') {
                        ++late_line_increase_counter;
                    }
                }
                AssertWithArgs(
                    curr_char != -1,
                    ErrorCodes::function_parse_error,
                    "Got EOF instead of function body or return type at line %u\n",
                    lines_count
                )
            }

            function_params_initialization_end:           
            Check_BuffLen_Reserve(buff_length, (size_t)2)
            AssertWithArgs(
                curr_char == ')',
                ErrorCodes::function_parse_error,
                "Expected ')' symbol after function params, got '%c' at line %u\n",
                curr_char,
                lines_count
            )
            buf[buff_length++] = ')';

            while ((curr_char = fin.get()) != -1) {
                if (!is_function_accepted_space(curr_char)) {
                    break;
                }

                if (curr_char == '\n' || curr_char == '\r') {
                    ++late_line_increase_counter;
                }

                Check_BuffLen(buff_length);
                buf[buff_length++] = (char)curr_char;
            }
            AssertWithArgs(
                curr_char != -1,
                ErrorCodes::function_parse_error,
                "Got EOF instead of function body or return type at line %u\n",
                lines_count
            )

            if (curr_char == ':') {
                buf[buff_length++] = ':';
                goto write_buf_after_func;
            }
            AssertWithArgs(
                curr_char == '-',
                ErrorCodes::function_parse_error,
                "Expected '-' symbol for function return type hint construction 'def foo() -> return_type:', got '%c' at line %u\n",
                curr_char,
                lines_count
            )
            curr_char = fin.get();
            AssertWithArgs(
                curr_char != -1,
                ErrorCodes::function_parse_error,
                "Got EOF instead of function return type hint at line %u\n",
                lines_count
            )
            AssertWithArgs(
                curr_char == '>',
                ErrorCodes::function_parse_error,
                "Expected '>' symbol for function return type hint construction 'def foo() -> return_type:', got '%c' at line %u\n",
                curr_char,
                lines_count
            )

            while ((curr_char = fin.get()) != -1) {
                if (is_function_accepted_space(curr_char)) {
                    Check_BuffLen(buff_length);
                    buf[buff_length++] = curr_char;
                    
                    if (curr_char == '\n' || curr_char == '\r') {
                        ++late_line_increase_counter;
                    }

                    continue;
                }

                if (curr_char == ':') {
                    Check_BuffLen(buff_length);
                    buf[buff_length++] = ':';
                    goto write_buf_after_func;
                }
            }
            AssertWithArgs(
                false,
                ErrorCodes::function_parse_error,
                "Got EOF instead of function initialization end symbol ':' at line %u\n",
                lines_count
            )
            continue;
        }
#ifdef _MSC_VER
#pragma endregion Function_parsing
#endif
        
        if (is_colon_operator(buf, buff_length)) {
            ++colon_separators_starts;
            goto write_buf;
        }

        if (contains_colon_symbol)
        {// Probably type hint started.
            if (colon_separators_starts != 0) {
                --colon_separators_starts;
                goto write_buf;
            }
            if (is_in_initialization_context) {
                goto write_buf;
            }

            if ((dict_or_set_open_minus_close_symbols_before_colon_count <= 0) && (list_or_index_open_minus_close_symbols_before_colon_count <= 0)) {
                size_t typing_end_index = colon_index;
                while ((++typing_end_index != buff_length) && is_not_typehint_end(buf[typing_end_index]))
                    ;
                
                if (typing_end_index != buff_length)
                {// st:set[int]= {}
                    memmove(buf + colon_index, buf + typing_end_index, buff_length - typing_end_index);
                    buf[colon_index + buff_length - typing_end_index] = '\0';
                } else {
                    while (((curr_char = fin.get()) != -1) && is_not_typehint_end(curr_char))
                    {/*Skip stream untill '=' e.g. type hint end */
                        if (curr_char == '\n' || curr_char == '\r') {
                            ++late_line_increase_counter;
                        }
                    }
                    AssertWithArgs(
                        curr_char != -1,
                        ErrorCodes::unexpected_eof,
                        "Got EOF instead of type hint end at line %u\n",
                        lines_count
                    )
                    buf[colon_index] = '\0';
                }

                goto write_buf;
            }
        }

        write_buf:
            buf[buff_length] = '\0';
            fout << buf << (char)curr_char;
            goto update_counters_and_buffer;
        
        write_buf_after_func:
            buf[buff_length] = '\0';
            fout << buf;
            goto update_counters_and_buffer;
        
        update_counters_and_buffer:
            dict_or_set_init_starts += dict_or_set_open_minus_close_symbols_before_colon_count;
            list_or_index_init_starts += list_or_index_open_minus_close_symbols_before_colon_count;

            if (is_debug_mode) {
                printf(
                    "Line: %u;\nTerm: '%s'; Buff length: %llu;\n'{' - '}' on line count: %d;\n'[' - ']' on line count: %d;\n'{' counts: %d\n'[' counts: %d\n\n",
                    lines_count,
                    buf,
                    buff_length,
                    dict_or_set_open_minus_close_symbols_before_colon_count,
                    list_or_index_open_minus_close_symbols_before_colon_count,
                    dict_or_set_init_starts,
                    list_or_index_init_starts
                );
            }
            
            AssertWithArgs(
                dict_or_set_init_starts >= 0,
                ErrorCodes::too_much_closing_curly_brackets,
                "Closing bracket '}' without opened one at line %u\nCurrent term is %s\n",
                lines_count,
                buf
            )
            AssertWithArgs(
                list_or_index_init_starts >= 0,
                ErrorCodes::too_much_closing_square_brackets,
                "Closing bracket ']' without opened one at line %u\nCurrent term is %s\n",
                lines_count,
                buf
            )

            lines_count += late_line_increase_counter;
            buff_length = 0;

            continue;
    }

    /* Flush buffer */
    if (buff_length != 0){
        buf[buff_length] = '\0';
        fout << buf;
    }

    fout.flush();
    return current_state;
}

static inline std::string gen_random_filename() {
    constexpr size_t length = 16;
    static const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";

    std::string tmp_s;
    tmp_s.reserve(length + sizeof("_tmp.py"));

    std::srand((uint32_t)(std::time(0) ^ std::rand()));
    for (size_t i = 0; i != length; ++i) {
        tmp_s += alphanum[std::rand() % (sizeof(alphanum) - 1)];
    }
    tmp_s += "_tmp.py";
    
    return tmp_s;
}

ErrorCodes process_file(const std::string &input_filename, const PreprocessorFlags preprocessor_flags) {
    const bool is_verbose_mode = (preprocessor_flags & PreprocessorFlags::verbose) != PreprocessorFlags::no_flags;

    std::ifstream fin(input_filename);
    if (!fin.is_open()) {
        if (is_verbose_mode) {
            std::clog << "Was not able to open " << input_filename << '\n';
        }

        return ErrorCodes::src_file_open_error;
    }

    const std::string &tmp_file_name = gen_random_filename();
    std::ofstream tmp_fout(tmp_file_name, std::ios::out | std::ios::trunc);
    if (!tmp_fout.is_open()) {
        fin.close();
        if (is_verbose_mode) {
            std::clog << "Was not able to open temporary file " << tmp_file_name << '\n';
        };

        return ErrorCodes::tmp_file_open_error;
    }

    ErrorCodes ret_code = process_file_internal(fin, tmp_fout, preprocessor_flags);
    fin.close();
    tmp_fout.close();

    if (fin.bad()) {
        if (is_verbose_mode) {
            std::clog << "Input file stream (src code) got bad bit: 'Error on stream (such as when this function catches an exception thrown by an internal operation).'\n";
        };
        ret_code |= ErrorCodes::src_file_io_error;
        return ret_code;
    }

    if ((preprocessor_flags & PreprocessorFlags::overwrite_file) != PreprocessorFlags::no_flags) {
        if (ret_code != ErrorCodes::no_errors) {
            if (is_verbose_mode) {
                std::clog << "No overwrite made due to errors appeared before\n";
            }
            return ret_code;
        }

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
            if (is_verbose_mode) {
                std::cout << "An error occured while deleting tmp file " << tmp_file_name << '\n';
            }
            ret_code |= ErrorCodes::tmp_file_delete_error;
        }
    }

    return ret_code;
}
