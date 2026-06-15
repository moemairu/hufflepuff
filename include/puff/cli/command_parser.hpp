#ifndef PUFF_CLI_COMMAND_PARSER_HPP
#define PUFF_CLI_COMMAND_PARSER_HPP

#include <string>
#include <vector>

namespace puff {

/// Represents a parsed command from the CLI.
struct ParsedCommand {
    std::string              command;   ///< Canonical command name.
    std::vector<std::string> args;      ///< Positional arguments after the command.
};

/// Parses and validates CLI arguments for the `puff` executable.
class CommandParser {
public:
    /// Parse argc/argv into a ParsedCommand.
    /// Resolves shorthand aliases (c → compress, x → extract, etc.).
    /// @throws std::runtime_error on invalid or missing arguments.
    static ParsedCommand parse(int argc, char* argv[]);

    /// Print usage information to stdout.
    static void print_usage(const std::string& program_name);

private:
    /// Resolve shorthand to full command name.
    static std::string resolve_alias(const std::string& cmd);

    /// Minimum number of positional arguments required per command.
    static int required_args(const std::string& cmd);
};

}  // namespace puff

#endif  // PUFF_CLI_COMMAND_PARSER_HPP
