#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "catch.hpp"
#include "task.hpp"
#include "utils.hpp"

TEST_CASE( "Test slice size", "[SplitFile]" ) {
    std::ifstream input_file("test.txt", std::ifstream::in);
    std::vector<uint64_t> slice_size(3);
    SplitFile(input_file, slice_size);
    REQUIRE( slice_size[0] == 22 );
    REQUIRE( slice_size[1] == 20 );
    REQUIRE( slice_size[2] == 16 );
}


TEST_CASE( "Test map stage", "[MapStage]" ) {
    std::ifstream input_file("test.txt", std::ifstream::in);
    std::vector<uint64_t> slice_size(3);
    SplitFile(input_file, slice_size);
    TaskConfig config(3, 3, 10, slice_size);
    MapStage(config, input_file);
}


TEST_CASE( "Test reduce stage", "[ReduceStage]" ) {
    
}