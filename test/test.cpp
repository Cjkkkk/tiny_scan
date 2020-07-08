#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "catch.hpp"
#include "task.hpp"
#include "utils.hpp"
#include <vector>
#include <set>
#include <cstdio>

void RemoveMr(TaskConfig& config) {
    for ( int i = 0 ; i < config.map_task_num ; i ++ ) {
        for ( int j = 0 ; j < config.reduce_task_num ; j ++ ) {
            std::string filename = "mr_" + std::to_string(i) + "_" + std::to_string(j);
            std::remove(filename.c_str());
        }
    }
}

TEST_CASE( "Test stage 1", "[split stage]" ) {
    std::ifstream input_file("test.txt", std::ifstream::in);
    std::vector<uint64_t> slice_size(3);
    SplitFile(input_file, slice_size);
    REQUIRE( slice_size[0] == 22 );
    REQUIRE( slice_size[1] == 20 );
    REQUIRE( slice_size[2] == 16 );
}

TEST_CASE( "Test buffer size not commmon factor of slice size" ) {
    // slice size is 58 and buffer size is 11
    std::ifstream input_file("test.txt", std::ifstream::in);
    std::vector<uint64_t> slice_size(1);
    const int buffer_size = 11;

    SplitFile(input_file, slice_size);
    TaskConfig config(1, 1, buffer_size, slice_size);

    MapStage(config, input_file);

    REQUIRE( slice_size[0] == 58 );
    std::ifstream mr("mr_0_0", std::ifstream::in);
    REQUIRE( mr.is_open() );
    std::vector<std::string> expected = {"a","b","c","d","e","f","g","h","i","j"
        ,"k","l","m","n","o","p","q","r","s","t","u","v","w","x","y","z","aa","ab"};
    std::set<std::string> found;
    std::string str;
    while (std::getline(mr, str)) {
        auto pos = str.find(" ");
        REQUIRE( pos != std::string::npos);
        auto word = str.substr(0, pos);
        found.insert(word);
    }

    REQUIRE( found.size() == expected.size());
    for(auto& expected_word: expected) {
        REQUIRE(found.find(expected_word) != found.end());
    }
    RemoveMr(config);
}

TEST_CASE( "Test stage2, stage 3", "[map stage, reduce stage]" ) {
    const int map_task_num = 3;
    const int reduce_task_num = 3;
    const int buffer_size = 10;

    SECTION( "file without repeation" ) {
        std::ifstream input_file("test.txt", std::ifstream::in);
        std::vector<uint64_t> slice_size(3);
        SplitFile(input_file, slice_size);
        TaskConfig config(map_task_num, reduce_task_num, buffer_size, slice_size);
        MapStage(config, input_file);
        std::string word;
        ReduceStage(config, word);
        REQUIRE( word == "a" );
        RemoveMr(config);
    }

    SECTION( "file with repeation" ) {
        std::ifstream input_file("test1.txt", std::ifstream::in);
        std::vector<uint64_t> slice_size(3);
        SplitFile(input_file, slice_size);
        TaskConfig config(map_task_num, reduce_task_num, buffer_size, slice_size);
        MapStage(config, input_file);
        std::string word;
        ReduceStage(config, word);
        REQUIRE( word == "l" );
        RemoveMr(config);
    }
}