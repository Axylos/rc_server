#include "include/sock.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

static int BACKLOG = 10;

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
