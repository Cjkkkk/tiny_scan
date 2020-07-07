#include <iostream>
#include <fstream>
#include <unordered_map>
#include <vector>
#include <memory>
#include <string> 
#include <limits>
#include <chrono>
#include <thread>
#include <cmath>
#include "utils.hpp"
#include "task.hpp"

typedef std::chrono::high_resolution_clock Clock;

int main(int argc, char** argv) {
    if ( argc != 5) {
        std::cerr << "Usage: scan [filename] [map_task_num] [reduce_task_num] [buffer_size]\n";
        exit(0);
    }

    std::ifstream input_file(argv[1], std::ifstream::in);
    uint32_t map_task_num = std::stoi(argv[2]);
    uint32_t reduce_task_num = std::stoi(argv[3]);
    uint64_t buffer_size = std::stoi(argv[4]);
    
    uint64_t min_pos = max_seq;
    std::string word;

    std::cout << "---------------     Job Config     ---------------\n"
              << "map task number: " << map_task_num << "\n"
              << "reduce task number: " << reduce_task_num << "\n"
              << "buffer size: " << buffer_size << "\n";

    std::cout << "--------------- Stage 1: Split Stage ---------------\n";
    auto t1 = Clock::now();
    std::vector<uint64_t> slice_size(map_task_num);
    SplitFile(input_file, slice_size);
    TaskConfig config(map_task_num, reduce_task_num, buffer_size, slice_size);
    auto t2 = Clock::now();
    std::cout << "Split Time: " 
              << std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count() * 1e-9
              << " seconds" << std::endl;

    std::cout << "--------------- Stage 2: Map Stage ---------------\n";
    t1 = Clock::now();
    MapStage(config, input_file);
    t2 = Clock::now();
    std::cout << "Map Time: " 
              << std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count() * 1e-9
              << " seconds" << std::endl;
    
    std::cout << "--------------- Stage 3: Reduce Stage --------------\n";
    t1 = Clock::now();
    ReduceStage(config, min_pos, word);
    t2 = Clock::now();
    std::cout << "Redice Time: " 
              << std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count() * 1e-9
              << " seconds" << std::endl;
    std::cout << "result: " << min_pos << " " << word << "\n";
    input_file.close();
    return 0; 
}