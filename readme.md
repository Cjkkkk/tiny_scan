## Task
有一个 100GB 的文件，里面内容是文本，要求：

1. 找出第一个不重复的词
2. 只允许扫一遍原文件
3. 尽量少的 IO
4. 内存限制 16G


## Build
```
mkdir build
make
# generate permutation of alphabet from (1, length)
python tools/generate_data.py [length] [output]
./build/scan [filename] [map_task_num] [reduce_task_num] [buffer_size] 
```