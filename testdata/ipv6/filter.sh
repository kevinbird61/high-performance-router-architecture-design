#!/bin/sh
# Use to eliminate the useless entries

# get the ip, without AS number
cat bgptable.txt| awk '{print $1}' > bgptable_ipv6.txt 

# eliminate the duplicated 
awk '!a[$0]++' bgptable_ipv6.txt > bgptable_ipv6_uniq.txt

# Notice: in this case, size changes:
# bgptable.txt -> 45M
# bgptable_ipv6.txt -> 21M
# bgptable_ipv6_uniq.txt -> 986K
