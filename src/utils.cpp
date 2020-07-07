#include <iostream>
#include <fstream>
#include <vector>
#include <memory>
#include <cmath>
#include "./utils.hpp"

int GetFileSize(std::ifstream& file) {
    std::streampos begin,end;
    begin = file.tellg();
    file.seekg (0, std::ios::end);
    end = file.tellg();
    file.seekg (0, std::ios::beg);
    return end - begin; 
}

int SplitFile(std::ifstream& file, std::vector<uint64_t>& slice_size) {
    const uint32_t buffer_size = 100;
    uint32_t map_task_num = slice_size.size();
    uint64_t file_size = GetFileSize(file);
    uint64_t equal_slice_size = std::ceil((double)file_size / map_task_num);
    uint64_t accum_slice_size = 0;
    uint32_t offset = 0;
    auto buffer = std::shared_ptr<char> (new char[buffer_size]);
    for ( int i = 0 ; i < map_task_num - 1 ; i ++ ) {
        file.clear();
        file.seekg (equal_slice_size * (i + 1), std::ios::beg);
        file.read(buffer.get(), buffer_size);
        std::streamsize bytes = file.gcount();
        std::string str_buf(buffer.get(), bytes);
        offset = str_buf.find("\n", 0);
        slice_size[i] = (equal_slice_size * (i + 1) + offset + 1 - accum_slice_size);
        accum_slice_size += slice_size[i];
    }
    slice_size[map_task_num - 1] = file_size - accum_slice_size;
    file.clear();
    file.seekg (0, std::ios::beg);
}


int GetHash (std::string& word)
{
    int seed = 131; 
    unsigned long hash = 0;
    for(int i = 0; i < word.length(); i++)
    {
        hash = (hash * seed) + word[i];
    }
    return hash;
}