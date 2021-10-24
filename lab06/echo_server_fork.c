#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#define MAXLINE 1024
#define PORTNUM 3600

int main(int argc, char **argv)
{
	int listen_fd, client_fd;
	pid_t pid;
	socklen_t addrlen;
	int readn;
	int cnt = 0;
	char buf[MAXLINE];
	char buf_temp[MAXLINE]; // 임시 수신 버퍼
	int arr_client_sock[3] = {
		0,
	};
	struct sockaddr_in client_addr, server_addr;

	if ((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		return 1;
	}
	memset((void *)&server_addr, 0x00, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(PORTNUM);

	if (bind(listen_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
	{
		perror("bind error");
		return 1;
	}
	if (listen(listen_fd, 5) == -1)
	{
		perror("listen error");
		return 1;
	}

	signal(SIGCHLD, SIG_IGN);
	while (1)
	{
		for (cnt = 0; cnt < 3; cnt++)
		{
			addrlen = sizeof(client_addr);
			client_fd = accept(listen_fd,
							   (struct sockaddr *)&client_addr, &addrlen);
			if (client_fd == -1)
			{
				printf("accept error\n");
				break;
			}
			printf("New Client Connect[%d]: %s\n", cnt, inet_ntoa(client_addr.sin_addr));
			arr_client_sock[cnt] = client_fd;
		} // 3 클라이언트의 연결을 승낙하고, 클라이언트별 각기다른 client_sockfd를 생성한 정수 배열(arr_client_sock)에 차례대로 정의
		for (int i = 0; i < cnt; i++)
		{
			memset(buf_temp, 0x00, sizeof(buf_temp));
			if ((readn = read(arr_client_sock[i], buf_temp, sizeof(buf_temp))) <= 0) // 생성한 임시 버퍼에 클라이언트 별로 메시지를 받아서 저장
			{
				close(arr_client_sock[i]);
				continue;
			}

			printf("Message from client[%d]: %s ", i, buf_temp);

			buf_temp[strlen(buf_temp) - 1] = 0x20; // 0x20: Space ASCII Code
			strcat(buf, buf_temp);				   // 생성한 임시 버퍼와 모든 메시지의 합쳐진 메시지를 담을 버퍼를 concatenate
			pid = fork();
			if (pid == 0)
			{
				close(listen_fd);
				if (i == 2)
				{
					for (int j = 0; j < cnt; j++)
					{
						write(arr_client_sock[j], buf, strlen(buf));
						close(arr_client_sock[j]);
					}

				}
			}
		}
		sleep(5);
		return 0;
	}
}
// 	while (1)
// 	{
// 		addrlen = sizeof(client_addr);
// 		client_fd = accept(listen_fd,
// 						   (struct sockaddr *)&client_addr, &addrlen);
// 		if (client_fd == -1)
// 		{
// 			printf("accept error\n");
// 			break;
// 		}
// 		pid = fork();
// 		if (pid == 0)
// 		{
// 			close(listen_fd);
// 			memset(buf, 0x00, MAXLINE);
// 			while ((readn = read(client_fd, buf, MAXLINE)) > 0)
// 			{
// 				printf("Read Data %s(%d) : %s",
// 					   inet_ntoa(client_addr.sin_addr),
// 					   client_addr.sin_port,
// 					   buf);
// 				write(client_fd, buf, strlen(buf));
// 				memset(buf, 0x00, MAXLINE);
// 			}
// 			close(client_fd);
// 			return 0;
// 		}
// 		else if (pid > 0)
// 			close(client_fd);
// 	}
// 	return 0;
// }
