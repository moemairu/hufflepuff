# Mathematical Logic of Hufflepuff (Huffman Coding)

This document explains the mathematical foundations of the compression engine used in Hufflepuff, which is based on Huffman Coding and Information Theory.

## 1. Shannon Entropy

In information theory, **Shannon Entropy** ($H$) is a measure of the unpredictability or average information content in a message. For a given file, we can treat each byte (0-255) as a symbol $x$. 

If $p(x)$ is the probability of encountering the symbol $x$ in the file, the Shannon Entropy is defined as:

$$ H = - \sum_{x} p(x) \log_2 p(x) $$

- **$p(x)$** is calculated as the frequency of byte $x$ divided by the total number of bytes.
- The unit of $H$ is **bits per symbol**.
- **Mathematical limit**: $H$ represents the theoretical absolute minimum number of bits needed, on average, to encode a single symbol without any loss of information. No symbol-by-symbol compression algorithm can compress data smaller than its Shannon Entropy.

## 2. Average Code Length

When we assign a variable-length binary code to each symbol, we can calculate the **Average Code Length** ($L$). Let $\ell(x)$ be the length (in bits) of the code assigned to symbol $x$.

$$ L = \sum_{x} p(x) \ell(x) $$

The goal of Huffman Coding is to minimize $L$. A perfectly optimal compression algorithm would achieve $L = H$.

## 3. Compression Efficiency

The efficiency $\eta$ of the compression algorithm compares the theoretical limit ($H$) against the actual achieved average code length ($L$):

$$ \eta = \frac{H}{L} \times 100\% $$

In Hufflepuff's `puff benchmark` command, you can see these exact mathematical metrics calculated for any file.

## 4. Prefix-Free Codes and Kraft's Inequality

Huffman coding generates **prefix-free codes**, meaning no code is a prefix of any other code. For example, if 'A' is encoded as `01`, no other symbol's code can start with `01` (like `010` or `011`).

Mathematically, any valid prefix-free code must satisfy the **Kraft-McMillan Inequality**:

$$ \sum_{x} 2^{-\ell(x)} \leq 1 $$

Because Huffman coding creates a full binary tree (where every internal node has exactly two children), the Kraft-McMillan sum for a Huffman tree is always exactly equal to 1.

## 5. The Huffman Algorithm (Optimality)

Huffman coding uses a greedy algorithm to build an optimal prefix tree:
1. Create a leaf node for each symbol with a weight equal to its frequency.
2. While there is more than one node:
   - Extract the two nodes with the lowest frequencies.
   - Create a new internal node with these two nodes as children.
   - The frequency of the new node is the sum of the children's frequencies.
3. The remaining node is the root of the tree.

**Mathematical Proof of Optimality**: Huffman's algorithm is mathematically proven to produce an optimal prefix code. It ensures that $\ell(x)$ is inversely proportional to $p(x)$—more frequent symbols get shorter codes, minimizing the expected length $L$.

## 6. Deterministic Tie-Breaking (Hufflepuff Implementation)

In a strict mathematical sense, when two nodes have the exact same frequency, choosing either one maintains the optimality of the tree. However, in software, if the encoder and decoder break ties differently, the trees will be structurally different, and decoding will fail.

To ensure absolute determinism, Hufflepuff implements a strict weak ordering mathematically for tie-breaking:
- **Primary condition**: Lowest frequency first.
- **Secondary condition (Tie-breaker)**: Lowest symbol value first.

For internal nodes (which don't have a single symbol), Hufflepuff mathematically defines their "symbol" as the absolute minimum symbol of all leaves within that node's subtree:

$$ S_{internal} = \min(S_{left}, S_{right}) $$

This guarantees that every single node in the tree has a strictly unique identifier, preventing any non-determinism during priority queue operations and ensuring that the mathematical model translates robustly into identical bitstreams across different platforms.
