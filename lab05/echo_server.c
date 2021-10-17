#include <sys/socket.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define MAXBUF 1024
struct trans_data{
	int max;
	struct in_addr max_IP;
	in_port_t max_PORT;
	int min;
	struct in_addr min_IP;
	in_port_t min_PORT;
	int avg;
};

struct pre_trans_data{
    int num;
    struct in_addr IP;
    in_port_t port;
};


struct pre_trans_data max(struct pre_trans_data *arr)
{ // 최댓값을 구하는 max함수 정의(입력 3개전용)
	int max = 0;
	struct pre_trans_data max_return;

	for (int i = 0; i <= 2; i++)
	{
		if (ntohl(arr[i].num) >= max)
			max = ntohl(arr[i].num);
			max_return = arr[i];
	}
	return max_return;
}

struct pre_trans_data min(struct pre_trans_data *arr)
{ // 최소값을 구하는 min함수 정의(입력 3개전용)
	int min = ntohl(max(arr).num); // 최소값을 구하기 위해 최대로 미리 정의
	struct pre_trans_data min_return;

	for (int i = 0; i <= 2; i++)
	{
		if (ntohl(arr[i].num) < min){

			min = ntohl(arr[i].num);
			min_return = arr[i];
			}
	}
	return min_return;
}

int avg(struct pre_trans_data *arr)
{ // 평균을 구하는 avg 함수 정의(입력 3개전용)
	int sum = 0;

	for (int i = 0; i <= 2; i++)
	{
		sum += ntohl(arr[i].num);
	}

	return sum / 3;
}


int main(int argc, char **argv)
{
	int server_sockfd, client_sockfd;
	int client_len, n;
	int cnt = 0;
	char buf[MAXBUF] = {
		0,
	};
	char buf_temp[MAXBUF]; // 임시 수신 버퍼
	// int buf[MAXBUF];
	int buf_int[3] = {
		0,
	}; // 임시 수신 버퍼(정수 변환)
	int arr_client_sock[3] = {
		0,
	};
	int arr_server_sock[3] = {
		0,
	};
	struct sockaddr_in clientaddr, serveraddr;
	struct trans_data t_data;
	struct pre_trans_data r_data;
	struct pre_trans_data arr_r_data[3];

	client_len = sizeof(clientaddr);
	for (int i = 0; i < 3;i++){
		if ((server_sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
		{
		perror("socket error : ");
		exit(0);
		}
		arr_server_sock[i] = server_sockfd;
	} // 3개의 서로 다른 포트로 접속을 허용하기 위해 3개의 포트를 열고, 정의한 배열에 server_sockfd를 저장
	
	memset(&serveraddr, 0x00, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);

	while (1)
	{
		for (cnt = 0; cnt < 3; cnt++)
		{
			serveraddr.sin_port = htons(atoi(argv[cnt+1])); // 바이너리 실행시 함께 입력되는 3개의 인자를 포트 번호로 받음
			
			bind(arr_server_sock[cnt], (struct sockaddr *)&serveraddr, sizeof(serveraddr));
			listen(arr_server_sock[cnt], 5);

			client_sockfd = accept(arr_server_sock[cnt], (struct sockaddr *)&clientaddr, &client_len);
			printf("New Client Connect[%d]: %s:%d\n", cnt, inet_ntoa(clientaddr.sin_addr), atoi(argv[cnt+1]));
			arr_client_sock[cnt] = client_sockfd;
		} // 3 클라이언트의 연결을 승낙하고, 클라이언트별 각기다른 client_sockfd를 생성한 정수 배열(arr_client_sock)에 차례대로 정의

		for (int i = 0; i < cnt; i++)
		{
			memset(buf_temp, 0x00, sizeof(buf_temp));
			if ((n = read(arr_client_sock[i], (void *)&r_data, sizeof(r_data))) <= 0) // 생성한 임시 구조체에 클라이언트 별로 메시지를 받아서 저장
			{
				close(arr_client_sock[i]);
				continue;
			}

			arr_r_data[i] = r_data; // 클라이언트로 부터 받은 메시지 구조체를 정의한 구조체 배열에 저장

			printf("Message from client[%d]: %d %s:%d \n", i, ntohl(r_data.num), inet_ntoa(r_data.IP), ntohs(r_data.port));
			// buf_int[i] = atoi(buf_temp); // 3개의 클라이언트로 부터 받은 정수를 정의한 배열에 저장
			// buf_temp[strlen(buf_temp)-1] = 0x00 ; // 0x00: NULL
			// strcat(buf, buf_temp); // 생성한 임시 버퍼와 모든 메시지의 합쳐진 메시지를 담을 버퍼를 concatenate

			if (i == 2)
			{
				// for(int i= 0; i<=2; i++)
				// 	printf("%d\n", buf_int[i]);

				printf("max: %d\n", ntohl(max(arr_r_data).num));
				printf("min: %d\n", ntohl(min(arr_r_data).num));
				printf("avg: %d\n", avg(arr_r_data));

				t_data.max = max(arr_r_data).num;
				t_data.max_IP = max(arr_r_data).IP;
				t_data.max_PORT = max(arr_r_data).port;
				t_data.min = min(arr_r_data).num;
				t_data.min_IP = min(arr_r_data).IP;
				t_data.min_PORT = min(arr_r_data).port;
				t_data.avg = htonl(avg(arr_r_data));

				for (int j = 0; j < cnt; j++)
				{
					if (write(arr_client_sock[j], (void *)&t_data, sizeof(t_data)) <= 0) // 합쳐진 메시지를 모든 클라이언트에 전달
					{
						perror("write error : ");
						close(arr_client_sock[j]);
					}
				}
			}
		}
	}
	close(server_sockfd);
	return 0;
}
