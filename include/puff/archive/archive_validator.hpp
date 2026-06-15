#ifndef PUFF_ARCHIVE_ARCHIVE_VALIDATOR_HPP
#define PUFF_ARCHIVE_ARCHIVE_VALIDATOR_HPP

#include <cstdint>
#include <istream>
#include <string>

namespace puff {

/// Validates structural properties of a .puff archive.
class ArchiveValidator {
public:
    /// Check whether the first 4 bytes of the stream are the PUFF magic.
    /// Does NOT consume the bytes (seeks back afterwards).
    [[nodiscard]] static bool validate_magic(std::istream& in);

    /// Check whether the given version is supported by this build.
    [[nodiscard]] static bool validate_version(uint8_t major, uint8_t minor);

    /// Verify the CRC-32 of a data buffer against an expected checksum.
    [[nodiscard]] static bool validate_crc(const uint8_t* data,
                                           size_t length,
                                           uint32_t expected_crc);

    /// Return a human-readable error string for the last validation failure.
    [[nodiscard]] static std::string last_error();

private:
    static thread_local std::string last_error_;
};

}  // namespace puff

#endif  // PUFF_ARCHIVE_ARCHIVE_VALIDATOR_HPP
