#include "puff/archive/archive_validator.hpp"

#include "puff/utils/crc32.hpp"
#include "puff/utils/types.hpp"

#include <cstring>

namespace puff {

thread_local std::string ArchiveValidator::last_error_;

bool ArchiveValidator::validate_magic(std::istream& in) {
    char magic[4]{};
    auto pos = in.tellg();
    if (!in.read(magic, 4)) {
        last_error_ = "Failed to read archive header (file too small)";
        in.clear();
        in.seekg(pos);
        return false;
    }
    in.seekg(pos);

    if (std::memcmp(magic, PUFF_MAGIC, 4) != 0) {
        last_error_ = "Invalid archive format (bad magic header)";
        return false;
    }
    return true;
}

bool ArchiveValidator::validate_version(uint8_t major, uint8_t minor) {
    if (major != PUFF_VERSION_MAJOR) {
        last_error_ = "Unsupported archive version (expected v" +
                      std::to_string(PUFF_VERSION_MAJOR) + ".x, got v" +
                      std::to_string(major) + "." + std::to_string(minor) + ")";
        return false;
    }
    // Minor version differences within the same major are accepted.
    return true;
}

bool ArchiveValidator::validate_crc(const uint8_t* data,
                                    size_t length,
                                    uint32_t expected_crc) {
    uint32_t actual = crc32(data, length);
    if (actual != expected_crc) {
        last_error_ = "Archive data corrupted (CRC-32 mismatch: expected 0x" +
                      ([&] {
                          char buf[9];
                          std::snprintf(buf, sizeof(buf), "%08X", expected_crc);
                          return std::string(buf);
                      })() +
                      ", got 0x" +
                      ([&] {
                          char buf[9];
                          std::snprintf(buf, sizeof(buf), "%08X", actual);
                          return std::string(buf);
                      })() +
                      ")";
        return false;
    }
    return true;
}

std::string ArchiveValidator::last_error() {
    return last_error_;
}

}  // namespace puff
