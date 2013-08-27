/*
 *  Name : Kuo Liu
 *  Andrew ID : kuol
 *
 * */
#include "cachelab.h"
#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <stdlib.h>

//structure for one line
typedef struct {
	int valid;
	int tag;
	int last_access;	//record the access sequence(range from 1 to E), the biggest represent the most recent
}line;

//structure for one set
typedef struct{
	line *lines;
}set;

//structure for one cache
typedef struct{
	int set_num;
	int line_num_per_set;
	int byte_num;
	set *sets;
}cache;

//used for explaining the arguments
void get_opts(int argc, char *argv[],int *p_s, int *p_E, int *p_b, char *p_t){
	int opt_num = 0;
	int opt;
	while((opt = getopt(argc, argv, "s:E:b:t:")) != -1){
		switch(opt){
			case 's':
				*p_s = atoi(optarg);
				++ opt_num;
				break;
			case 'E':
				*p_E = atoi(optarg);
				++ opt_num;
				break;
			case 'b':
				*p_b = atoi(optarg);
				++ opt_num;
				break;
			case 't':
				strcpy(p_t, optarg);
				++ opt_num;
				break;
			default:
				exit(-1);
				break;
		}
    }
	if(opt_num != 4){
		fprintf(stderr, "there should be four arguments s,E,b,t");
		exit(-1);
	}
}

//get set bits and explain the bits as int
int get_set_value(unsigned address, int s, int b){
	int mask = (0xffffffffu >> (32 - s));
	address >>= b;
	return (address&mask);
}

//get tag bits and explain the bits as int
int get_tag_value(unsigned address, int s, int b){
	return (address >> (s + b));
}

//refresh the last_access num for each line in particular set
void refresh_last_access(line *lines, int E, int sel){
	int last_access = lines[sel].last_access;
	lines[sel].last_access = E;
	for(int i = 0; i < E; ++ i){
		if(i == sel)
			continue;
		if((lines[i].last_access > last_access) && (lines[i].valid == 1))
			-- lines[i].last_access;
	}
}

//init cache
void init_cache(cache *sim_cache, int s, int E, int b){
	int set_num = (1 << s);
	sim_cache->set_num = set_num;
	sim_cache->line_num_per_set = E;
	sim_cache->byte_num = (1 << b);
	sim_cache->sets = (set*) malloc(set_num * sizeof(set));
	if(! sim_cache->sets){
		fprintf(stderr, "error when malloc sets\n");
		exit(-1);
	}
	for(int i = 0; i < set_num; ++ i){
		sim_cache->sets[i].lines = (line*) malloc(E * sizeof(line));
		if(! sim_cache->sets[i].lines){
			fprintf(stderr, "error when malloc lines\n");
			exit(-1);
		}
		for(int j = 0; j < E; ++ j){
			sim_cache->sets[i].lines[j].valid = 0;
		}
	}
}

//simulate each memory operation
void parse_buffer(cache *sim_cache, char *buffer, int s, int E, int b, int *hit, int *miss, int *evict){
	int address;
	char opt;
	sscanf(buffer, " %s %x", &opt, &address);
	int seti = get_set_value(address, s, b);
	int tagi = get_tag_value(address, s, b);
	//simulate cache hit
	for(int i = 0; i < E; ++ i){
		if((sim_cache->sets[seti].lines[i].valid == 1) 
				&& (sim_cache->sets[seti].lines[i].tag == tagi)){
			++ (*hit);
			if(opt == 'M')
				++ (*hit);
			refresh_last_access(sim_cache->sets[seti].lines, E, i);
			return ;
		}
	}
	
	//simulate cache miss while there is no need to evict
	++ (*miss);
	for(int i = 0; i < E; ++ i){
		if(sim_cache->sets[seti].lines[i].valid == 0){
			sim_cache->sets[seti].lines[i].valid = 1;
			sim_cache->sets[seti].lines[i].tag = tagi;
			if(opt == 'M')
				++ (*hit);
			refresh_last_access(sim_cache->sets[seti].lines, E, i);
			return ;
		}
	}

	//simulate cache miss and there is need to evict
	++ (*evict);
	for(int i = 0; i < E; ++ i){
		if(sim_cache->sets[seti].lines[i].last_access == 1){
			sim_cache->sets[seti].lines[i].tag = tagi;
			if(opt == 'M')
				++ (*hit);
			refresh_last_access(sim_cache->sets[seti].lines, E, i);
			return;
		}
	}
}

int main(int argc, char *argv[]){
    int s, E, b;
	char *trace = (char*) malloc(50 * sizeof(char));
	cache sim_cache;
	get_opts(argc,argv, &s, &E, &b, trace);
	init_cache(&sim_cache, s, E, b);

	FILE *p_t = fopen(trace, "r");
	if(! p_t){
		fprintf(stderr, "error when open trace file %s\n", trace);
		return -1;
	}

	int hit, miss, evict;
	hit = miss = evict = 0;
	
	char *buffer = (char*) malloc(50 * sizeof(char));
	while(fgets(buffer, 50, p_t) != NULL){
		if(buffer[0] == ' '){
			parse_buffer(&sim_cache, buffer, s, E, b, &hit, &miss, &evict);
		}
	}

	printSummary(hit, miss, evict);
	return 0;
}
