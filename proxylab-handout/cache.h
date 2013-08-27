/*
 * Andrew ID: kuol, Name: Kuo Liu
 */
#ifndef __CACHE_H__
#define __CACHE_H__

#include "csapp.h"

#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

#define STDERR(str) fprintf(stderr, str)

typedef struct {
	char request_line[MAXLINE];
	char host[MAXLINE];
	char *content;
} HTTP_Request;

typedef struct {
	char header[MAXLINE];
	char *content;
	int size;
} HTTP_Response;

typedef struct Cache_Obj Cache_Obj;
struct Cache_Obj{
	unsigned key;
	char *content;
	int size;
	Cache_Obj *pre;
	Cache_Obj *next;
};

typedef struct {
	Cache_Obj *head;
	Cache_Obj *tail;
	int size;
} Cache;
Cache cache;

pthread_mutex_t mutex_i;

void init_cache();
void free_cache();
void add_cache_obj(Cache_Obj *cache_obj);
void save_to_cache(HTTP_Request *request, HTTP_Response *response);
void delete_obj_from_list(Cache_Obj *cache_obj);
void free_cache_obj(Cache_Obj *cache_obj);
void delete_cache_obj(Cache_Obj *cache_obj);
int is_in_cache(HTTP_Request *request, HTTP_Response *response);
unsigned crc32(char *s, int len);

#endif
