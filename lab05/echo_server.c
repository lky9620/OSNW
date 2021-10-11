#include <sys/socket.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define MAXBUF 1024

int max(int *arr){  // 최댓값을 구하는 max함수 정의
	int max = 0;

	for(int i = 0; i<=2 ; i++){
		if (arr[i]>= max)
			max = arr[i];
	}
		return max;
}

int min(int *arr){  // 최소값을 구하는 min함수 정의
	int min = max(arr);

	for(int i = 0; i<=2 ; i++){
		if (arr[i]<= min)
			min = arr[i];
	}
		return min;
}

int avg(int *arr){ // 평균을 구하는 avg 함수 정의
	int sum = 0;

	for(int i = 0; i<=2; i++){
		sum += arr[i];
	}

	return sum/3;
}



int main(int argc, char **argv)
{
	int server_sockfd, client_sockfd;
	int client_len, n;
	int cnt = 0;
	char buf[MAXBUF] = {0,};
	char buf_temp[MAXBUF]; // 임시 수신 버퍼
	// int buf[MAXBUF];
	int buf_int[3] = {0,}; // 임시 수신 버퍼(정수 변환)
	int arr_client_sock[3] = {0, };
	struct sockaddr_in clientaddr, serveraddr;

	client_len = sizeof(clientaddr);
	if ((server_sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
	{
		perror("socket error : ");
		exit(0);
	}
	memset(&serveraddr, 0x00, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(atoi(argv[1]));

	bind(server_sockfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
	listen(server_sockfd, 5);

	while (1)
	{
		for (cnt = 0; cnt < 3; cnt++)
		{
			client_sockfd = accept(server_sockfd, (struct sockaddr *)&clientaddr, &client_len);
			printf("New Client Connect[%d]: %s\n", cnt, inet_ntoa(clientaddr.sin_addr));
			arr_client_sock[cnt] = client_sockfd;
		} // 3 클라이언트의 연결을 승낙하고, 클라이언트별 각기다른 client_sockfd를 생성한 정수 배열(arr_client_sock)에 차례대로 정의

		for (int i = 0; i < cnt; i++)
		{
			memset(buf_temp, 0x00, sizeof(buf_temp));
			if ((n = read(arr_client_sock[i], buf_temp, sizeof(buf_temp))) <= 0) // 생성한 임시 버퍼에 클라이언트 별로 메시지를 받아서 저장
			{
				close(arr_client_sock[i]);
				continue;
			}

			printf("Message from client[%d]: %s ", i, buf_temp);
			buf_int[i] = atoi(buf_temp);
			// buf_temp[strlen(buf_temp)-1] = 0x00 ; // 0x00: NULL
			// strcat(buf, buf_temp); // 생성한 임시 버퍼와 모든 메시지의 합쳐진 메시지를 담을 버퍼를 concatenate

			if (i == 2)
			{
				// for(int i= 0; i<=2; i++)
				// 	printf("%d\n", buf_int[i]);
				
				printf("max: %d\n", max(buf_int));
				printf("min: %d\n", min(buf_int));
				printf("avg: %d\n", avg(buf_int));
				printf("Sending Message is : %s", buf);

				for (int j = 0; j < cnt; j++)
				{
					if (write(arr_client_sock[j], buf, sizeof(buf)) <= 0) // 합쳐진 메시지를 모든 클라이언트에 전달
					{
						perror("write error : ");
						close(arr_client_sock[j]);
					}
				}
			}
		}
		// memset(buf, 0x00, MAXBUF);
	}
	close(server_sockfd);
	return 0;
}
