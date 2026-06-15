# Hufflepuff Architecture Overview

Hufflepuff is designed with a modular architecture separating the core compression engine, the archive format logic, and the command-line interface.

## Modules

### 1. Huffman Engine (`huffman/`)
The core compression logic. It has no knowledge of files or archives; it purely operates on byte streams.
- `HuffmanTree`: Constructs a min-heap from byte frequencies and generates a deterministic, prefix-free code map.
- `BitWriter` & `BitReader`: Handles packing and unpacking variable-length bits into byte-aligned streams.
- `Encoder`: Computes byte frequencies and transforms raw streams into packed bitstreams.
- `Decoder`: Traverses the Huffman tree bit-by-bit to recover the original bytes.

### 2. Archive Subsystem (`archive/`)
Handles the serialization and deserialization of the `.puff` format.
- `ArchiveWriter`: Orchestrates the compression of files/directories. It uses a two-pass approach:
  1. Compress all files to memory buffers.
  2. Write the sequential archive (Header -> Metadata -> File Table -> Payload -> CRC).
- `ArchiveReader`: Parses the archive structure and handles extraction by extracting each file's specific frequency table and bitstream.
- `ArchiveValidator`: Ensures integrity using magic bytes, version checks, and a custom CRC-32 implementation.

### 3. Filesystem Layer (`filesystem/`)
Abstracts cross-platform path handling and folder traversal.
- `FileCollector`: Recursively walks directories, tracking files and empty directories while skipping symlinks.
- `PathUtils`: Normalizes paths (forward slashes) and prevents path traversal attacks (`../`) during extraction.

### 4. Benchmark Engine (`benchmark/`)
A standalone module that runs an analysis on a single file, measuring compression ratios, execution time, and mathematical properties like Shannon entropy.

### 5. CLI Layer (`cli/`)
Parses user arguments and routes them to the appropriate subsystem commands.

## Data Flow (Compression)
1. `FileCollector` maps out the target path.
2. `ArchiveWriter` processes each file:
   - `Encoder` computes frequencies.
   - `HuffmanTree` generates codes.
   - `Encoder` packs bits into a buffer.
3. `ArchiveWriter` lays out the binary structure and writes to disk, appending a CRC-32 checksum.
