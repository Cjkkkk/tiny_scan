#include <iostream>
#include <fstream>
#include "./utils.hpp"

int GetParitionSize(std::ifstream& partion) {
    std::streampos begin,end;
    begin = partion.tellg();
    partion.seekg (0, std::ios::end);
    end = partion.tellg();
    partion.seekg (0, std::ios::beg);
    return end - begin; 
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