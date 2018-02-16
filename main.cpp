#include "huffman.hpp"

int main(int argc, char *argv[])
{
   // setlocale(LC_ALL, "");
    compress("text.txt", "output.txt");
    decompress("output.txt", "decompressed.txt");
    return 0;
}
