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
unsigned int num_nodes=0;

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

int main(void){
    // root
    root = (NODE *)malloc(sizeof(NODE*));
    IPTABLE *counter = NULL;
    counter = (IPTABLE *)malloc(129*sizeof(IPTABLE*));
    // init
    for(int i=0;i<=IPV6;i++){
        counter[i].count=0;
    }
    // read the test file
    FILE *fp = fopen("../ipv6/bgptable_ipv6_uniq.txt","r+");
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
        // inc
        num_entries++;

        // create binary trie
        add_node(ipv6,len,nexthop);

        /* debug
        for(int i=0;i<IPV6;i++){
            if(i%16==0)
                puts(" ");
            printf("%d",(ipv6>>(IPV6-i-1) & 0x1));
        }
        printf("\n");
        */
    }

    // Print debug
    /*for(int i=0;i<=IPV6;i++){
        printf("[%d] - %d\n",i,counter[i].count);
    }*/

    fclose(fp);
    return 0;
}
