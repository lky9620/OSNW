#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/types.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAXLINE 1024
#define PORTNUM 3600
#define SOCK_SETSIZE 1021

struct trans_data
{
	char in_str[MAXLINE];
	int in_num;
}; // 입력받은 문자열과 정수 전달을 위한 구조체 지정(서버, 클라이언트 동일)

struct trans_data func_split(char data[MAXLINE])
{
	struct trans_data return_data;
	int space;
	char temp[MAXLINE];
	int j = 0;

	memset(temp, 0x00, MAXLINE);

	for (int i = 0; i <= MAXLINE; i++)
	{
		if (data[i] == 0x20)
		{
			space = i;
			break;
		}
	} // 사용자로부터 입력받은 문자열의 공백 위치 탐색

	for (int i = 0; i <= space; i++)
	{
		return_data.in_str[i] = data[i];
	} // space 이전의 문자열은 문자열로 정의

	for (int i = space + 1; i <= MAXLINE; i++)
	{
		temp[j] = data[i];
		if (data[i] == 0x0D)
			break;
		j++;
	} // 문자열 이후 부터 엔터 이전까지는 정수로 취급

	return_data.in_num = atoi(temp); // 정수로 변환하여 리턴

	return return_data;

}; // 사용자로부터 입력받아 전송받은 문자열을 스페이스 문자를 중심으로 문자열과 정수로 split하여 구조체로 정의

struct trans_data func_concat(struct trans_data *arr)
{
	struct trans_data return_data;
	char temp_char[MAXLINE];
	int temp_int = 0;
	char temp_num[MAXLINE];

	memset(temp_char, 0x00, MAXLINE);

	// printf("data1 %s, %d\n", arr[0].in_str, arr[0].in_num);
	// printf("data2 %s, %d\n", arr[1].in_str, arr[1].in_num);
	// printf("data3 %s, %d\n", arr[2].in_str, arr[2].in_num);
	for (int i = 0; i < 3; i++)
	{
		strcat(temp_char, arr[i].in_str);
		temp_int += arr[i].in_num;
		temp_char[strlen(temp_char) + 1] = 0x20;
		if (arr[i + 1].in_str[0] == 0x00)
			break;

	} // 문자열 합치기

	for (int i = 0; i < sizeof(temp_char); i++)
		return_data.in_str[i] = temp_char[i]; // 정수 합치기

	return_data.in_num = temp_int;

	// printf("data %s, %d \n", return_data.in_str, return_data.in_num);

	return return_data;
}

int main(int argc, char **argv)
{
	int listen_fd, client_fd;
	socklen_t addrlen;
	int fd_num;
	int maxfd = 0;
	int sockfd;
	int i = 0;
	int j = 0;
	char buf[MAXLINE];
	char num_temp[MAXLINE];
	struct timeval time;
	fd_set readfds, allfds;
	struct trans_data t_data;
	struct trans_data temp[3] = {
		0,
	};
	int init_sock = 0; // 최초 값을 위해 연결할 클라이언트의 수보다 1크게 지정

	struct sockaddr_in server_addr, client_addr;

	if ((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		perror("socket error");
		return 1;
	}
	memset((void *)&server_addr, 0x00, sizeof(server_addr));
	memset(buf, 0x00, MAXLINE);
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

	FD_ZERO(&readfds);
	FD_SET(listen_fd, &readfds);

	maxfd = listen_fd;
	while (1)
	{
		allfds = readfds;
		// printf("Select Wait %d\n", maxfd);

		time.tv_sec = 3;
		time.tv_usec = 0;
		fd_num = select(maxfd + 1, &allfds, (fd_set *)0, (fd_set *)0, &time); // time의 시간마다 반복적으로 전송

		if (fd_num == 0)
		{
			if(buf[0]!=0){
				for (int k = init_sock; k <= maxfd; k++)
					write(k, buf, sizeof(buf));
			}
			
		} // 아무런 입력이 없을 경우 timeout 되는 시간마다 기존 buf의 내용을 연결된 모든 소켓에 전송

		else
		{
			if (FD_ISSET(listen_fd, &allfds))
			{
				addrlen = sizeof(client_addr);
				client_fd = accept(listen_fd, (struct sockaddr *)&client_addr, &addrlen);

				FD_SET(client_fd, &readfds);

				if (client_fd > maxfd)
					maxfd = client_fd;
				continue;
			}
			for (i = 0; i <= maxfd; i++)
			{
				sockfd = i;
				if (FD_ISSET(sockfd, &allfds))
				{
					memset(buf, 0x00, MAXLINE);
					if (read(sockfd, buf, MAXLINE) <= 0)
					{
						close(sockfd);
						FD_CLR(sockfd, &readfds);
					}

					else
					{
						if (strncmp(buf, "quit\n", 5) == 0)
						{
							close(sockfd);
							FD_CLR(sockfd, &readfds);
						}
						else
						{
							t_data = func_split(buf); // 문자열과 정수 split하여 구조체로 지정

							if (init_sock == 0)
							{
								init_sock = sockfd; // 최초의 소켓 디스크립터 정수를 기억
							}

							temp[maxfd - init_sock] = t_data;

							printf("Read(%d) : %s, %d \n", sockfd, t_data.in_str, t_data.in_num);

							t_data = func_concat(temp); // 클라이언트별로 문자열은 문자열끼리 합치고, 정수는 정수끼리 더함

							memset(buf, 0x00, MAXLINE);

							strcat(buf, t_data.in_str);
							// buf[strlen(buf) + 1] = 0x20;
							sprintf(num_temp, "%d", t_data.in_num);
							strcat(buf, num_temp); // 합쳐진 문자열과 더해진 정수가 포함된 구조체를 한개의 문자열로 정의 

							printf("send data: %s\n", buf);

							for (int k = init_sock; k <= maxfd; k++)
							{
								write(k, buf, sizeof(buf));
							}
						}
					}
					if (--fd_num <= 0)
						break;
				}
			}
		}
	}
}
