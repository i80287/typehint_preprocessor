#include <iostream> // std::clog
#include <fstream>  // std::ifstream

#include <preprocessor.hpp>
#include <flags_parser.hpp>

using preprocessor_tools::PreprocessorFlags;
using preprocessor_tools::ErrorCodes;
using preprocessor_tools::process_files;
using preprocessor_tools::parse_flags;
using preprocessor_tools::from_error;

int main(int argc, const char ** argv) {
    std::ifstream files_is("files.txt");
    if (files_is.fail()) {
        std::clog << "Was not able to open file with file paths\n";
        return 1;
    }

    std::unordered_set<std::string> filenames;
    std::string filename;
    while (files_is) {
        std::getline(files_is, filename);
        if (!filename.empty()) {
            filenames.insert(filename);
        }
    }
    files_is.close();

    std::unordered_set<std::string> ignored_functions;
    std::ifstream functions_is("ignored_functions.txt");
    if (!functions_is.fail()) {
        std::string func_name;
        while (functions_is) {
            std::getline(functions_is, func_name);
            if (!func_name.empty()) {
                ignored_functions.insert(func_name);
            }
        }
        functions_is.close();
    }

    ErrorCodes ret_code = ErrorCodes::no_errors;
    PreprocessorFlags flags = parse_flags(argc, argv);
    try {
        if (!flags) {
            ret_code = process_files(filenames, ignored_functions);
        } else {
            ret_code = process_files(filenames, ignored_functions, flags);
        }
    } catch(const std::exception& e) {
        std::cerr << "An error occured: " << e.what() << '\n';
    }

    if (!ret_code) {
        return 0;
    }

    std::clog << from_error(ret_code);
    return 1;
}
