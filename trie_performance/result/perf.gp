#!/usr/bin/gnuplot
set macros
BIN="statistic/binary_trie.txt"
MSPT="statistic/mspt.txt"
TREE="statistic/tree_bitmap.txt"

set title "Searching Time Distribution"
set xlabel "Counter"
set ylabel "CPU Clock usage"
set terminal png font "Times_New_Roman, 12"
set output "searching_time_perf.png"

set xtics rotate by 45 right 
set key right 

plot \
BIN using 1:2 with linespoints linewidth 2 lt rgb "#FF0000" title "BinaryTrie",\
MSPT using 1:2 with linespoints linewidth 2 lt rgb "#00FF00" title "MSPT",\
TREE using 1:2 with linespoints linewidth 2 lt rgb "#0000FF" title "TreeBitmap"