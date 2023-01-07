#include <stdlib.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <assert.h>
#include "include/handler.h"
#include "include/sock.h"


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

		puts("got a connection");
		handle_conn(conn);
		close(conn);
	}
	close(sockfd);
	exit(0);
}