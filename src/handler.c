#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "include/store.h"

static int REQ_SIZE = 50000;

static void consume_req(int conn, char *req)
{
	char *buf = calloc(REQ_SIZE, sizeof(char));
	memset(buf, 0, sizeof(*buf));
	ssize_t size;
	while ((size = read(conn, buf, REQ_SIZE)) > 0) {
		strcat(req, buf);
		char *tail = calloc(5, sizeof(char));
		memset(tail, 0, sizeof(&tail));

		// get str slice of last 4 chars
		snprintf(tail, 5 * sizeof(char), "%s", &req[strlen(req) - 4]);

		// if end of req msg, break out of reading loop
		if (strcmp(tail, "\r\n\r\n") == 0) {
			break;
		}
	}
}

static int parse_req(char *req, char **request_line)
{
	char *chunk;
	chunk = strtok(req, "\r\n");
	printf("chunk: %s\n", chunk);
	if (chunk == NULL) {
		puts("invalid request");
		return -1;
	} else {
		*request_line = strdup(chunk);
	}

	return 0;
}


void handle_conn(int conn)
{
	int msg_size = REQ_SIZE * 5;

	char *req = calloc(msg_size, sizeof(char));
	memset(req, 0, sizeof(*req));

	consume_req(conn, req);

	char *request_line = calloc(500, sizeof(char));

	if ((parse_req(req, &request_line) < 0)) {
		puts("failed to parse request");
		close(conn);
		exit(-1);
	};

	char *marker;
	strtok_r(request_line, " ", &marker);
	char *path = strtok_r(NULL, " ", &marker);
	puts("got a path");
	char *key = calloc(100, sizeof(char));
	char *val = calloc(100, sizeof(char));
	int size; 
	if ((size = sscanf(path, "/set?%[A-Za-z0-9\\-]=%[A-Za-z0-9\\-]", key, val)) != 0) {
		int result = put(key, val);
		printf("result: %d\n", result);
		printf("store: %s %s\n", key, val);
	} else if ((size = sscanf(path, "/get?%s", key)) != 0) {
		char *result = get(key);
		if (result == NULL) {
			puts("not found");
		} else {
			printf("found it: %s\n", result);
		} 
		
		printf("fetch: %s\n", key);
	} else {
		puts("invalid path");
	}

	close(conn);
}
