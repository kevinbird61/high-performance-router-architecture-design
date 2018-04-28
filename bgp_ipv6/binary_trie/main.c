#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#define IPV4 32
#define IPV6 128

typedef unsigned __int128 uint128_t;

typedef struct node {
    unsigned int port;
    struct node *left,*right;
} NODE;

typedef struct iptable {
    unsigned int count;
} IPTABLE;

typedef struct entry {
    uint128_t ipv6;
    unsigned char len;
    unsigned char port;
} ENTRY;

// global variables
NODE *root;
unsigned int num_entries=0;

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

void read_ip_info(char *str,uint128_t *ipv6,int *len,unsigned int *nexthop){
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
    FILE *fp = fopen("../bgptable_ipv6_uniq.txt","r+");
    if(!fp){
        fprintf(stderr,"read file error\n");
        exit(1);
    }

    // processing
    char line[1024];
    num_entries=0;
    while(fgets(line,1024,fp)!=NULL){
        //printf("%s\n",line);
        char *value = strtok(line,"/");
        char *key = strtok(NULL,"/");
        // TODO: implement insert method, to record the value into this block
        counter[atoi(key)].count++;
        // printf("%s, %s\n",value,key);
        uint128_t ipv6=0;
        int len;
        unsigned int nexthop;
        printf("source: %s\n",value);
        read_ip_info(value,&ipv6,&len,&nexthop);
        //printf("ipv6 size: %d\n",(int)sizeof(ipv6));
        for(int i=0;i<IPV6;i++){
            if(i%16==0)
                puts(" ");
            printf("%d",(ipv6>>(IPV6-i-1) & 0x1));
        }
        printf("\n");
    }

    // Print debug
    /*for(int i=0;i<=IPV6;i++){
        printf("[%d] - %d\n",i,counter[i].count);
    }*/

    // fclose(fp);
    return 0;
}
