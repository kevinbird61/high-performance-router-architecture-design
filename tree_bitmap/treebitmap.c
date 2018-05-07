#include<stdlib.h>
#include<stdio.h>
#include<string.h>
////////////////////////////////////////////////////////////////////////////////////
struct ENTRY{
	unsigned int ip1;
    unsigned int ip2;
    unsigned int ip3;
    unsigned int ip4;
	unsigned char len;
	unsigned char port;
};
////////////////////////////////////////////////////////////////////////////////////
struct INPUT{
    unsigned int ip1;
    unsigned int ip2;
    unsigned int ip3;
    unsigned int ip4;
};
////////////////////////////////////////////////////////////////////////////////////
inline unsigned long long int rdtsc()
{
	unsigned long long int x;
	asm   volatile ("rdtsc" : "=A" (x));
	return x;
}
////////////////////////////////////////////////////////////////////////////////////
struct list{//structure of binary trie
	unsigned int port;
	struct list *left,*right;
};
typedef struct list node;
typedef node *btrie;
////////////////////////////////////////////////////////////////////////////////////
struct list1{
    char internal[15];
    char external[16];
    unsigned int *port;
    struct list1 *child[16];
};
typedef struct list1 mapnode;
typedef mapnode *maptrie;
////////////////////////////////////////////////////////////////////////////////////
/*global variables*/
btrie root;
maptrie maproot;
int num_entry=0;
int num_query=0;
struct ENTRY *table;
struct INPUT *query;
int N=0;//number of nodes
unsigned long long int begin,end,total=0;
unsigned long long int *clock;
int num_node=0;//total number of nodes in the binary trie
int num_mapnode = 0;
unsigned int MemCount = 0;
////////////////////////////////////////////////////////////////////////////////////
btrie create_node(){
	btrie temp;
	num_node++;
	temp=(btrie)malloc(sizeof(node));
	temp->right=NULL;
	temp->left=NULL;
	temp->port=256;//default port
	return temp;
}
////////////////////////////////////////////////////////////////////////////////////
maptrie create_mapnode(){
    int i;
    unsigned int temp_port = 256;
    maptrie temp;
    num_mapnode++;
    temp = (maptrie)malloc(sizeof(mapnode));
    for(i=0;i<15;i++) temp->internal[i] = 0;
    for(i=0;i<16;i++){
        temp->external[i] = 0;
        temp->child[i] = NULL;
    }
    temp->port = &temp_port;
    return temp;
}
////////////////////////////////////////////////////////////////////////////////////
void add_node(unsigned int ip1,unsigned int ip2, unsigned int ip3, unsigned int ip4, unsigned char len,unsigned char nexthop){
	btrie ptr=root;
	int i;
    int level;
    int len1=32, len2=32, len3=32, len4=32;
    if(len<=32 && len>=0) level = 1;
    else if(len>32 && len<=64) level = 2;
    else if(len>64 && len<=96) level = 3;
    else if(len>96 && len<=128) level = 4;
//////////////////////////////////////////////////
    if(level==1) len1 = len;
    else len1 = 32;
    if(level==2) len2 = len-32;
    else len2 = 32;
    if(level==3) len3 = len-64;
    else len3 =32;
    if(level==4) len4 = len-96;
    else len4 = 32;

	for(i=0;i<len1;i++){
		if(ip1&(1<<(31-i))){
			if(ptr->right==NULL)
				ptr->right=create_node(); // Create Node
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
////////////////////////////////////////////////////////////////////////////////////
void buildtree(btrie current, maptrie head, int num, unsigned int *arr){
    if(current->port!=256){
        if(num<15){
            head->internal[num] = 1;
            arr[num] = current->port;
        }
    }
    if(num<15){
        if(current->left!=NULL)
            buildtree(current->left, head, num*2+1, arr);
        if(current->right!=NULL)
            buildtree(current->right, head, num*2+2, arr);
    }
    else{
        if(current->left!=NULL){
            unsigned int arr1[15];
            head->child[(num-15)] = create_mapnode();
            buildtree(current->left, head->child[(num-15)], 0, arr1);
        }
        if(current->right!=NULL){
            unsigned int arr2[15];
            head->child[(num-15)] = create_mapnode();
            buildtree(current->right, head->child[(num-15)], 0, arr2);
        }
    }
    if(num==0){
        int i;
        int counter = 0;
        for(i=0;i<15;i++){
            if(head->internal[i]!=0)
                arr[counter++] = arr[i];
        }
        unsigned int *outport = (unsigned int*)malloc(counter*sizeof(unsigned int));
        for(i=0;i<counter;i++)
            *(outport+i) = arr[i];
        head->port = outport;
    }
}
////////////////////////////////////////////////////////////////////////////////////
int read_table(char *str,unsigned int *ip1, unsigned int *ip2, unsigned int *ip3, unsigned int*ip4 ,int *len,unsigned int *nexthop){
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
////////////////get len part//////////////////////
        char *tok3 = " ";
        char *delim3;
        delim3 = (char *)strtok(valid2, tok3);
        *len = atoi(delim);
/////////////////get ip part//////////////////////
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
//        printf("In %u\t%u\t%u\t%u\t%u\n",*ip1,*ip2,*ip3,*ip4,*len);
////////////////////////get nexthop////////////////
        *nexthop = *ip1>>8;
    }
    return set;
}
////////////////////////////////////////////////////////////////////////////////////
void search(unsigned int ip1, unsigned int ip2, unsigned int ip3, unsigned int ip4){
    int i,j,k;
    maptrie current = maproot, temp = NULL;
    unsigned int shift_ip;
    unsigned int x, y, z;
    char outport;
    int countx = 0, county = 0, countz = 0;
    int flagy = 1, flagz = 1;
    for(k=0;k<4;k++){
        for(i=28;i>=0;i-=4){
            countx = 0;
            county = 0;
            countz = 0;
            flagy = 1;
            flagz = 1;
            if(k==0){
                shift_ip = (ip1>>i) & 15;
                x = (ip1>>i+1) & 7;
                y = (ip1>>i+2) & 3;
                z = (ip1>>i+3) & 1;
            }
            else if(k==1){
                shift_ip = (ip2>>i) & 15;
                x = (ip2>>i+1) & 7;
                y = (ip2>>i+2) & 3;
                z = (ip2>>i+3) & 1;
            }
            else if(k==2){
                shift_ip = (ip3>>i) & 15;
                x = (ip3>>i+1) & 7;
                y = (ip3>>i+2) & 3;
                z = (ip3>>i+3) & 1;
            }
            else if(k==3){
                shift_ip = (ip4>>i) & 15;
                x = (ip4>>i+1) & 7;
                y = (ip4>>i+2) & 3;
                z = (ip4>>i+3) & 1;
            }
            ///////////////////////////////////////////////
            if(current->internal[x+7]==1){
                for(j=0;j<x+7;j++)
                    if(current->internal[j]==1)countx++;
            }
            if(countx>0){
                flagy = 0;
                flagz = 0;
                outport = *(current->port + (countx-1));
            }
            if(flagy==1){
                if(current->internal[y+3]==1){
                    for(j=0;j<y+3;j++)
                        if(current->internal[j]==1)county++;
                }
            }
            if(county>0){
                flagz = 0;
                outport = *(current->port + (county-1));
            }
            if(flagz==1){
                if(current->internal[z+1]==1){
                    for(j=0;j<z+1;j++)
                        if(current->internal[j]==1)countz++;
                }
            }
            if(countz>0)
                outport = *(current->port + (countz-1));
            if(current->child[shift_ip]!=NULL){
                temp = current;
                current = current->child[shift_ip];
            }
            else 
                return;
        }
    }
}
////////////////////////////////////////////////////////////////////////////////////
void set_table(char *file_name){
	FILE *fp;
	int len;
	char string[200];
    int set_rule = 0;
	unsigned int ip1 = 0,ip2 = 0,ip3 = 0,ip4 = 0,nexthop;
	fp=fopen(file_name,"r");
	while(fgets(string,200,fp)!=NULL){
		set_rule = read_table(string,&ip1,&ip2,&ip3,&ip4,&len,&nexthop);
        if(set_rule) num_entry++;
	}
	rewind(fp);
	table=(struct ENTRY *)malloc(num_entry*sizeof(struct ENTRY));
	num_entry=0;
    int i =0;
	while(fgets(string,200,fp)!=NULL){
		set_rule = read_table(string,&ip1,&ip2,&ip3,&ip4,&len,&nexthop);
        if(set_rule){
		    table[num_entry].ip1=ip1;
		    table[num_entry].ip2=ip2;
		    table[num_entry].ip3=ip3;
		    table[num_entry].ip4=ip4;
		    table[num_entry].port=nexthop;
		    table[num_entry++].len=len;
        }
	}
    printf("Number of Entry %d\n", num_entry);
//    for(i=0;i<20;i++) printf("%u\t%u\t%u\t%u\t%u\n", table[i].ip1,table[i].ip2,table[i].ip3,table[i].ip4,table[i].len);
}
////////////////////////////////////////////////////////////////////////////////////
void set_query(char *file_name){
	FILE *fp;
	int len;
	char string[200];
	unsigned int ip1 = 0,ip2 = 0,ip3 = 0,ip4 = 0,nexthop;
    int set_trace = 0;
	fp=fopen(file_name,"r");
	while(fgets(string,200,fp)!=NULL){
		set_trace = read_table(string,&ip1,&ip2,&ip3,&ip4,&len,&nexthop);
		if(set_trace) num_query++;
	}
	rewind(fp);
	query=(struct INPUT *)malloc(num_query*sizeof(struct INPUT));
	clock=(unsigned long long int *)malloc(num_query*sizeof(unsigned long long int));
	num_query=0;
	while(fgets(string,200,fp)!=NULL){
		set_trace = read_table(string,&ip1, &ip2, &ip3, &ip4,&len,&nexthop);
        if(set_trace){
		    query[num_query].ip1=ip1;
		    query[num_query].ip2=ip2;
		    query[num_query].ip3=ip3;
		    query[num_query].ip4=ip4;
		    clock[num_query++]=10000000;
        }
	}
    printf("Number of Query %d\n", num_query);
}
////////////////////////////////////////////////////////////////////////////////////
void create(){
	int i;
	root=create_node();
	begin=rdtsc();
	for(i=0;i<num_entry;i++)
		add_node(table[i].ip1,table[i].ip2,table[i].ip3,table[i].ip4,table[i].len,table[i].port);
}
////////////////////////////////////////////////////////////////////////////////////
void create1(){
    unsigned int arr[15];
    maproot = create_mapnode();
    buildtree(root,maproot,0,arr);
	end=rdtsc();
}
////////////////////////////////////////////////////////////////////////////////////
void CountClock()
{
	unsigned int i;
	unsigned int* NumCntClock = (unsigned int* )malloc(50 * sizeof(unsigned int ));
	for(i = 0; i < 50; i++) NumCntClock[i] = 0;
	unsigned long long MinClock = 10000000, MaxClock = 0;
	for(i = 0; i < num_query; i++)
	{
		if(clock[i] > MaxClock) MaxClock = clock[i];
		if(clock[i] < MinClock) MinClock = clock[i];
		if(clock[i] / 100 < 50) NumCntClock[clock[i] / 100]++;
		else NumCntClock[49]++;
	}
	printf("(MaxClock, MinClock) = (%5llu, %5llu)\n", MaxClock, MinClock);
	
	for(i = 0; i < 50; i++)
	{
		printf("%d : %d\n", (i + 1) * 100, NumCntClock[i]);
	}
	return;
}
////////////////////////////////////////////////////////////////////////////////////
void CountMem(maptrie head){
    int i;
    for(i=0;i<15;i++){
        if(head->internal[i]==1) MemCount++;
    }
    for(i=0;i<16;i++){
        if(head->child[i]!=NULL){
            CountMem(head->child[i]);
        }
    }
}
////////////////////////////////////////////////////////////////////////////////////
int main(int argc,char *argv[]){
	int i,j;
	set_table(argv[1]);
    set_query(argv[1]);
	create();
    create1();
	printf("Avg. Insert: %llu\n",(end-begin)/num_entry);
	printf("number of nodes: %d\n",num_mapnode);
	////////////////////////////////////////////////////////////////////////////
	for(j=0;j<100;j++){
		for(i=0;i<num_query;i++){
			begin=rdtsc();
			search(query[i].ip1,query[i].ip2,query[i].ip3,query[i].ip4);
			end=rdtsc();
			if(clock[i]>(end-begin))
				clock[i]=(end-begin);
		}
	}
	total=0;
	for(j=0;j<num_query;j++)
		total+=clock[j];
	printf("Avg. Search: %llu\n",total/num_query);
	CountClock();
    CountMem(maproot);
    unsigned int MemTotal = 0;
    MemTotal = (31*num_mapnode)/8 + (2*num_mapnode) + MemCount;
	printf("Total memory requirement: %d KB\n",((MemTotal)/1024));
    printf("MemCount: %u\n", MemCount);
	return 0;
}
