# Archive Format Specification (.puff)

The `.puff` format is a sequential binary format. All multi-byte integers are stored in **little-endian** byte order.

## High-Level Layout

```
[ Magic Header ]
[ Version ]
[ Archive Metadata ]
[ File Table ]
[ Payload (Compressed Files) ]
[ CRC-32 Footer ]
```

## Section 1: Magic Header (4 bytes)
| Size | Content | Description |
|------|---------|-------------|
| 4 | `PUFF` | ASCII magic bytes to identify the file. |

## Section 2: Version (2 bytes)
| Size | Type | Description |
|------|------|-------------|
| 1 | `uint8_t` | Major version (`0x01`) |
| 1 | `uint8_t` | Minor version (`0x00`) |

## Section 3: Archive Metadata (24 bytes)
| Size | Type | Description |
|------|------|-------------|
| 8 | `uint64_t` | Creation timestamp (Unix epoch seconds) |
| 8 | `uint64_t` | Total original size of all files |
| 4 | `uint32_t` | File count |
| 4 | `uint32_t` | Directory count |

## Section 4: File Table (Variable)
Repeated `file_count + dir_count` times.

| Size | Type | Description |
|------|------|-------------|
| 1 | `uint8_t` | Entry type (`0x00` = File, `0x01` = Directory) |
| 2 | `uint16_t` | Length of relative path (`L`) |
| L | `char[]` | Relative path (UTF-8, forward slashes) |
| 8 | `uint64_t` | Original size (0 for directories) |
| 8 | `uint64_t` | Payload offset in bytes relative to the start of the Payload section |
| 8 | `uint64_t` | Compressed size in bytes (Includes frequency table + bitstream) |

## Section 5: Payload
Sequential blocks of compressed file data. Each file's data consists of:

### Frequency Table
| Size | Type | Description |
|------|------|-------------|
| 2 | `uint16_t` | Unique symbols count (`N`) |
| N * 9 | - | Pairs of `uint8_t` (symbol) and `uint64_t` (frequency) |
| 1 | `uint8_t` | Padding bits in the last byte of the bitstream (0-7) |

### Bitstream
Variable length packed bits containing the Huffman-encoded data.

## Section 6: Footer (4 bytes)
| Size | Type | Description |
|------|------|-------------|
| 4 | `uint32_t` | CRC-32 checksum of the entire Payload section |
