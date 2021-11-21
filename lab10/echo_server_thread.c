#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>

#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define MAXLINE 1024
#define PORTNUM 3600
struct trans_data{
    char in_str[MAXLINE];
    int in_num;
};  // 입력받은 문자열과 정수 전달을 위한 구조체 지정(서버, 클라이언트 동일)


struct trans_data func_HW(struct trans_data data) // 전송받은 문자열과 정수 구조체를 입력으로 받음
{

	int len = strlen(data.in_str);
	char temp[MAXLINE];
	int i;

	memset(temp, 0x00, sizeof(temp));

	for (i = 0; i < len; i++)
		temp[i] = data.in_str[i + 1]; // 전송받은 문자열의 첫글자를 제외하고 모두 한칸씩 shift left

	temp[strlen(temp)] = data.in_str[0]; // 리턴할 문자열의 마지막은 전송받은 문자열의 첫글자

	strcpy(data.in_str, temp);
	data.in_num += 1; // 리턴할 정수는 전송받은 정수의 +1 값

	return data;
}; // 클라이언트로 부터 받은 문자열은 순환적으로 rotate 되며, 정수는 한개씩 커지는 값을 반환하는 함수 생성

void  * thread_consumer(void *data);

pthread_mutex_t t_lock;
int i = 0; // 클라이언트의 인덱스
struct trans_data t_data[5]; // 클라이언트별 전송받은 문자열과 정수의 구조체 배열

void * thread_func(void *data) // 생산자 스레드, 클라이언트로부터 입력받은 문자열과 정수 구조체를 func_HW 함수에 입력시켜 리턴값을 계속하여 생산
{
	int sockfd = *((int *)data);
	int readn;
	socklen_t addrlen;
	char buf[MAXLINE];
	struct sockaddr_in client_addr;
	pthread_t thread_id;
	// struct trans_data t_data;
	int mkflg = 0; // 소비자 프로세스의 1회 생성을 위한 플래그
	int blk = i;
	memset(buf, 0x00, MAXLINE);
	addrlen = sizeof(client_addr);
	getpeername(sockfd, (struct sockaddr *)&client_addr, &addrlen);
	while ((readn = read(sockfd, (void *)&t_data[blk], sizeof(t_data[blk]))) > 0)
	{
		printf("Read Data %s(%d) : %s %d \n", inet_ntoa(client_addr.sin_addr),  ntohs(client_addr.sin_port),t_data[blk].in_str, t_data[blk].in_num);
		while(1){
			pthread_mutex_lock(&t_lock); // 프로세스 자원 lock 
			t_data[blk] =  func_HW(t_data[blk]);
			// write(sockfd, (void *)&t_data, sizeof(t_data));
			
			if (mkflg ==0){
				pthread_create(&thread_id, NULL, thread_consumer, &sockfd);
				pthread_detach(thread_id);
			}
			mkflg = 1;
			pthread_mutex_unlock(&t_lock); // 프로세스 자원 unlock 
			sleep(1);
		}
	}
	close(sockfd);
	printf("worker thread end\n");
	return 0;
}

void  * thread_consumer(void *data) // 소비자 스레드 
{
	int sockfd = *((int *)data);
	int readn;
	socklen_t addrlen;
	char buf[MAXLINE];
	struct sockaddr_in client_addr;
	int blk = i;

	addrlen = sizeof(client_addr);
	getpeername(sockfd, (struct sockaddr *)&client_addr, &addrlen);

	while(1){
		pthread_mutex_lock(&t_lock); // 프로세스 자원 lock 
		write(sockfd, (void *)&t_data[blk], sizeof(t_data[blk])); // 클라이언트에게 생산자가 처리한 문자열과 정수 구조체를 전달
		pthread_mutex_unlock(&t_lock); // 프로세스 자원 unlock 
	}

}

int main(int argc, char **argv)
{
	int listen_fd, client_fd;
	socklen_t addrlen;
	int readn;
	char buf[MAXLINE];
	pthread_t thread_id;

	struct sockaddr_in server_addr, client_addr;

	if( (listen_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		return 1;
	}
	memset((void *)&server_addr, 0x00, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(PORTNUM);

	if(bind(listen_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) ==-1	)
	{
		perror("bind error");
		return 1;
	}
	if(listen(listen_fd, 5) == -1)
	{
		perror("listen error");
		return 1;
	}

	while(1)
	{
		addrlen = sizeof(client_addr);
		client_fd = accept(listen_fd,
			(struct sockaddr *)&client_addr, &addrlen);
		printf("new client connected from %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
		if(client_fd == -1)
		{
			printf("accept error\n");
		}
		else
		{
			pthread_create(&thread_id, NULL, thread_func, (void *)&client_fd); // 생산자 스레드 생성
			pthread_detach(thread_id);
			i +=1; // 클라이언트의 인덱스 증가
		}
	}
	return 0;
}

