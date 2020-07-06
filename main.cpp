#include <iostream>
#include <fstream>
#include <unordered_map>
#include <vector>
#include <memory>
#include <string> 
#include <limits>
#include <chrono>
#include "./utils.hpp"

typedef std::chrono::high_resolution_clock Clock;

class TaskConfig {
public:
    TaskConfig(int mn, int rn, int bs): map_task_num(mn), reduce_task_num(rn), buffer_size(bs){};
    const int map_task_num;
    const int reduce_task_num;
    const int buffer_size;
};

void MapStage(TaskConfig& config, std::ifstream& input_file) {
    auto buffer = std::unique_ptr<char> (new char [config.buffer_size]);
    std::vector<std::ofstream> partions;
    
    for ( int i = 0 ; i < config.reduce_task_num ; i ++ ) {
        partions.push_back(std::ofstream("mr_" + std::to_string(i), std::ofstream::out));
    }

    for ( int i = 0 ; i < config.map_task_num; i ++ ) {
        std::unordered_map<std::string, int32_t> dict;
        input_file.read (buffer.get(), config.buffer_size);
        std::string str_buf(buffer.get());
        auto start_pos = 0;
        auto end_pos = str_buf.find("\n", start_pos);
        while(end_pos != std::string::npos) {
            auto key = str_buf.substr(start_pos, end_pos - start_pos);
            auto iter = dict.find(key);
            if ( iter == dict.end() ) {
                dict.insert({key, start_pos});
            } else {
                iter->second = -1;
            }
            start_pos = end_pos + 1;
            end_pos = str_buf.find("\n", start_pos);
        }

        for (std::pair<std::string, int32_t> element : dict) {
            if (element.second != -1 ) {
                int index = GetHash(element.first) % config.reduce_task_num;
                partions[index] << element.first << " " << element.second << "\n";
            }
        }

    }
    for ( int i = 0 ; i < config.reduce_task_num ; i ++ ) {
        partions[i].close();
    }
}

void ReduceStage(TaskConfig& config) {
    std::vector<std::ifstream> partions;
    int min_pos = std::numeric_limits<int>::max();
    std::string word;

    for ( int i = 0 ; i < config.reduce_task_num ; i ++ ) {
        partions.push_back(std::ifstream("mr_" + std::to_string(i), std::ifstream::in));
    }

    for ( int i = 0 ; i < config.reduce_task_num; i ++ ) {
        int size = GetParitionSize(partions[i]);
        auto buffer = std::unique_ptr<char> (new char [size]);
        partions[i].read(buffer.get(), size);
        std::string str_buf(buffer.get());
        std::unordered_map<std::string, int32_t> dict;

        auto start_pos = 0;
        auto end_pos = str_buf.find("\n", start_pos);
        while(end_pos != std::string::npos) {
            auto line = str_buf.substr(start_pos, end_pos - start_pos);
            auto blank_pos = line.find(" ");
            // if (blank_pos == std::string::npos) {
            //     std::cout << "err: no blank found in " << line << " " << i << "\n";
            // }
            auto key = line.substr(0, blank_pos);
            std::string::size_type sz;
            auto value = std::stoi(line.substr(blank_pos + 1, line.length() - blank_pos - 1), &sz);
            auto iter = dict.find(key);
            if ( iter == dict.end() ) {
                dict.insert({key, value});
            } else {
                iter->second = -1;
            }
            start_pos = end_pos + 1;
            end_pos = str_buf.find("\n", start_pos);
        }

        for (std::pair<std::string, int32_t> element : dict) {
            if (element.second != -1 && element.second < min_pos) {
                min_pos = element.second;
                word = element.first;
            }
        }
    }   
    std::cout << min_pos << " " << word << "\n";
    for ( int i = 0 ; i < config.reduce_task_num ; i ++ ) {
        partions[i].close();
    }
}


int main(int argc, char** argv) {
    TaskConfig config(5, 5, 100000);
    std::ifstream input_file("test.txt", std::ifstream::in);
    auto t1 = Clock::now();
    MapStage(config, input_file);
    ReduceStage(config);
    auto t2 = Clock::now();
    std::cout << "Time: " 
              << std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count() * 1e-9
              << " seconds" << std::endl;
    input_file.close();
    return 0;
}