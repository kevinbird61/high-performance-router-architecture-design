#include <stdio.h>
#include <stdlib.h>
#include <string.h>

unsigned long long int rdtsc() {
	unsigned long long x;
	asm volatile ("rdtsc" : "=A" (x));
	return x;
}

unsigned long long int rdtsc_64bits()//64-bit
{
   unsigned long long int x;
   unsigned a, d;

   __asm__ volatile("rdtsc" : "=a" (a), "=d" (d));

   return ((unsigned long long)a) | (((unsigned long long)d) << 32);
}

struct ipv6_prefix {
	unsigned int ip[4];
	unsigned char len;
	unsigned char layer;
};

struct Eset {
	int ruleID;
	struct Eset *next;
};

struct MSPTrie {
	int ruleID;
	struct MSPTrie *left;
	struct MSPTrie *right;
	struct Eset *set;
};

int MSPT_Search(int);
int cover_ip(int, int);
int cmp_ip(int, int);
struct MSPTrie *new_node();
int node_height(struct MSPTrie *);
int node_factor(struct MSPTrie *);
void regroup_Eset(struct MSPTrie *, struct MSPTrie *);
struct MSPTrie *rotate_LL(struct MSPTrie *);
struct MSPTrie *rotate_RR(struct MSPTrie *);
struct MSPTrie *rotate_LR(struct MSPTrie *);
struct MSPTrie *rotate_RL(struct MSPTrie *);
struct MSPTrie *node_balance(struct MSPTrie *);
void insert_node(int);
struct Eset *insert_eset(struct Eset *, int);
int cmp_r_l(int, int);
int cover_r(int, int);
int cmp_r(int, int);
void readTable(char *);



struct MSPTrie *root;

struct ipv6_prefix *table;
int num_entry = 0;

int main(int argc, char *argv[]) {
	int i, j;
	readTable(argv[1]);

	unsigned long long int begin, end, total = 0;
	unsigned long long int *clock = malloc(num_entry * sizeof(unsigned long long int));

	begin = rdtsc();
	for (i = 0; i < num_entry; i++) {
		//printf("%d\n", i);
		insert_node(i);	
	}
	end = rdtsc();
	printf("Avg. Insert: %llu\n", (end-begin)/num_entry);

	for(i=0; i<num_entry; i++) {
		clock[i] = 10000000;
	}
	for(j=0; j<100; j++) {
		for(i=0; i<num_entry; i++) {
			begin = rdtsc();
			MSPT_Search(i);
			end = rdtsc();

			if(clock[i] > (end-begin)) {
				clock[i] = end-begin;
			}
		}
	}
	for(i=0; i<num_entry; i++) {
		total += clock[i];
	}
	printf("Avg. Search: %llu\n", total/num_entry);

	unsigned long long int min, max;
	unsigned int dis[50] = {0};
	min = 10000000;
	max = 0;

	for(i=0; i<num_entry; i++) {
		if(clock[i] > max) max = clock[i];
		if(clock[i] < min) min = clock[i];
		if(clock[i] / 1000 < 50) dis[clock[i]/1000]++;
		else dis[49]++;
	}
	printf("(Min, Max) = (%5llu, %5llu)\n", min, max);

	for(i=0; i<50; i++) {
		printf("%d , %d\n", (i+1)*1000, dis[i]);
	}

	return 0;
}

int MSPT_Search(int ruleID) {
	struct MSPTrie *now = root;
	struct MSPTrie *node[20];
	int s = 0;
	int i;
	while (now != NULL) {
		if (cover_ip(ruleID, now->ruleID)) {
			return 1;
		}
		else {
			if (now->set != NULL) {
				node[s++] = now;
			}
			if (cmp_ip(ruleID, now->ruleID) == -1) {
				now = now->left;
			}
			else if (cmp_ip(ruleID, now->ruleID) == 1) {
				now = now->right;
			}
		}
	}

	struct Eset *no;
	for (i = s - 1; i >= 0; i--) {
		no = node[i]->set;

		while (no != NULL) {
			if (cover_ip(ruleID, no->ruleID)) {
				return 2;
			}
			no = no->next;
		}
	}
	return 0;
}

int cover_ip(int r1, int r2) { //check if r1.ip covered by r2
	int i;

	unsigned int bit1, bit2;
	for (i = 0; i < table[r2].len; i++) {
		if (i < 32) {
			bit1 = table[r1].ip[0] & (1 << 31 - i);
			bit2 = table[r2].ip[0] & (1 << 31 - i);
		}
		else if (i < 64) {
			bit1 = table[r1].ip[1] & (1 << 63 - i);
			bit2 = table[r2].ip[1] & (1 << 63 - i);
		}
		else if (i < 96) {
			bit1 = table[r1].ip[2] & (1 << 95 - i);
			bit2 = table[r2].ip[2] & (1 << 95 - i);
		}
		else {
			bit1 = table[r1].ip[3] & (1 << 127 - i);
			bit2 = table[r2].ip[3] & (1 << 127 - i);
		}


		if (bit1 != bit2) return 0;
	}

	return 1;
}

int cmp_ip(int r1, int r2) { // check > = < between r1.ip and r2
	int i;
	unsigned int bit1, bit2;
	int cmp1, cmp2;

	for (i = 0; i < table[r2].len; i++) {
		if (i < 32) {
			bit1 = table[r1].ip[0] & (1 << 31 - i);
			bit2 = table[r2].ip[0] & (1 << 31 - i);
		}
		else if (i < 64) {
			bit1 = table[r1].ip[1] & (1 << 63 - i);
			bit2 = table[r2].ip[1] & (1 << 63 - i);
		}
		else if (i < 96) {
			bit1 = table[r1].ip[2] & (1 << 95 - i);
			bit2 = table[r2].ip[2] & (1 << 95 - i);
		}
		else {
			bit1 = table[r1].ip[3] & (1 << 127 - i);
			bit2 = table[r2].ip[3] & (1 << 127 - i);
		}

		if (bit1 > bit2) return 1;
		else if (bit1 < bit2) return -1;
	}

	return 0;
}


int node_height(struct MSPTrie *node) {
	int hl = 0;
	int hr = 0;

	if (node->left) hl = node_height(node->left);
	if (node->right) hr = node_height(node->right);

	return hr > hl ? ++hr : ++hl;
}

int node_factor(struct MSPTrie *node) {
	int factor = 0;

	if (node->left) factor += node_height(node->left);
	if (node->right) factor -= node_height(node->right);

	return factor;
}

void regroup_Eset(struct MSPTrie *a, struct MSPTrie *b) { //take rule from a.eset to b.eset
	struct Eset *now = a->set;
	struct Eset *prev = NULL;

	while (now != NULL) {
		if (cover_r(now->ruleID, b->ruleID) == 1) { // now in a.set cover b
			//printf("take %d\n", now->ruleID);

			b->set = insert_eset(b->set, now->ruleID);

			if (prev == NULL) { // now is root
				//printf("head\n");
				a->set = now->next;
				free(now);
				now = a->set;
			}
			else { // now isn't root
				//printf("not head\n");
				prev->next = now->next;
				free(now);
				now = prev->next;
			}
		}
		else {
			prev = now;
			now = now->next;
		}
	}

}

struct MSPTrie *rotate_LL(struct MSPTrie *node) {
	//printf("LL\n");
	struct MSPTrie *a = node;
	struct MSPTrie *b = a->left;

	a->left = b->right;
	b->right = a;

	regroup_Eset(a, b);
	return b;
}
struct MSPTrie *rotate_RR(struct MSPTrie *node) {
	//printf("RR\n");
	struct MSPTrie *a = node;
	struct MSPTrie *b = a->right;

	a->right = b->left;
	b->left = a;

	regroup_Eset(a, b);
	return b;

}
struct MSPTrie *rotate_LR(struct MSPTrie *node) {
	//printf("LR\n");
	struct MSPTrie *a = node;
	struct MSPTrie *b = a->left;
	struct MSPTrie *c = b->right;

	a->left = c->right;
	b->right = c->left;
	c->left = b;
	c->right = a;

	regroup_Eset(b, c);
	regroup_Eset(a, c);
	return c;

}
struct MSPTrie *rotate_RL(struct MSPTrie *node) {
	//printf("RL\n");
	struct MSPTrie *a = node;
	struct MSPTrie *b = a->right;
	struct MSPTrie *c = b->left;

	a->right = c->left;
	b->left = c->right;
	c->right = b;
	c->left = a;

	regroup_Eset(a, c);
	regroup_Eset(b, c);
	return c;
}

struct MSPTrie *node_balance(struct MSPTrie *node) {
	struct MSPTrie *new = NULL;

	if (node->left)
		node->left = node_balance(node->left);
	if (node->right)
		node->right = node_balance(node->right);

	int factor = node_factor(node);

	if (factor > 1) {
		if (node_factor(node->left) < 0) {
			new = rotate_LR(node);
		}
		else {
			new = rotate_LL(node);
		}
	}
	else if (factor < -1) {
		if (node_factor(node->right) > 0) {
			new = rotate_RL(node);
		}
		else {
			new = rotate_RR(node);
		}
	}
	else
		new = node;

	return new;
}

struct MSPTrie *new_node() {
	struct MSPTrie *tmp = malloc(sizeof(struct MSPTrie));

	tmp->ruleID = -1;
	tmp->left = NULL;
	tmp->right = NULL;
	tmp->set = NULL;

	return tmp;
}

void insert_node(int ruleID) {
	struct MSPTrie *now = root;
	struct MSPTrie *prev;

	if (now == NULL) { //no root
		root = new_node();
		root->ruleID = ruleID;

		return;
	}

	while (now != NULL) {
		prev = now;

		if (cmp_r(ruleID, now->ruleID) == 0) //do nothing
			return;

		if (cover_r(ruleID, now->ruleID) == -1) { //now cover new, change and insert now to eset
			now->set = insert_eset(now->set, now->ruleID);
			now->ruleID = ruleID;
			return;
		}

		if (cover_r(ruleID, now->ruleID) == 1) { //new cover now, insert to eset
			now->set = insert_eset(now->set, ruleID);

			return;
		}

		if (cmp_r(ruleID, now->ruleID) == 1) { // go right;
			now = now->right;
		}

		else if (cmp_r(ruleID, now->ruleID) == -1) { // go left;
			now = now->left;
		}
	}

	//now is null
	now = new_node();
	now->ruleID = ruleID;

	if (cmp_r(now->ruleID, prev->ruleID) == 1) prev->right = now;
	else prev->left = now;

	//balance
	root = node_balance(root);
	return;
}
struct Eset *insert_eset(struct Eset *head, int ruleID) {
	struct Eset *now = head;
	struct Eset *prev = NULL;

	if (now == NULL) { //no head
		now = malloc(sizeof(struct Eset));
		now->ruleID = ruleID;
		now->next = NULL;

		return now;
	}

	while (now != NULL) {
		if (cmp_r_l(ruleID, now->ruleID) == 1)
			break;
		else if (cmp_r_l(ruleID, now->ruleID) == 0 && cmp_r(ruleID, now->ruleID) == 0) {
			printf("AAAA\n");
			return head;
		}

		prev = now;
		now = now->next;
	}

	if (now == NULL) { // the end of linked-list
		prev->next = malloc(sizeof(struct Eset));
		prev->next->ruleID = ruleID;
		prev->next->next = NULL;

		return head;
	}
	else { //insert into some position of linked-list
		if (prev == NULL) { //insert to head
			prev = malloc(sizeof(struct Eset));
			prev->ruleID = ruleID;
			prev->next = head;

			return prev;
		}

		else { //insert to a position which isn't head
			prev->next = malloc(sizeof(struct Eset));
			prev->next->ruleID = ruleID;
			prev->next->next = now;

			return head;
		}
	}

	return head;
}

int cmp_r_l(int r1, int r2) {
	if (table[r1].len > table[r2].len) return 1;
	else if (table[r1].len < table[r2].len) return -1;
	else return 0;
}

int cover_r(int r1, int r2) {
	int i;
	int min_len = (table[r1].len < table[r2].len) ? table[r1].len : table[r2].len;

	if (table[r1].len == 0) return 1;
	if (table[r2].len == 0) return -1;

	unsigned int bit1, bit2;
	for (i = 0; i < min_len; i++) {
		if (i < 32) {
			bit1 = table[r1].ip[0] & (1 << 31 - i);
			bit2 = table[r2].ip[0] & (1 << 31 - i);
		}
		else if (i < 64) {
			bit1 = table[r1].ip[1] & (1 << 63 - i);
			bit2 = table[r2].ip[1] & (1 << 63 - i);
		}
		else if (i < 96) {
			bit1 = table[r1].ip[2] & (1 << 95 - i);
			bit2 = table[r2].ip[2] & (1 << 95 - i);
		}
		else {
			bit1 = table[r1].ip[3] & (1 << 127 - i);
			bit2 = table[r2].ip[3] & (1 << 127 - i);
		}


		if (bit1 != bit2) return 0;
	}

	if (table[r1].len < table[r2].len)
		return 1;
	else
		return -1;
}

int cmp_r(int r1, int r2) {
	int i;
	unsigned int bit1, bit2;
	int cmp1, cmp2;
	int max_len = (table[r1].len > table[r2].len) ? table[r1].len : table[r2].len;

	for (i = 0; i < max_len; i++) {
		if (i >= table[r1].len) {
			cmp1 = 0;
		}
		else {
			if (i < 32) {
				bit1 = table[r1].ip[0] & (1 << 31 - i);
			}
			else if (i < 64) {
				bit1 = table[r1].ip[1] & (1 << 63 - i);
			}
			else if (i < 96) {
				bit1 = table[r1].ip[2] & (1 << 95 - i);
			}
			else {
				bit1 = table[r1].ip[3] & (1 << 127 - i);
			}
			cmp1 = bit1 ? 1 : -1;
		}

		if (i >= table[r2].len) {
			cmp2 = 0;
		}
		else {
			if (i < 32) {
				bit2 = table[r2].ip[0] & (1 << 31 - i);
			}
			else if (i < 64) {
				bit2 = table[r2].ip[1] & (1 << 63 - i);
			}
			else if (i < 96) {
				bit2 = table[r2].ip[2] & (1 << 95 - i);
			}
			else {
				bit2 = table[r2].ip[3] & (1 << 127 - i);
			}
			cmp2 = bit2 ? 1 : -1;
		}


		//printf("%d %d\n", cmp1, cmp2);

		if (cmp1 > cmp2) return 1;
		else if (cmp1 < cmp2) return -1;
	}

	return 0;
}

void readTable(char *filename) {
	int i;
	FILE *fp;
	char string[100];
	fp = fopen(filename, "r");

	while (fgets(string, 100, fp) != NULL) {
		for (i = 0; i < 100 && string[i] != '\0'; i++) {
			if (string[i] == '/') {
				num_entry++;
				break;
			}
		}
	}

	table = calloc(num_entry, sizeof(struct ipv6_prefix));
	rewind(fp);

	char check;
	num_entry = 0;
	char left[100];
	char right[100];
	char *ip;

	while (fgets(string, 100, fp) != NULL) {
		check = 0;
		for (i = 0; i < 100 && string[i] != '\0'; i++) {
			if (string[i] == '/') {
				check = 1;
				break;
			}
		}

		if (!check) continue;

		sprintf(left, "%s%c", strtok(string, "/"), '\0');
		sprintf(right, "%s%c", strtok(NULL, "/"), '\0');

		table[num_entry].len = atoi(strtok(right, " "));

		sprintf(left, "%s%c", strtok(left, " *>"), '\0');

		i = 0;
		unsigned ips[8] = {0};
		ip = strtok(left, ":");
		while (ip != NULL) {
			sscanf(ip, "%x", &ips[i++]);

			ip = strtok(NULL, ":");
		}


		table[num_entry].ip[0] = ips[0] << 16 | ips[1];
		table[num_entry].ip[1] = ips[2] << 16 | ips[3];
		table[num_entry].ip[2] = ips[4] << 16 | ips[5];
		table[num_entry].ip[3] = ips[6] << 16 | ips[7];

		table[num_entry].layer = 0;

		num_entry++;
	}

	return;
}
