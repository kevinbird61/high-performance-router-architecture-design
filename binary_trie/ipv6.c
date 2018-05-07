#include <time.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#define IPV4 32
#define IPV6 128

typedef unsigned __int128 uint128_t;

typedef struct node {
    unsigned long port;
    struct node *left,*right;
} NODE;

typedef struct iptable {
    unsigned int count;
} IPTABLE;

typedef struct entry {
    uint128_t ipv6;
    unsigned long len;
    unsigned long port;
} ENTRY;

// global variables
NODE *root;
unsigned int num_entries=0;
unsigned int num_nodes=0,N=0,Layer=0;
unsigned long long int *clocker;
clock_t begin,end;

inline unsigned long long int rdtsc()
{
	unsigned long long int x;
	asm   volatile ("rdtsc" : "=A" (x));
	return x;
}

int hex2int(char c){
    if(c>=48 && c<=57)
        return c-'0';
    else if(c>=97){
        return c-'a'+10;
    }
    else if(c>=65 && c<=90){
        return c-'A'+10;
    }
}

NODE *create_empty_node(){
    NODE *temp;
    temp=(NODE *)malloc(sizeof(NODE));
    temp->right = NULL;
    temp->left = NULL;
    temp->port = 256; // default port 
    return temp;
}

void add_node(uint128_t ip, unsigned long len, unsigned long nexthop){
    // start from root
    NODE *ptr = root;

    for(int i=0;i<len;i++){
        if(ip&(1<<(128-i))){
            // if not create, then create default for it
            if(ptr->right==NULL)
                ptr->right=create_empty_node();
            // and move to right branch
            ptr=ptr->right;
            // if goes to end, and this port is default, then set the nexthop for it
            if((i==len-1)&&(ptr->port==256))
                ptr->port=nexthop;
        }
        else{
            if(ptr->left==NULL)
                ptr->left=create_empty_node();
            ptr=ptr->left;
            if((i==len-1)&&(ptr->port==256))
                ptr->port=nexthop;
        }
    }
}

void read_ip_info(char *str,uint128_t *ipv6,unsigned long *len,unsigned long *nexthop){
    // read from str, and then store them into the other
    // ip
    // len
    // nexthop
    const char tok[]=":";
    // ipv6 = 128 bits
    char *buf;
    // each piece will take 16 bits
    int index=0;
    unsigned long pieces[8]={0,0,0,0,0,0,0,0};

    buf = strtok(str,tok);
    while(buf!=NULL){
        // store into pieces
        if(strlen(buf)==0)
            pieces[index]=0;
        else{
            // convert hex to dec
            for(int i=0;i<4;i++){
                pieces[index]+=(hex2int(buf[i])*pow(16,3-i));
            }
        }
        index++;
        // cut
        buf = strtok(NULL,tok);
    }

    // increase 'len' size
    *len=(index*16);
    // pick nexthop
    *nexthop=pieces[index-1];

    // After cutting, restore then into ipv6
    for(int i=0;i<7;i++){
        *ipv6+=pieces[i];
        *ipv6<<=16;
    }

}

void search(uint128_t ipv6){
    NODE *current=root,*temp=NULL;
    for(int i=127;i>=(-1);i--){
        if(current==NULL)
            break;
        if(current->port!=256)
            temp=current;
        if(ipv6&(1<<i)){
            current=current->right;  
        }
        else{
            current=current->left;
        }
    }
}

void count_node(NODE *r){
	if(r==NULL)
		return;
	count_node(r->left);
	N++;
	count_node(r->right);
}

void detect_layer(NODE *r,int l){
	if(r==NULL)
		return;
	if(l>Layer)
		Layer=l;
	detect_layer(r->left,l++);
	detect_layer(r->right,l++);
}

void CountClock()
{
	unsigned int i;
	unsigned int* NumCntClock = (unsigned int* )malloc(50 * sizeof(unsigned int ));
	for(i = 0; i < 50; i++) NumCntClock[i] = 0;
	unsigned long long MinClock = 10000000, MaxClock = 0;
	for(i = 0; i < num_entries; i++)
	{
		if(clocker[i] > MaxClock) MaxClock = clocker[i];
		if(clocker[i] < MinClock) MinClock = clocker[i];
		if(clocker[i] / 100 < 50) NumCntClock[clocker[i] / 100]++;
		else NumCntClock[49]++;
	}
	printf("(MaxClock, MinClock) = (%5llu, %5llu)\n", MaxClock, MinClock);
	
	for(i = 0; i < 50; i++)
	{
		printf("%d : %d\n", (i + 1) * 100, NumCntClock[i]);
	}
	return;
}

int main(int argc,char *argv[]){
    // root
    root = (NODE *)malloc(sizeof(NODE*));
    IPTABLE *counter = NULL;
    counter = (IPTABLE *)malloc(129*sizeof(IPTABLE*));
    // init
    for(int i=0;i<=IPV6;i++){
        counter[i].count=0;
    }
    // read the test file
    FILE *fp = fopen(argv[1],"r+");
    if(!fp){
        fprintf(stderr,"read file error\n");
        exit(1);
    }

    // processing
    char line[1024];
    num_entries=0;
    while(fgets(line,1024,fp)!=NULL){
        num_entries++;
    }
    // printf("%d\n",num_entries);
    // allocated table and query 
    // table -> ip, len, nexthop
    // query -> ip (use to test the searching performance)
    ENTRY *table = (ENTRY *)malloc(num_entries*sizeof(ENTRY));
    uint128_t *query = (uint128_t *)malloc(num_entries*sizeof(uint128_t));
    clocker=(unsigned long long int *)malloc(num_entries*sizeof(unsigned long long int));
    num_entries=0;
    // move fp back to beginning pos
    rewind(fp);
    while(fgets(line,1024,fp)!=NULL){
        //printf("%s\n",line);
        char *value = strtok(line,"/");
        char *key = strtok(NULL,"/");
        // TODO: implement insert method, to record the value into this block
        counter[atoi(key)].count++;
        // printf("%s, %s\n",value,key);
        uint128_t ipv6=0;
        unsigned long len;
        unsigned long nexthop;

        // reading source and store them into: 
        // ipv6, len, nexthop
        read_ip_info(value,&ipv6,&len,&nexthop);

        // fill info
        query[num_entries] = ipv6;
        table[num_entries].ipv6 = ipv6;
        table[num_entries].port = nexthop;
        table[num_entries].len = len;
        clocker[num_entries]=10000000;
        // inc
        num_entries++;

        // create binary trie, and count the time

        /* debug
        for(int i=0;i<IPV6;i++){
            if(i%16==0)
                puts(" ");
            printf("%d",(ipv6>>(IPV6-i-1) & 0x1));
        }
        printf("\n");
        */
    }

    // create node 
    begin = clock();
    for(int i=0;i<num_entries;i++){
        add_node(table[i].ipv6,table[i].len,table[i].port);
    }
    end = clock();

    //printf("%ld, %ld\n",begin,end);
    // Print info 
    printf("Total entries: %d\n",num_entries);
    printf("Total inserting time: %lf (sec)\n",(double)(end-begin)/(CLOCKS_PER_SEC));
    
    // searching
    begin = clock();
    for(int i=0;i<num_entries;i++){
        search(query[i]);
    }
    end = clock();

    //printf("%ld, %ld\n",begin,end);
    // Print info 
    printf("Total searching queries: %d\n",num_entries);
    printf("Total searching time: %lf (sec)\n",(double)(end-begin)/(CLOCKS_PER_SEC));
    
    count_node(root);
    printf("Total nodes(in binary trie): %d\n",N);
    detect_layer(root,1);
    printf("Maximum Layer: %d\n",Layer);

    // 
    for(int j=0;j<100;j++){
        for(int i=0;i<num_entries;i++){
            begin=rdtsc();
            search(query[i]);
            end=rdtsc();
            if(clocker[i]>(end-begin))
                clocker[i]=(end-begin);
        }
    }
    CountClock();

    // Print debug
    /*for(int i=0;i<=IPV6;i++){
        printf("[%d] - %d\n",i,counter[i].count);
    }*/

    fclose(fp);
    return 0;
}
