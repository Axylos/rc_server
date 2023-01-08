/**
I implemented the Database Server task in C using POSIX TCP sockets and standard lib utilities.

I'm still pretty new to C, so I chose to perform a task I understand pretty well (building a simple web server) in a context I'm less familiar with to learn more about the next level "down stack" from where I typically work.  I tested this out on linux using cURL and chrome to verify that the requirements have been met.

While I'm pretty sure the code is free of plain data corruption issues, there are a few quirks I noted in comments, mostly related to setup/teardown of the socket object.
Also, malicious or invalid input is not gracefully handled.

I really enjoyed working on this task and look forward to returning to it in the pairing session.
**/

#include <stdlib.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <assert.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

static int BACKLOG = 10;
const char *OK_STATUS = "HTTP/1.1 200 OK";
const char *CREATED_STATUS = "HTTP/1.1 201 Created";
const char *CONNECTION_H = "Connection: close";
const char *CONN_TYPE_H = "Content-Type: \"application/json\"";
const char *NOT_FOUND_STATUS = "HTTP/1.1 404 Not Found";

struct Node {
	char *key;
	char *val;
	struct Node *next;
};

static struct Node *head = NULL;

static int REQ_SIZE = 50000;

int put(char *key, char *val)
{
	if (head == NULL) {
		struct Node *node = malloc(sizeof(struct Node));
		node->key = strdup(key);
		node->val = strdup(val);
		node->next = NULL;
		head = node;
	} else {
		struct Node *prev = head;
		struct Node *next = head;
		do {
			if (strcmp(next->key, key) == 0) {
				free(next->val);
				next->val = strdup(val);
				return 2;
			}
			prev = next;
			next = prev->next;
		} while (next != NULL);

		struct Node *node = malloc(sizeof(struct Node));
		node->key = strdup(key);
		node->val = strdup(val);
		node->next = NULL;
		prev->next = node;
		return 1;
	}

	return 0;
}

char *get(char *key)
{
	struct Node *cur = head;
	while (cur != NULL) {
		if (strcmp(key, cur->key) == 0) {
			return cur->val;
		} else {
			cur = cur->next;
		}
	}
	return NULL;
}

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
		free(req);
		free(request_line);
		close(conn);
		return;
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
		int ret = put(key, val);
		printf("ret: %d\n", ret);
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

int setup_socket()
{
	int sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sockfd == -1) {
		perror("failed to create socket");
		exit(-1);
	}

	/**
	 this took some googling and is really a kludge
	 to deal with not properly tearing down the sock address
	**/
	int enable = 1;
	int ret = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (void *)&enable,
			     sizeof(int));
	if (ret != 0)
		perror("error setting sock opt");

	struct sockaddr_in addr;
	struct in_addr address;

	memset(&address, 0, sizeof(address));
	memset(&addr, 0, sizeof(addr));

	inet_aton("127.0.0.1", &address);

	addr.sin_family = AF_INET;
	addr.sin_port = htons(4000);
	addr.sin_addr = address;

	// still unsure how to "unbind" the socket
	// this requires enabling "rebinding"
	if ((bind(sockfd, (struct sockaddr *)&addr, sizeof(addr))) == -1) {
		perror("failed to bind socket");
		exit(-1);
	}

	if ((listen(sockfd, BACKLOG)) == -1) {
		perror("failed to listen to socket");
		exit(-1);
	}

	puts("bound to socket!");
	return sockfd;
}
int main(int argc, char *argv[])
{
	int sockfd = setup_socket();

	while (true) {
		struct sockaddr_in client_address;
		socklen_t ca_l = sizeof(client_address);

		int conn = accept(sockfd, (struct sockaddr *)&client_address,
				  &ca_l);

		if (conn == -1) {
			perror("failed to accept connection");
			exit(EPERM);
		}

		handle_conn(conn);
		close(conn);
	}
	close(sockfd);
	exit(0);
}