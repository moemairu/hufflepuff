#include "puff/cli/command_parser.hpp"

#include <iostream>
#include <stdexcept>

namespace puff {

std::string CommandParser::resolve_alias(const std::string& cmd) {
    if (cmd == "c")  return "compress";
    if (cmd == "x")  return "extract";
    if (cmd == "ls") return "list";
    if (cmd == "i")  return "info";
    if (cmd == "b")  return "benchmark";
    return cmd;
}

int CommandParser::required_args(const std::string& cmd) {
    if (cmd == "compress")  return 1;
    if (cmd == "extract")   return 1;
    if (cmd == "list")      return 1;
    if (cmd == "info")      return 1;
    if (cmd == "benchmark") return 1;
    if (cmd == "help")      return 0;
    return 0;
}

ParsedCommand CommandParser::parse(int argc, char* argv[]) {
    if (argc < 2) {
        throw std::runtime_error("No command specified. Use 'puff help' for usage.");
    }

    ParsedCommand result;
    result.command = resolve_alias(argv[1]);

    // Validate known commands.
    if (result.command != "compress" &&
        result.command != "extract" &&
        result.command != "list" &&
        result.command != "info" &&
        result.command != "benchmark" &&
        result.command != "help") {
        throw std::runtime_error("Unknown command: " + std::string(argv[1]) +
                                 ". Use 'puff help' for usage.");
    }

    // Collect positional arguments.
    for (int i = 2; i < argc; ++i) {
        result.args.emplace_back(argv[i]);
    }

    // Validate argument count.
    int needed = required_args(result.command);
    if (static_cast<int>(result.args.size()) < needed) {
        throw std::runtime_error("Command '" + result.command +
                                 "' requires at least " + std::to_string(needed) +
                                 " argument(s). Use 'puff help' for usage.");
    }

    return result;
}

void CommandParser::print_usage(const std::string& program_name) {
    std::cout <<
R"(Hufflepuff — Archive & Compression Utility powered by Huffman Coding

Usage:
  )" << program_name << R"( <command> [arguments]

Commands:
  compress <path>       Compress a file or directory into a .puff archive
  extract  <archive>    Extract a .puff archive
  list     <archive>    List contents of a .puff archive
  info     <archive>    Display archive metadata and statistics
  benchmark <file>      Run compression benchmark on a file
  help                  Show this help message

Shorthand:
  c   →  compress
  x   →  extract
  ls  →  list
  i   →  info
  b   →  benchmark

Examples:
  )" << program_name << R"( compress notes.txt          → notes.puff
  )" << program_name << R"( compress project/           → project.puff
  )" << program_name << R"( extract notes.puff
  )" << program_name << R"( list project.puff
  )" << program_name << R"( info project.puff
  )" << program_name << R"( benchmark large.log
)";
}

}  // namespace puff
