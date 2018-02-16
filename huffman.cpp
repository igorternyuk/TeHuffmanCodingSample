#include "huffman.hpp"

#include <vector>
#include <map>
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <locale>
#include <cstdlib>

void compress(const std::string &path_to_input_file,
              const std::string &path_to_output_file)
{
    int weight[0x100];
    for(auto &w: weight)
        w = 0;
    std::ifstream fi(path_to_input_file);

    if(fi.is_open())
    {
        while (!fi.eof()) {
            unsigned char ch;
            fi.read((char*)&ch, sizeof(ch));
            ++weight[ch];
        }
        fi.close();
    }
    //Print nonzero weights - for debugging only
    for(int i = 0; i < 0x100; ++i)
        if(weight[i] > 0)
            std::cout << weight[i] << " " << (char)i << std::endl;

    // In the following multimap the first element of each pair is the weight(frequency)
    // of the character and second one is the character index in the tree

    std::multimap<int,int> sorted_characters;
    std::vector<Node> tree;
    std::map<char, int> char_map; // This maps a character to the index in the tree
    for(int i{0}; i < 0x100; ++i)
        if(weight[i] > 0)
        {
            tree.push_back({(char)i, -1, -1, -1, false});
            sorted_characters.insert({weight[i], tree.size() - 1});
            char_map.insert({(char)i, tree.size() - 1});
        }

    for(auto i: sorted_characters)
    {
        if(i.first > 0)
            std::cout << "w = " << i.first << " ch = " << i.second << std::endl;
    }

    //Here we construct the tree
    while(sorted_characters.size() > 1)
    {
        //We pull two elements with minimal weights out

        int weight0 = sorted_characters.begin()->first;
        int node0 = sorted_characters.begin()->second;
        sorted_characters.erase(sorted_characters.begin());

        int weight1 = sorted_characters.begin()->first;
        int node1 = sorted_characters.begin()->second;
        sorted_characters.erase(sorted_characters.begin());

        //We create the parent node for node0 and node1

        tree.push_back({'\0', -1, node0, node1, false});

        //Here we set the parent and branch for children

        tree[node0].parent = tree.size() - 1;
        tree[node0].branch = false;
        tree[node1].parent = tree.size() - 1;
        tree[node1].branch = true;

        //Here we insert created parent node to the tree

        sorted_characters.insert({weight0 + weight1, tree.size() - 1});
    }

    std::vector<bool> data;
    std::map<char, std::vector<bool>> char_codes; // For caching already calculeted codes
    fi.open(path_to_input_file);
    if(fi.is_open())
    {
        while(!fi.eof())
        {
            unsigned char curr_char;
            fi.read((char*)&curr_char, sizeof(curr_char));
            std::vector<bool> compressed_char;
            if(char_codes.find(curr_char) != char_codes.end())
            {
                compressed_char = char_codes[curr_char];
            }
            else
            {
                auto curr_index = char_map[curr_char];
                auto curr_node = tree[curr_index];
                while(curr_node.parent != -1)
                {
                    compressed_char.push_back(curr_node.branch);
                    curr_node = tree[curr_node.parent];
                }
                std::reverse(compressed_char.begin(), compressed_char.end());
                char_codes[curr_char] = compressed_char;
            }
            data.insert(data.end(), compressed_char.begin(), compressed_char.end());
        }
        fi.close();
    }

     //For debugging only
    for(auto it = char_codes.begin(); it != char_codes.end(); ++it)
    {
        std::cout << it->first << " --> ";
        for(auto bit: it->second)
            std::cout << bit;
        std::cout << std::endl;
    }

   // std::cout << "compressor data.size() = " << data.size() << std::endl;
    std::ofstream fo(path_to_output_file);
    if(fo.is_open())
    {
        //Writes tree to the file
        size_t tree_size = tree.size();
        fo.write((char*)&tree_size, sizeof(tree_size));
        for(auto node: tree)
            fo.write((char*)&node, sizeof(node));
         //Now we write compressed data to the output file
        size_t data_size = data.size();
        fo.write((char*)&data_size, sizeof(data_size));
        for(size_t i{0}; i <= data_size / 8; ++i)
        {
            unsigned char ch {0};
            //Writes bytes from array of bits
            for(int j{0}; j < 8; ++j)
                //if data i-th byte has j-th bit we will set it to ch
                if(data[i * 8 + j])
                    ch |= (1 << j);
            //And now we write received byte to the output file
            fo.write((char*)&ch, sizeof(ch));
        }
        fo.close();
    }
}

void decompress(const std::string &path_to_input_file,
                const std::string &path_to_output_file)
{
    std::ifstream fi(path_to_input_file);
    if(!fi.is_open()) return;
    size_t tree_size;
    std::vector<Node> tree;
    //First of all we read the tree from file
    tree.reserve(tree_size);

    fi.read((char*)&tree_size, sizeof(tree_size));
    for(size_t i{0}; i < tree_size; ++i)
    {
        Node node;
        fi.read((char*)&node, sizeof(node));
        tree.push_back(node);
    }
    size_t data_size_init;
    fi.read((char*)&data_size_init, sizeof(data_size_init));

    std::vector<bool> data;
    while(!fi.eof())
    {
        unsigned char ch;
        fi.read((char*)&ch, sizeof(ch));
        for(int i{0}; i < 8; ++i)
            data.push_back((ch & (1 << i)) != 0);
    }
    fi.close();

    std::ofstream fo(path_to_output_file);
    if(!fo.is_open()) return;

    auto curr_node = tree.size() - 1;
    /*
    auto excessive_bits_num =  data.size() - data_size_init;
    std::cout << "excessive_bits_num = " << excessive_bits_num << std::endl;
    */
    for(size_t i{0}; i < data_size_init - 1; ++i)
    {
        curr_node = data[i] ? tree[curr_node].child_one : tree[curr_node].child_zero;
        if(tree[curr_node].child_one == -1)
        {
            auto ch = tree[curr_node].ch;
            fo << ch;
           // std::cout << ch;
            curr_node = tree.size() - 1;
        }
    }
    fo.close();
}
