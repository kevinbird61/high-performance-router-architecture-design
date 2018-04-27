#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define IPV4 32
#define IPV6 128

typedef struct node {
    int id;
    struct node *left,*right;
} NODE;

typedef struct iptable {
    int count;
} IPTABLE;

int main(void){
    // root
    NODE *root = NULL;
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
    while(fgets(line,1024,fp)!=NULL){
        //printf("%s\n",line);
        char *value = strtok(line,"/");
        char *key = strtok(NULL,"/");
        // TODO: implement insert method, to record the value into this block
        counter[atoi(key)].count++;
        // printf("%s, %s\n",value,key);
    }

    // Print debug
    for(int i=0;i<=IPV6;i++){
        printf("[%d] - %d\n",i,counter[i].count);
    }

    // fclose(fp);
    return 0;
}
