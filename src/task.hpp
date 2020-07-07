#ifndef TASK_HPP
#define TASK_HPP

#include <limits>
#include <vector>

#define max_seq std::numeric_limits<uint64_t>::max()

class TaskConfig {
public:
    TaskConfig(uint32_t mn, uint32_t rn, uint32_t bs, std::vector<uint64_t> ss): map_task_num(mn), reduce_task_num(rn), buffer_size(bs), slice_size(ss){};
    const uint32_t map_task_num;
    const uint32_t reduce_task_num;
    const std::vector<uint64_t> slice_size;
    const uint32_t buffer_size;
};

void MapStage(TaskConfig& config, std::ifstream& input_file);
void ReduceStage(TaskConfig& config, uint64_t& min_pos, std::string& word);

#endif