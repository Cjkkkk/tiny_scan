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
#include "./utils.hpp"

#define max_seq std::numeric_limits<uint64_t>::max()
typedef std::chrono::high_resolution_clock Clock;

class TaskConfig {
public:
    TaskConfig(uint32_t mn, uint32_t rn, uint32_t bs, std::vector<uint64_t> ss): map_task_num(mn), reduce_task_num(rn), buffer_size(bs), slice_size(ss){};
    const uint32_t map_task_num;
    const uint32_t reduce_task_num;
    const std::vector<uint64_t> slice_size;
    const uint32_t buffer_size;
};

void MapStage(TaskConfig& config, std::ifstream& input_file) {
    auto buffer = std::unique_ptr<char> (new char [config.buffer_size]);
    std::vector<std::vector<std::ofstream>> partions(config.map_task_num);
    
    for ( int i = 0 ; i < config.map_task_num ; i ++ ) {
        for ( int j = 0 ; j < config.reduce_task_num ; j ++ ) {
            partions[i].push_back(std::ofstream("mr_" + std::to_string(i) + "_" + std::to_string(j), std::ofstream::out));
        }
    }

    uint64_t seq = 0;
    for ( int i = 0 ; i < config.map_task_num; i ++ ) {
        uint64_t total_size = config.slice_size[i];
        uint64_t current_size = 0;
        std::unordered_map<std::string, uint64_t> dict;
        while (current_size < total_size)
        {
            uint64_t should_read = current_size + config.buffer_size > total_size ? total_size - current_size : config.buffer_size;
            input_file.read (buffer.get(), should_read);
            current_size += should_read;
            std::string str_buf(buffer.get(), should_read);
            auto start_pos = 0;
            auto end_pos = str_buf.find("\n", start_pos);
            while(end_pos != std::string::npos) {
                auto key = str_buf.substr(start_pos, end_pos - start_pos);
                auto iter = dict.find(key);
                if ( iter == dict.end() ) {
                    dict.insert({key, seq});
                } else {
                    iter->second = max_seq;
                }
                start_pos = end_pos + 1;
                end_pos = str_buf.find("\n", start_pos);
                seq ++;
            }
            
            // go back if buffer's last char is not '\n'
            // eg 'a\nab\n' could be read in as 'a\na' and 'b\n'
            uint64_t go_back = should_read - start_pos;
            input_file.seekg(-go_back, std::ios::cur);
            current_size -= go_back;

        }
        
        for (std::pair<std::string, uint64_t> element : dict) {
            if ( element.second != max_seq ) {
                int index = std::abs(GetHash(element.first)) % config.reduce_task_num;
                partions[i][index] << element.first << " " << element.second << "\n";
            }
        }
    }

    for ( int i = 0 ; i < config.map_task_num ; i ++ ) {
        for ( int j = 0 ; j < config.reduce_task_num ; j ++ ) {
            partions[i][j].close();
        }
    }
}

void ReduceStage(TaskConfig& config, uint64_t& min_pos, std::string& word) {
    std::vector<std::vector<std::ifstream>> partions(config.map_task_num);

    for ( int i = 0 ; i < config.map_task_num ; i ++ ) {
        for ( int j = 0 ; j < config.reduce_task_num ; j ++ ) {
            partions[i].push_back(std::ifstream("mr_" + std::to_string(i) + "_" + std::to_string(j), std::ifstream::in));
        }
    }

    for ( int i = 0 ; i < config.reduce_task_num; i ++ ) {
        std::unordered_map<std::string, uint64_t> dict;
        for ( int j = 0 ; j < config.map_task_num; j ++ ) {
            int size = GetFileSize(partions[j][i]);
            auto buffer = std::unique_ptr<char> (new char [size]);
            partions[j][i].read(buffer.get(), size);
            std::streamsize bytes = partions[j][i].gcount();
            std::string str_buf(buffer.get(), bytes);
            
            auto start_pos = 0;
            auto end_pos = str_buf.find("\n", start_pos);
            while(end_pos != std::string::npos) {
                auto line = str_buf.substr(start_pos, end_pos - start_pos);
                auto blank_pos = line.find(" ");
                auto word = line.substr(0, blank_pos);
                std::string::size_type sz;
                uint64_t seq = 0;
                try{
                    seq = std::stoull(line.substr(blank_pos + 1, line.length() - blank_pos - 1), &sz);
                }catch(std::exception& e) {
                    std::cout << line << " " << line.substr(blank_pos + 1, line.length() - blank_pos - 1) << "\n";
                }
                auto iter = dict.find(word);
                if ( iter == dict.end() ) {
                    dict.insert({word, seq});
                } else {
                    iter->second = max_seq;
                }
                
                start_pos = end_pos + 1;
                end_pos = str_buf.find("\n", start_pos);
            }
        }
        // check if repeated
        for (std::pair<std::string, uint64_t> element : dict) {
            if (element.second < min_pos) {
                min_pos = element.second;
                word = element.first;
            }
        }
    }

    for ( int i = 0 ; i < config.map_task_num ; i ++ ) {
        for ( int j = 0 ; j < config.reduce_task_num ; j ++ ) {
            partions[i][j].close();
        }
    }
}


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