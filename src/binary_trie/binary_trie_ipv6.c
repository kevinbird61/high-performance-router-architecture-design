#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#define NUM_CNT_CLOCK 50

/* IPV6 */
struct ENTRY{
    // separated into 4 uint (32x4=128)
    unsigned int ip1,ip2,ip3,ip4;
    unsigned char len;
    unsigned char port;
};

struct INPUT{
    unsigned ip1,ip2,ip3,ip4;
};

/* time measurement */
unsigned long long int rdtsc(){
    unsigned long long int x;
    asm volatile("rdtsc":"=A"(x));
    return x;
}

/* structure of binary trie */
struct LIST{
    unsigned int port;
    struct LIST *left,*right;
};

typedef struct LIST node;
typedef node *btrie;

/**
 * Global Variables
 */
btrie root;
int num_entry = 0;
int num_query = 0;
struct ENTRY *table;
struct INPUT *query;
int num_nodes = 0;
unsigned long long int begin,end,total=0;
unsigned long long int *clocker;

unsigned int mem_count=0;

/**
 * Function declaration
 */
btrie create_node();

void add_node(unsigned int ip1,
    unsigned int ip2,
    unsigned int ip3,
    unsigned int ip4,
    unsigned char len,
    unsigned char nexthop);

int read_table(char *str,
    unsigned int *ip1,
    unsigned int *ip2,
    unsigned int *ip3,
    unsigned int *ip4,
    int *len,
    unsigned int *nexthop);

void set_table(char *fn);
void set_query(char *fn);

void build_tree();

void search(unsigned int ip1,
    unsigned int ip2,
    unsigned int ip3,
    unsigned int ip4);

void count_clock(char *fn);
void count_mem(char *fn);

/* Main */
int main(int argc,char *argv[]){

    set_table(argv[1]);
    set_query(argv[1]);
    
    build_tree();

    printf("Avg. Insertion time: %llu\n",(end-begin)/num_entry);
    printf("Number of nodes: %d\n",num_nodes);

    for(int j=0;j<100;j++)
        for(int i=0;i<num_query;i++){
            begin=rdtsc();
            search(query[i].ip1,query[i].ip2,query[i].ip3,query[i].ip4);
            end=rdtsc();
            if(clocker[i]>(end-begin))
                clocker[i]=(end-begin);
        }

    total=0;

    for(int j=0;j<num_query;j++)
        total+=clocker[j];
    
    printf("Avg. Search: %llu\n",total/num_query);

    if(argc < 2)
        count_clock((char *)stdout);
    else if(argc>=2)
        count_clock(argv[2]);
    
    if(argc < 3)
        count_mem((char *)stdout);
    else if(argc>=3)
        count_mem(argv[3]);

    return 0;
}

void search(unsigned int ip1,
    unsigned int ip2,
    unsigned int ip3,
    unsigned int ip4)
{
    btrie current = root, temp=NULL;

    // search len1
    for(int i=31;i>=(-1);i--){
        if(current==NULL)
            break;
        if(current->port!=256)
            temp=current;
        if(ip1&(1<<i)){
            current=current->right;
        }
        else{
            current=current->left;
        }
    }

    // search len2 
    for(int i=31;i>=(-1);i--){
        if(current==NULL)
            break;
        if(current->port!=256)
            temp=current;
        if(ip2&(1<<i)){
            current=current->right;
        }
        else{
            current=current->left;
        }
    }

    // search len3
    for(int i=31;i>=(-1);i--){
        if(current==NULL)
            break;
        if(current->port!=256)
            temp=current;
        if(ip3&(1<<i)){
            current=current->right;
        }
        else{
            current=current->left;
        }
    }

    // search len4
    for(int i=31;i>=(-1);i--){
        if(current==NULL)
            break;
        if(current->port!=256)
            temp=current;
        if(ip4&(1<<i)){
            current=current->right;
        }
        else{
            current=current->left;
        }
    }
}

void count_mem(char *fn){
    // Open file
    FILE *fout = fopen(fn,"w+");

    printf("Each node size: %lu byte\n",sizeof(node));
    printf("Total number of node: %d\n",num_nodes);
    printf("Total memory consumption: %f(KB)\n",num_nodes*sizeof(node)/1024.0);

    fprintf(fout,"%lu %d %f\n", sizeof(node), num_nodes, num_nodes*sizeof(node)/1024.0);
}

void count_clock(char *fn){
    // Open file
    FILE *fout = fopen(fn,"w+");

    // Time Measurement
    unsigned int *num_cnt_clock = (unsigned int *)malloc(NUM_CNT_CLOCK*sizeof(unsigned int));
    for(unsigned int i=0;i<NUM_CNT_CLOCK;i++) num_cnt_clock[i]=0;
    
    unsigned long long min_clock = 10000000,max_clock = 0;
    for(unsigned int i=0;i<num_query;i++){
        if(clocker[i] > max_clock) max_clock=clocker[i];
        if(clocker[i] < min_clock) min_clock=clocker[i];
        if(clocker[i] / 100 < 50) num_cnt_clock[clocker[i]/100]++;
        else num_cnt_clock[49]++;
    }

    printf("(Max_clock, Min_clock) = (%5llu, %5llu)\n", max_clock, min_clock);

    // Max/Min clock 
    fprintf(fout,"# %5llu %5llu\n", max_clock, min_clock);

    for(unsigned int i=0;i<50;i++){
        printf("%d: %d\n",(i+1)*100, num_cnt_clock[i]);
        fprintf(fout,"%d %d\n",(i+1)*100, num_cnt_clock[i]);
    }

    return;
}

void build_tree(){
    root=create_node();
    begin=rdtsc();
    for(int i=0;i<num_entry;i++)
        add_node(table[i].ip1,table[i].ip2,table[i].ip3,table[i].ip4,table[i].len,table[i].port);
}

void set_table(char *fn){
    FILE *fp;
    int len;
    char str[200];
    unsigned int ip1=0,ip2=0,ip3=0,ip4=0,nexthop;
    // read file
    fp=fopen(fn,"r");
    // count number of entries
    while(fgets(str,200,fp)!=NULL){
        if(read_table(str,&ip1,&ip2,&ip3,&ip4,&len,&nexthop))
            num_entry++;
    }
    // rewind
    rewind(fp);
    table=(struct ENTRY *)malloc(num_entry*sizeof(struct ENTRY));
    num_entry=0;
    
    // store value into table
    while(fgets(str,200,fp)!=NULL){
        if(read_table(str,&ip1,&ip2,&ip3,&ip4,&len,&nexthop)){
            table[num_entry].ip1=ip1;
            table[num_entry].ip2=ip2;
            table[num_entry].ip3=ip3;
            table[num_entry].ip4=ip4;
            table[num_entry].port=nexthop;
            table[num_entry++].len=len;
        }
    }
    printf("Number of Entry: %d\n", num_entry);
}

void set_query(char *fn){
    FILE *fp;
    int len;
    char str[200];
    unsigned int ip1=0,ip2=0,ip3=0,ip4=0,nexthop;
    
    // read file 
    fp=fopen(fn,"r");
    // count number of query
    while(fgets(str,200,fp)!=NULL){
        if(read_table(str,&ip1,&ip2,&ip3,&ip4,&len,&nexthop)){
            num_query++;
        }
    }
    // rewind
    rewind(fp);
    query=(struct INPUT *)malloc(num_query*sizeof(struct INPUT));
    clocker=(unsigned long long int *)malloc(num_query*sizeof(unsigned long long int));
    num_query=0;
    // set the queries
    while(fgets(str,200,fp)!=NULL){
        if(read_table(str,&ip1,&ip2,&ip3,&ip4,&len,&nexthop)){
            query[num_query].ip1=ip1;
            query[num_query].ip2=ip2;
            query[num_query].ip3=ip3;
            query[num_query].ip4=ip4;
            clocker[num_query++]=10000000;
        }
    }
    printf("Number of Query %d\n", num_query);
}

int read_table(char *str,
    unsigned int *ip1,
    unsigned int *ip2,
    unsigned int *ip3,
    unsigned int *ip4,
    int *len,
    unsigned int *nexthop)
{
    char *tok="/";
    char *delim;
    char *valid1;
    char *valid2;
    *ip1 = 0, *ip2 = 0, *ip3 = 0, *ip4 = 0;
    int set = 0;
    delim = (char *)strtok(str, tok);
    valid1 = delim;
    if(delim!=NULL){
        delim = (char *)strtok(NULL,tok);
    }
    if(delim!=NULL){
        valid2 = delim;
        set = 1;
    }
    if(set){
        /* get len part */
        char *tok3 = " ";
        char *delim3;
        delim3 = (char *)strtok(valid2, tok3);
        *len = atoi(delim);
        /* get ip part */
        char *tok2=":";
        char *delim2;
        char *temp;
        temp = (char *)strtok(valid1, " ");
        temp = (char *)strtok(NULL, " ");
        delim2 = (char *)strtok(temp, tok2);
        int i = 0;
        unsigned int temp_ip = 0;
        while(delim2!=NULL){
            sscanf(delim2, "%x", &temp_ip);
            if(i==0){
                *ip1 = temp_ip<<16;
            }
            else if(i==1){
                *ip1 = *ip1 + temp_ip;
            }
            else if(i==2){
                *ip2 = temp_ip<<16;
            }
            else if(i==3){
                *ip2 = *ip2 + temp_ip;
            }
            else if(i==4){
                *ip3 = temp_ip<<16;
            }
            else if(i==5){
                *ip3 = *ip3 + temp_ip;
            }
            else if(i==6){
                *ip4 = temp_ip<<16;
            }
            else if(i==7){
                *ip4 = *ip4 + temp_ip;
            }
            delim2 = strtok(NULL, tok2);
            i++;
        }
        //printf("In %u\t%u\t%u\t%u\t%u\n",*ip1,*ip2,*ip3,*ip4,*len);
        /* get nexthop */
        *nexthop = *ip1>>8;
    }
    return set;
}

void add_node(unsigned int ip1,
    unsigned int ip2,
    unsigned int ip3,
    unsigned int ip4,
    unsigned char len,
    unsigned char nexthop)
{
    btrie ptr=root;
    int i;
    int level;
    // 32x4=128
    int len1=32,len2=32,len3=32,len4=32;
    // test len 
    if(len<=32 && len>=0) level=1;
    else if(len>32 && len<=64) level=2;
    else if(len>64 && len<=96) level=3;
    else if(len>96 && len<=128) level=4;

    // test level
    if(level==1) len1=len;
    else len1=32;
    if(level==2) len2=len-32;
    else len2=32;
    if(level=3) len3=len-64;
    else len3=32;
    if(level=4) len4=len-96;
    else len4=32;

    // create len1
    for(int i=0;i<len1;i++){
        if(ip1&(1<<(31-i))){
            if(ptr->right==NULL)
                ptr->right=create_node();
            ptr=ptr->right;
            if((level==1)&&(i==len1-1)&&(ptr->port==256))
                ptr->port=nexthop;
        }
        else{
            if(ptr->left==NULL)
                ptr->left=create_node();
            ptr=ptr->left;
            if((level==1)&&(i==len1-1)&&(ptr->port==256))
                ptr->port=nexthop;
        }
    }

    // create len2
    if(level>=2){
	    for(i=0;i<len2;i++){
		    if(ip2&(1<<(31-i))){
			    if(ptr->right==NULL)
				    ptr->right=create_node(); // Create Node
			    ptr=ptr->right;
			    if((level==2)&&(i==len2-1)&&(ptr->port==256))
			        ptr->port=nexthop;
		    }
		    else{
			    if(ptr->left==NULL)
				    ptr->left=create_node();
			    ptr=ptr->left;
			    if((level==2)&&(i==len2-1)&&(ptr->port==256))
				    ptr->port=nexthop;
		    }
	    }
    }

    // create len3
    if(level>=3){
	    for(i=0;i<len3;i++){
		    if(ip3&(1<<(31-i))){
			    if(ptr->right==NULL)
				    ptr->right=create_node(); // Create Node
			    ptr=ptr->right;
			    if((level==3)&&(i==len3-1)&&(ptr->port==256))
			        ptr->port=nexthop;
		    }
		    else{
			    if(ptr->left==NULL)
				    ptr->left=create_node();
			    ptr=ptr->left;
			    if((level==3)&&(i==len3-1)&&(ptr->port==256))
				    ptr->port=nexthop;
		    }
	    }
    }
    if(level==4){
	    for(i=0;i<len4;i++){
		    if(ip4&(1<<(31-i))){
			    if(ptr->right==NULL)
				    ptr->right=create_node(); // Create Node
			    ptr=ptr->right;
			    if((i==len4-1)&&(ptr->port==256))
			        ptr->port=nexthop;
		    }
		    else{
			    if(ptr->left==NULL)
				    ptr->left=create_node();
			    ptr=ptr->left;
			    if((i==len4-1)&&(ptr->port==256))
				    ptr->port=nexthop;
		    }
	    }
    }
}


btrie create_node(){
    btrie temp;
    num_nodes++;
    temp = (btrie)malloc(sizeof(node));
    temp->right=NULL;
    temp->left=NULL;
    temp->port=256;
    return temp;
}

