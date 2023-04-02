#include <iostream> // std::clog
#include <fstream>  // std::ifstream

#include "preprocessor.hpp"
#include "flags_parser.hpp"

int main(int argc, const char **const argv) {
    const PreprocessorFlags flags = parse_flags(argc, argv);

    std::ifstream files_is("files.txt");
    if (files_is.fail()) {
        std::clog << "Was not able to open file with file paths\n";
        return 1;
    }

    std::unordered_set<std::string> filenames;
    std::string filename;
    while (files_is) {
        std::getline(files_is, filename);
        if (filename.size() != 0) {
            filenames.insert(filename);
        }
    }
    files_is.close();

    std::ifstream functions_is("ignored_functions.txt");
    if (functions_is.fail()) {
        std::clog << "Was not able to open file with functions names\n";
        return 1;
    }

    std::unordered_set<std::string> ignored_functions;
    std::string func_name;
    while (functions_is) {
        std::getline(functions_is, func_name);
        if (func_name.size() != 0) {
            ignored_functions.insert(func_name);
        }
    }
    functions_is.close();

    if (flags == PreprocessorFlags::no_flags) {
        process_files(filenames, ignored_functions);
    } else {
        process_files(filenames, ignored_functions, flags);
    }

    return 0;
}
