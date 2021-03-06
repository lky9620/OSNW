#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
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
	char buf[MAXLINE];
	char buf_temp[MAXLINE];
	struct sockaddr_in client_addr, server_addr;
	int fdA[2], fdB[2];
	int i = 0;
	if (pipe(fdA) < 0)
	{
		perror("pipe error : ");
		return 1;
	}
	if (pipe(fdB) < 0)
	{
		perror("pipe error : ");
		return 1;
	}
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
	memset(buf_temp, 0x00, MAXLINE);
	memset(buf, 0x00, MAXLINE);
	signal(SIGCHLD, SIG_IGN);
	while (1)
	{
		addrlen = sizeof(client_addr);
		client_fd = accept(listen_fd,
						   (struct sockaddr *)&client_addr, &addrlen);
		if (client_fd == -1)
		{
			printf("accept error\n");
			break;
		}

		pid = fork();
		if (pid == 0) // 자식프로세스
		{
			close(listen_fd);
			// close(fdA[1]);
			// close(fdB[0]);
			printf("Client (pid: %d) Connected \n", getpid());

			while ((readn = read(client_fd, buf_temp, MAXLINE)) > 0)
			{
				printf("Read Data %s : %s", inet_ntoa(client_addr.sin_addr), buf_temp);

				write(fdB[1], buf_temp, sizeof(buf_temp)); // 자식프로세스가 클라이언트로부터 수신받은 자료를 자식프로세스 입장에서의 쓰기 파이프에 씀
				if ((readn = read(fdA[0], buf, sizeof(buf)) > 0)) // 자식프로세스 입장에서 읽기 파이프에 읽을 자료가 있을 경우
					write(client_fd, buf, sizeof(buf)); // 읽은 병합된 자료를 클라이언트에게 송신
			}
			close(client_fd);
			return 0;
		}
		else if (pid > 0) // 부모프로세스
		{
			close(client_fd);
			// close(fdA[0]);
			// close(fdB[1]);
			if ((readn = read(fdB[0], buf_temp, sizeof(buf_temp))) > 0) // 부모프로세스 입장에서 읽기 파이프에 읽을 자료가 있을 경우
			{
				i++;
				buf_temp[strlen(buf_temp) - 1] = 0x20; // 0x20: Space ASCII Code
				strcat(buf, buf_temp); // 생성한 임시 버퍼와 모든 메시지의 합쳐진 메시지를 담을 버퍼를 concat
				if (i == 3) // 3개의 자식 프로세스가 클라이언트로부터 자료 수신을 완료 했을 경우
				{
					for(int j = 0;j<3;j++)
						write(fdA[1], buf, sizeof(buf)); // 부모프로세스 입장에서 쓰기 파이프에 병합된 메시지를 씀(fork된 자식 프로세스가 3개이므로 3번 write를 수행)
					printf("Concat data is %s\n", buf);
				}
			}
		}
	}
	return 0;
}
