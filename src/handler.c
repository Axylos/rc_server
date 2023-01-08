#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "include/store.h"
#include "include/resp_bits.h"

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
	if (chunk == NULL) {
		puts("invalid request");
		return -1;
	} else {
		*request_line = strdup(chunk);
	}

	return 0;
}

static char *build_set_resp()
{
	char *len = "Content-Length: 0";
	ssize_t size = 500 * sizeof(char);
	char *resp = malloc(size);
	memset(resp, 0, size);
	snprintf(resp, size, "%s\r\n%s\r\n%s\r\n\r\n", CREATED_STATUS,
		 CONNECTION_H, len);

	return resp;
}

static char *build_not_found(char *key)
{
	char *body = calloc(600, sizeof(char));
	memset(body, 0, 600 * sizeof(char));
	snprintf(body, 600 * sizeof(char), "Key: %s was not found", key);
	size_t body_l = strlen(body);
	char *len_h = calloc(50, sizeof(char));
	snprintf(len_h, 50 * sizeof(char), "Content-Length: %lu", body_l);
	ssize_t size = 500 * sizeof(char);
	char *resp = malloc(size);
	memset(resp, 0, size);
	snprintf(resp, size, "%s\r\n%s\r\n%s\r\n\r\n%s", NOT_FOUND_STATUS,
		 CONNECTION_H, len_h, body);

	free(body);
	free(len_h);
	return resp;
}

static char *build_invalid_path()
{
	char *len = "Content-Length: 0";
	ssize_t size = 500 * sizeof(char);
	char *resp = malloc(size);
	memset(resp, 0, size);
	snprintf(resp, size, "%s\r\n%s\r\n%s\r\n\r\n", NOT_FOUND_STATUS,
		 CONNECTION_H, len);

	return resp;
}

static char *build_found(char *result)
{
	ssize_t body_size = 500 * sizeof(char);

	char *body = malloc(body_size);
	memset(body, 0, body_size);

	snprintf(body, body_size, "{\"result\": \"%s\"}", result);

	size_t body_len = strlen(body);
	char *len_h = malloc(100 * sizeof(char));
	memset(len_h, 0, 100 * sizeof(char));

	// snprintf is overkill; building the content-length header should probably use a macro
	snprintf(len_h, 100 * sizeof(char), "Content-Length: %lu", body_len);

	ssize_t size = 1000 * sizeof(char);
	char *resp = malloc(size);

	snprintf(resp, size, "%s\r\n%s\r\n%s\r\n%s\r\n\r\n%s", OK_STATUS,
		 CONNECTION_H, len_h, CONN_TYPE_H, body);

	free(len_h);
	free(body);

	return resp;
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
	char *key = calloc(100, sizeof(char));
	char *val = calloc(100, sizeof(char));
	char *resp;

	// definitely nota safe or flexible way to handle url encoded values
	if (sscanf(path, "/set?%[A-Za-z0-9\\-]=%[A-Za-z0-9\\-]", key, val) !=
	    0) {
		put(key, val);
		resp = build_set_resp();
	} else if (sscanf(path, "/get?%s", key) != 0) {
		char *result = get(key);
		if (result == NULL) {
			resp = build_not_found(key);
		} else {
			resp = build_found(result);
		}

	} else {
		resp = build_invalid_path();
		puts("invalid path");
	}
	ssize_t written = write(conn, resp, strlen(resp));
	printf("%lu bytes written\n", written);
	free(key);
	free(val);
	free(resp);
	free(request_line);
	free(req);

	close(conn);
}
