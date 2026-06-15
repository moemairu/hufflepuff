# CLI Usage Guide

Hufflepuff operates entirely through the command line.

## Commands

### `compress` (or `c`)
Compresses a single file or an entire directory into a `.puff` archive.
```bash
puff compress my_file.txt
puff c my_folder/
```
The output file is automatically named by appending `.puff` (e.g., `my_file.puff` or `my_folder.puff`).

### `extract` (or `x`)
Extracts the contents of a `.puff` archive.
```bash
puff extract my_archive.puff
puff x my_archive.puff
```
- If the archive contains a single file, it is extracted to the current directory.
- If the archive contains multiple files/directories, a folder named after the archive is created, and contents are extracted there.

### `list` (or `ls`)
Prints a list of all files and directories stored inside the archive without extracting them.
```bash
puff list my_archive.puff
puff ls my_archive.puff
```

### `info` (or `i`)
Displays metadata and statistics about the archive, such as file count, compression ratio, and creation time.
```bash
puff info my_archive.puff
puff i my_archive.puff
```

### `benchmark` (or `b`)
Runs a compression analysis on a single file. It compresses and decompresses the file in memory, measuring the exact execution time, resulting compression ratio, Shannon entropy, and symbol distribution.
```bash
puff benchmark data.bin
puff b data.bin
```

### `help`
Prints the help and usage menu.
```bash
puff help
```
