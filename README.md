# Hufflepuff

Hufflepuff is a cross-platform command-line archive and compression utility powered entirely by a from-scratch Huffman Coding engine. 

It provides an experience similar to modern utilities (like `tar` or `zip`) but relies purely on custom implementations of Huffman Trees, binary bit-packing, and a bespoke archive format (`.puff`).

## Features

- **Single-File Compression**: Compress individual files efficiently.
- **Directory Archiving**: Recursively compress entire folder structures, preserving empty directories and file hierarchies.
- **Custom Format**: Uses the `.puff` file format with built-in versioning, metadata, and CRC-32 corruption detection.
- **Zero External Dependencies**: Implemented entirely in C++20 using only the standard library. No zlib, miniz, or libarchive.
- **Benchmark Mode**: Built-in analysis tool to measure encoding times, compression ratios, and estimated Shannon entropy.

## Building

```bash
# Configure the build
cmake -B build -DCMAKE_BUILD_TYPE=Release

# Build the project
cmake --build build --config Release
```

The resulting executable `puff` will be placed in the `build/src` directory (or `build/Release` on Windows).

## Quick Start

```bash
# Compress a file
./build/src/puff compress notes.txt

# Compress a directory
./build/src/puff compress project/

# List contents of an archive
./build/src/puff list project.puff

# Extract an archive
./build/src/puff extract project.puff

# Run a compression benchmark
./build/src/puff benchmark notes.txt
```

For more details, see the [CLI Usage Guide](docs/CLI_USAGE.md) or run `puff help`.

## Documentation

- [Architecture Overview](docs/ARCHITECTURE.md)
- [Mathematical Logic of Hufflepuff](docs/MATHEMATICAL_LOGIC.md)
- [Archive Format Specification](docs/ARCHIVE_FORMAT.md)
- [CLI Usage Guide](docs/CLI_USAGE.md)
