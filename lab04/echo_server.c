#include <sys/socket.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define MAXBUF 1024

int main(int argc, char **argv)
{
	int server_sockfd, client_sockfd;
	int client_len, n;
	int cnt = 0;
	char buf[] = {0, };
	struct sockaddr_in clientaddr, serveraddr;
	client_len = sizeof(clientaddr);
	if ((server_sockfd = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP )) == -1)
	{
		perror("socket error : ");
		exit(0);
	}
	memset(&serveraddr, 0x00, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(atoi(argv[1]));

	bind (server_sockfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
	listen(server_sockfd, 5);

	while(1)
	{
		while(1){
			client_sockfd = accept(server_sockfd, (struct sockaddr *)&clientaddr,
			&client_len);
			printf("New Client Connect[%d]: %s\n", cnt, inet_ntoa(clientaddr.sin_addr));
			cnt ++;
			
			if ((n = read(client_sockfd, buf, MAXBUF)) <= 0){
			close(client_sockfd);
			continue;
			}
			printf(" %s, %ld ", buf, sizeof(buf) );

			if(cnt == 3)
				break;
		}
		
		// memset(buf, 0x00, MAXBUF);


		if (write(client_sockfd, buf, MAXBUF) <=0)
		{
			perror("write error : ");
			close(client_sockfd);
		}
		close(client_sockfd);
		break;
	}
	close(server_sockfd);
	return 0;
}
