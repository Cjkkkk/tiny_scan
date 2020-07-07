## Task
有一个 100GB 的文件，里面内容是文本，要求：

1. 找出第一个不重复的词
2. 只允许扫一遍原文件
3. 尽量少的 IO
4. 内存限制 16G

# Thoughts
（1）由于待处理数据过大，因此需要将大文件分成若干个小文件（文件数目为map_task_num）读入分别处理。
（2）读入每一个小文件，用哈希表统计其中每一个单词的第一次出现的位置, 如果某一个单词出现两次及以上，设置第一次出现的位置为max_seq。处理完一个文件后遍历哈希表。对每一个单词进行哈希并取模预先设定的桶数目（桶数目为reduce_task_num），根据取模结果将单词以及第一次出现的位置写入对应的文件，这样能保证相同的单词被写入同一个文件中便于后序处理。如果第一次出现的位置为max_seq则不写入。
（3）从上一个阶段产生的若干个文件中重新读出哈希表，并找出其中序号最小的不重复单词。

eg:
假设此时map_task_num = 2, reduce_task_num = 3。
阶段2产生6个文件，命名为mr_{map_num}_{reduce_num}：
(a : 1代表单词a出现的位置为1)
```
mr_0_0 => a : 1, b: 4
mr_0_1 => c : 2
mr_0_2 => e : 5
mr_1_0 => a : 11, b : 10
mr_1_1 => d : 3
mr_1_2 => e : 8, f: 6
```
阶段3读入reduce_num相等的文件进行横向比对得到每一个不重复单词最早出现的位置。
```
mr_0_0 + mr_1_0 => a : 1, b: 4 => a : 1
mr_0_1 + mr_1_1 => c : 2, d: 3 => c : 2
mr_0_2 + mr_1_2 => e : 5, f: 6 => e : 5
```
最后比较得到结果为a。

细节:
阶段1需要注意的是分割的时候不能简单的把每一份小文件的大小设定为 total_size / n (这里n为小文件数目)。考虑这样的情况: 'a\nba\n'直接分割成2份会生成'a\nb'和'a\n'，应该分为'a\n'和'ba\n'。

## Build
```
mkdir build
make
# generate permutation of alphabet from (1, length)
python tools/generate_data.py [length] [output]
./build/scan [filename] [map_task_num] [reduce_task_num] [buffer_size] 
```