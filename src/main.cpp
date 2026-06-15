#include "puff/cli/command_parser.hpp"
#include "puff/cli/commands.hpp"

#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    try {
        puff::ParsedCommand cmd = puff::CommandParser::parse(argc, argv);

        if (cmd.command == "compress")
            return puff::cmd_compress(cmd.args[0]);

        if (cmd.command == "extract")
            return puff::cmd_extract(cmd.args[0]);

        if (cmd.command == "list")
            return puff::cmd_list(cmd.args[0]);

        if (cmd.command == "info")
            return puff::cmd_info(cmd.args[0]);

        if (cmd.command == "benchmark")
            return puff::cmd_benchmark(cmd.args[0]);

        if (cmd.command == "help")
            return puff::cmd_help(argv[0]);

        // Should never reach here — CommandParser validates commands.
        std::cerr << "Internal error: unhandled command '" << cmd.command << "'\n";
        return 1;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
}
