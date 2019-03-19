/*
 * Copyright © 2019 LambdAurora <aurora42lambda@gmail.com>
 *
 * This file is part of typing_cat.
 *
 * Licensed under the MIT license. For more information,
 * see the LICENSE file.
 */

#include <lambdacommon/system/system.h>
#include <CLI11.hpp>
#include <fstream>
#include <map>
#include <cctype>

namespace fs = lambdacommon::fs;
namespace sys = lambdacommon::system;
namespace term = lambdacommon::terminal;
namespace lstr = lambdacommon::lstring;
using namespace std::rel_ops;

static const std::vector<std::string> __COMMON_LANG_KEYWORDS(
        {
                "if",
                "else",
                "for",
                "while",
                "do",
                "return",
                "this",
                "true",
                "false",
                "continue",
                "break",
                "switch",
                "case",
                "default"
        }
);

static const std::vector<std::string> __COMMON_LANG_TYPES(
        {
                "void",
                "bool",
                "boolean",
                "unsigned",
                "char",
                "byte",
                "short",
                "int",
                "long",
                "float",
                "double",
                "int8_t",
                "int16_t",
                "int32_t",
                "int64_t",
                "uint8_t",
                "uint16_t",
                "uint32_t",
                "uint64_t",
                "intmax_t",
                "uintmax_t",
                "size_t",
                "string",
                "String"
        }
);

template<class T>
std::vector<T> merge_vectors(const std::vector<T>& base, const std::vector<T>& other) {
    std::vector<T> result;
    result.reserve(base.size() + other.size());
    for (const auto& elem : base)
        result.push_back(elem);
    for (const auto& elem : other)
        result.push_back(elem);
    return result;
}

bool begin_with(const std::string& str, const std::string& prefix) {
    return lstr::starts_with(lstr::replace_all(lstr::replace_all(str, "\t", ""), " ", ""), prefix);
}

class Language {
private:
    std::string name;
    std::string comment_line_prefix;
    std::vector<std::string> keywords;

public:
    Language() {}

    Language(std::string name, std::string comment_line_prefix, std::vector<std::string> keywords) : name(name),
                                                                                                     comment_line_prefix(std::move(comment_line_prefix)),
                                                                                                     keywords(std::move(keywords)) {

    }

    const std::string& get_name() const {
        return this->name;
    }

    const std::string& get_comment_line_prefix() const {
        return this->comment_line_prefix;
    }

    const std::vector<std::string>& get_keywords() const {
        return this->keywords;
    }

    bool operator==(const Language& other) const {
        return this->name == other.name && this->comment_line_prefix == other.comment_line_prefix && this->keywords == other.keywords;
    }

    bool operator<(const Language& other) const {
        return std::tie(this->name, this->comment_line_prefix, this->keywords) < std::tie(other.name, other.comment_line_prefix, other.keywords);
    }
};

std::pair<bool, std::string> has_lang(const std::map<std::string, Language> langs, const std::string& lang_name) {
    for (auto const &[name, lang] : langs) {
        if (name.find(lang_name) != std::string::npos)
            return {true, name};
    }
    return {false, ""};
}

char get_char_at(const std::string& line, size_t index) {
    if (index >= line.length())
        return 0;
    return line[index];
}

int main(int argc, char** argv) {
    term::setup();
    fs::path file;
    lambdacommon::utime_t delay = 0;
    bool line_numbers = false;

    //==============================================================================================================================================================================
    // Arguments reading.
    CLI::App app{"A cat \"clone\" but with a typing effect."};
    app.add_option("file", file, "The input file.");
    app.add_option("-t,--time", delay, "Delay before printing down the next character in milliseconds.");
    app.add_flag("-n,--lines", line_numbers, "Print line numbers.");

    CLI11_PARSE(app, argc, argv);

    if (file.empty() || !file.exists()) {
        std::cout << term::LIGHT_RED << "Please specify a valid path to a file to read." << term::RESET << std::endl;
        return EXIT_FAILURE;
    }

    // The real fun begins...
    term::clear();
    term::set_cursor_position(0, 0);
    term::set_title("> " + file.get_filename().to_string());
    auto lang_ext = lstr::replace_all(file.get_extension().to_string(), ".", "");
    std::ifstream file_content(file);

    //==============================================================================================================================================================================
    // Languages
    std::map<std::string, Language> langs;
    langs["c/cpp/cxx/h/hpp"] = {"c/cpp", "//", merge_vectors(
            {"auto", "catch", "class", "const", "delete", "explicit", "extern", "inline", "namespace", "new", "noexcept", "nullptr", "operator", "private", "protected", "public",
             "sizeof", "static", "static_cast", "struct", "template", "throw", "try", "typedef", "typename", "union", "using"},
            __COMMON_LANG_KEYWORDS)};
    langs["cmake"] = {"cmake", "#", {}};
    langs["java"] = {"java", "//",
                     merge_vectors({"catch", "class", "final", "import", "new", "null", "package", "private", "protected", "public", "static", "throw", "throws", "try", "var"},
                                   __COMMON_LANG_KEYWORDS)};
    langs["js"] = {"js", "//",
                   merge_vectors(
                           {"catch", "class", "const", "delete", "exports", "function", "import", "let", "module", "new", "null", "package", "private", "protected", "public",
                            "static", "throw", "throws", "try", "undefined", "var"},
                           __COMMON_LANG_KEYWORDS)};

    char ch;
    std::optional<Language> lang = std::nullopt;
    auto hasl = has_lang(langs, lang_ext);
    if (hasl.first)
        lang = langs[hasl.second];

    //==============================================================================================================================================================================
    // Read the file.
    std::vector<std::string> lines;
    {
        std::string line;
        while (file_content >> std::noskipws >> ch) {
            if (ch == '\n') {
                lines.push_back(line);
                line = "";
            } else
                line += ch;
        }
        lines.push_back(line);
    }

    //==============================================================================================================================================================================
    // Parsing and printing.
    bool is_comment_block = false;
    size_t line_number = 1;
    auto total_lines = lines.size();
    bool skip_whitespaces = true;
    for (const std::string& line : lines) {
        // Display the line numbers if the option is on.
        if (line_numbers) {
            auto number_length = std::to_string(line_number).length();
            auto total_number_length = std::to_string(total_lines).length();
            std::cout << term::DARK_GRAY << '#' << line_number;
            // Alignment
            for (size_t i = 0; i < total_number_length - number_length; i++)
                std::cout << ' ';
            std::cout << " | " << term::RESET;
            line_number++;
        }

        // Recognization of comments, keywords and types in the line.
        bool is_comment = false;
        std::map<size_t, size_t> keywords_index;
        std::map<size_t, size_t> types_index;
        if (lang) {
            if (begin_with(line, lang->get_comment_line_prefix()) || is_comment_block) {
                is_comment = true;
                std::cout << term::LIGHT_CYAN;
            } else if (lang->get_name() == "c/cpp" && begin_with(line, "#")) {
                is_comment = true;
                std::cout << term::LIGHT_YELLOW;
            } else {
                std::string temp = line;
                for (const auto& keyword : lang->get_keywords()) {
                    if (keyword.empty())
                        continue;
                    size_t index = temp.find(keyword);
                    size_t offset = 0;
                    while (index != std::string::npos) {
                        temp = temp.substr(index + keyword.length() - 1);
                        keywords_index[index + offset] = index + keyword.length() + offset;
                        offset += index + keyword.length() - 1;
                        index = temp.find(keyword);
                    }
                    temp = line;
                }
                for (const auto& types : __COMMON_LANG_TYPES) {
                    if (types.empty())
                        continue;
                    size_t index = temp.find(types);
                    size_t offset = 0;
                    while (index != std::string::npos) {
                        temp = temp.substr(index + types.length() - 1);
                        types_index[index + offset] = index + types.length() + offset;
                        offset += index + types.length() - 1;
                        index = temp.find(types);
                    }
                    temp = line;
                }
            }
        }

        // Display character after character...
        auto end_coloring = static_cast<size_t>(-1);
        bool str_coloring = false;
        bool is_last_number = false;
        char last_ch = 0;
        char next_ch = 0;
        for (size_t i = 0; i < line.length(); i++) {
            ch = line[i];
            if (i + 1 < line.length())
                next_ch = line[i + 1];
            else next_ch = 0;

            if (lang) {
                if (!is_comment_block && !is_comment && !str_coloring && ch == '/') {
                    if (next_ch == '*') {
                        is_comment_block = true;
                        std::cout << term::LIGHT_CYAN << ch;
                        continue;
                    } else if (next_ch == '/') {
                        is_comment = true;
                        std::cout << term::LIGHT_CYAN << ch;
                        continue;
                    }
                } else if (is_comment_block && ch == '/' && last_ch == '*') {
                    is_comment_block = false;
                    std::cout << ch << term::RESET;
                    continue;
                }

                if (!is_comment && !is_comment_block && !str_coloring) {
                    if ((ch == '"' || ch == '\'') && last_ch != '\\') {
                        str_coloring = true;
                        std::cout << term::LIGHT_GREEN << ch;
                        is_last_number = false;
                    } else if (std::isdigit(ch) && ((std::isdigit(last_ch) && is_last_number) || last_ch == '.' || last_ch == ' ' || last_ch == '-' || last_ch == '(') &&
                               end_coloring == static_cast<size_t>(-1)) {
                        std::cout << term::YELLOW << ch << term::RESET;
                        is_last_number = true;
                    } else if (keywords_index.count(i) && !(std::isalpha(last_ch) || std::isalpha(get_char_at(line, keywords_index[i])))) {
                        end_coloring = keywords_index[i];
                        std::cout << term::LIGHT_BLUE << ch;
                        is_last_number = false;
                    } else if (types_index.count(i) && !(std::isalpha(last_ch) || std::isalpha(get_char_at(line, types_index[i])))) {
                        end_coloring = types_index[i];
                        std::cout << std::vector<term::TermFormatting>{term::LIGHT_MAGENTA, term::BOLD} << ch;
                        is_last_number = false;
                    } else if (i == end_coloring) {
                        end_coloring = static_cast<size_t>(-1);
                        std::cout << term::RESET << ch;
                        is_last_number = false;
                    } else {
                        std::cout << ch;
                        is_last_number = false;
                    }
                } else {
                    if (str_coloring && (ch == '"' || ch == '\'') && last_ch != '\\') {
                        str_coloring = false;
                        std::cout << ch << term::RESET;
                        is_last_number = false;
                    } else {
                        std::cout << ch;
                        is_last_number = false;
                    }
                }
            } else // If the language is not recognized just print out the character.
                std::cout << ch;
            std::cout << std::flush;
            if ((ch == ' ' || ch == '\t') && skip_whitespaces) {
                if (next_ch != ' ' && next_ch != '\t')
                    skip_whitespaces = false;
            } else
                sys::sleep(delay);
            last_ch = ch;
        }
        std::cout << term::RESET << std::endl;
        skip_whitespaces = true;
    }
    return 0;
}
