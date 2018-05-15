all: ipv6 ipv4

ipv6: binary_trie_ipv6.c
	#gcc -o ipv6.out ipv6.c -lm 
	g++ -o ipv6.out binary_trie_ipv6.c -lm

ipv4: ipv4.c
	# gcc -o ipv4.out ipv4.c -lm -std=gnu99 -O1
	g++ -o ipv4.out ipv4.c 

run:
	./ipv4.out ../testdata/ipv4/400k_IP_build.txt
	./ipv6.out ../testdata/ipv6/bgptable.txt

clean:
	rm ipv4.out ipv6.out
