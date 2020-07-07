#include <iostream>
#include <fstream>
#include <unordered_map>
#include <vector>
#include <memory>
#include <string> 
#include <limits>
#include "utils.hpp"
#include "task.hpp"


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
                    std::cout << e.what() << "\n";
                    std::cout << "error when parse: " << line << "\n";
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