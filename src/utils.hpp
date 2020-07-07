#ifndef UTILS_HPP
#define UTILS_HPP

#include <iostream>
#include <fstream>

int SplitFile(std::ifstream& file, std::vector<uint64_t>& slice_size);
int GetFileSize(std::ifstream& partion);
int GetHash (std::string& word);

#endif