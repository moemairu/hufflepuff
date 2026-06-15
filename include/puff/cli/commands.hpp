#ifndef PUFF_CLI_COMMANDS_HPP
#define PUFF_CLI_COMMANDS_HPP

#include <string>

namespace puff {

/// Execute the 'compress' command.
/// @return 0 on success, 1 on error.
int cmd_compress(const std::string& path);

/// Execute the 'extract' command.
/// @return 0 on success, 1 on error.
int cmd_extract(const std::string& archive);

/// Execute the 'list' command.
/// @return 0 on success, 1 on error.
int cmd_list(const std::string& archive);

/// Execute the 'info' command.
/// @return 0 on success, 1 on error.
int cmd_info(const std::string& archive);

/// Execute the 'benchmark' command.
/// @return 0 on success, 1 on error.
int cmd_benchmark(const std::string& file);

/// Execute the 'help' command.
/// @return 0 always.
int cmd_help(const std::string& program_name);

}  // namespace puff

#endif  // PUFF_CLI_COMMANDS_HPP
