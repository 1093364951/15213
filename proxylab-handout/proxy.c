/*
 * Andrew ID: kuol Name: Kuo Liu
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "csapp.h"
#include "sbuf.h"
#include "cache.h"
#define DEBUG

sbuf_t sbuf;	/* Shared buffer of connected descriptors */

void forward_response(int, int, HTTP_Response *);
int forward_request(int, HTTP_Request *, HTTP_Response *);
void parse_request_header(int, HTTP_Request *);
void handle_request(int);
void *thread(void *);

#define NTHREADS  16
#define SBUFSIZE  4
#define MIN(a,b)	((a) < (b) ? (a) : (b))

int main (int argc, char *argv []) {
    int i, listen_fd, connfd, port;
    socklen_t clientlen = sizeof(struct sockaddr_in);
    struct sockaddr_in clientaddr;
    pthread_t tid;
    
    if(argc != 2){
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(1);
    }
    Signal(SIGPIPE, SIG_IGN);
    init_cache();

    port = atoi(argv[1]); 
    sbuf_init(&sbuf, SBUFSIZE);
    listen_fd = Open_listenfd(port);
    for (i = 0; i < NTHREADS; i++)
        Pthread_create(&tid, NULL, thread, NULL);

    while (1) {
        connfd = Accept(listen_fd, (SA*)&clientaddr, &clientlen);
        sbuf_insert(&sbuf, connfd);
    }
    free_cache();
    return 0;
}

/* Worker Thread */
void *thread(void *vargp) {  
    Pthread_detach(pthread_self()); 
    while (1) { 
        int client_fd = sbuf_remove(&sbuf);
        handle_request(client_fd);
    }    
}

/* Process request */
void handle_request(int connfd) {
    HTTP_Request request;
    HTTP_Response response;
    parse_request_header(connfd, &request);
    if(forward_request(connfd, &request, &response) < 0) {
        close(connfd);
    }else{
        if(response.size <= MAX_OBJECT_SIZE && response.header[9]=='2')
            save_to_cache(&request, &response);
    }
    close(connfd);
}

void parse_request_header(int connfd, HTTP_Request *request) {
    size_t n, size = 0;
    rio_t cli_rio;
    char buf[MAXLINE];
    rio_readinitb(&cli_rio, connfd);

    request->content = Malloc(sizeof(char) * MAX_OBJECT_SIZE);
	char * current_pos = request->content;    
	while ((n = rio_readlineb(&cli_rio, buf, MAXLINE)) != 0) { 
		if (strstr(buf, "HTTP/1.1") != NULL) {
			strncpy(strstr(buf, "HTTP/1.1"), "HTTP/1.0", 8);
			strcpy(request->request_line, buf);
			request->request_line[n]='\0';
		}
		size += n;
		memcpy(current_pos, buf, n * sizeof(char));
		current_pos += n;
		printf("...");
		if (strstr(buf, "Host") != NULL) {
			int len = strlen(buf + 6) - 2;  
			strncpy(request->host, buf + 6, len); 
			request->host[len] = '\0';
		}
		if (!strcmp(buf, "\r\n")) {
			break;
		}
	}
	request->content[size] = '\0'; 
	size = 0;
}

/* forward the response from server to client */
void forward_response(int client_fd, int server_fd, HTTP_Response *response) {
    int length = -1, n, size;
    char header_buffer[MAXLINE], content_buffer[MAX_OBJECT_SIZE];
    rio_t ser_rio;

    Rio_readinitb(&ser_rio, server_fd);
    while((n = rio_readlineb(&ser_rio, header_buffer, MAXLINE)) != 0) { 
        strcat(response->header, header_buffer); 
        if(rio_writen(client_fd, header_buffer, n) < 0)
            STDERR("rio_writen in forward_response header error");
        if(strstr(header_buffer, "Content-Length: "))
            sscanf(header_buffer + 16, "%d", &length);
        if(!strcmp(header_buffer, "\r\n"))
            break;
    }

    if(length == -1)
        size = MAX_OBJECT_SIZE;
    else 
        size = MIN(length, MAX_OBJECT_SIZE);
    
    int sum = 0;
    while((n = rio_readnb(&ser_rio, content_buffer, size)) != 0) { 
        if(rio_writen(client_fd, content_buffer, n) < 0) {
            STDERR("rio_writen in forward_response content error");  
            Close(client_fd);
        }
        sum += n;
    }

    response->size = sum;
    if(sum <= MAX_OBJECT_SIZE){
        response->content = Malloc(sizeof(char) * sum);
        memcpy(response->content, content_buffer, sum * sizeof(char)); 
    }
}

/* forward the request from client to server */
int forward_request(int cli_fd, HTTP_Request *request, HTTP_Response *response) {
    rio_t ser_rio;
    int ser_fd, port = 80;

    if((ser_fd = open_clientfd(request->host, port)) < 0) {
        STDERR("Fail to connect to the server\n"); 
        return -1;
    }
    rio_readinitb(&ser_rio, ser_fd);
    rio_writen(ser_fd, request->content, strlen(request->content));
    free(request->content);
    forward_response(cli_fd, ser_fd, response);
    close(ser_fd);
    return 0;
}
