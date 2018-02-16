#ifndef HUFFMAN_HPP
#define HUFFMAN_HPP

#include <string>

struct Node
{
    char ch;
    int parent;
    int child_zero, child_one;
    bool branch;
};

void compress(const std::string &path_to_input_file,
              const std::string &path_to_output_file);
void decompress(const std::string &path_to_input_file,
              const std::string &path_to_output_file);

#endif // HUFFMAN_HPP
