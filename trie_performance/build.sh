#!/bin/bash

# Step 1 - Compile all the source
gcc -o binary_trie.out ../src/binary_trie/binary_trie_ipv6.c
gcc -o mspt.out ../src/mspt/mspt.c 
gcc -o tree_bitmap.out ../src/tree_bitmap/treebitmap.c

# Step 2 - Run program to generate statistics

# clear the cache 
sync; sudo sh -c  "/bin/echo 3 > /proc/sys/vm/drop_caches"
./binary_trie.out   ../testdata/ipv6/bgptable.txt   ./statistic/binary_trie.txt     ./statistic/binary_trie_mem.txt  > /dev/null 2>&1 

sync; sudo sh -c  "/bin/echo 3 > /proc/sys/vm/drop_caches"
./mspt.out          ../testdata/ipv6/bgptable.txt   ./statistic/mspt.txt            ./statistic/mspt_mem.txt         > /dev/null 2>&1

sync; sudo sh -c  "/bin/echo 3 > /proc/sys/vm/drop_caches"
./tree_bitmap.out   ../testdata/ipv6/bgptable.txt   ./statistic/tree_bitmap.txt     ./statistic/tree_bitmap_mem.txt  > /dev/null 2>&1

# Step 3 - Plot out the result
gnuplot result/perf.gp 
gnuplot result/treebitmap_trie_perf.gp
