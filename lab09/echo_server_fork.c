#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#define MAXLINE 1024
#define PORTNUM 3600

struct trans_data
{
	char in_str[MAXLINE];
	int in_num;
}; // 입력받은 문자열과 정수 전달을 위한 구조체 지정(서버, 클라이언트 동일)

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

union semun
{
	int val;
};

int main(int argc, char **argv)
{
	int listen_fd, client_fd;
	pid_t pid;
	pid_t pid2;
	socklen_t addrlen;
	int readn;
	char buf[MAXLINE];
	struct sockaddr_in client_addr, server_addr;
	struct trans_data t_data;
	struct trans_data *temp;
	int shmid;
	int semid;
	void *shared_memory = NULL;
	union semun sem_union;
	struct sembuf semopen = {0, -1, SEM_UNDO};
	struct sembuf semclose = {0, 1, SEM_UNDO};


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
		addrlen = sizeof(client_addr);
		client_fd = accept(listen_fd,
						   (struct sockaddr *)&client_addr, &addrlen);
		if (client_fd == -1)
		{
			printf("accept error\n");
			break;
		}
		pid = fork();
		if (pid == 0) // 자식프로세스라면, Client의 자식프로세스
		{
			close(listen_fd);
			// memset(t_data.in_str, 0x00, sizeof(t_data.in_str));
			// memset(t_data.in_num, 0x00, sizeof(t_data.in_num));
			if ((readn = read(client_fd, (void *)&t_data, sizeof(t_data))) > 0)
			{
				printf("Read Data %s(%d) : %s %d\n", inet_ntoa(client_addr.sin_addr), getpid(), t_data.in_str, t_data.in_num);
				
				pid2 = fork(); // 값을 받은후 fork를 통해 클라이언트별로 부모프로세스(Producer), 자식프로세스(Consumer) 지정

				if (pid2 > 0){ // 부모프로세스라면 공유메모리에 계속 저장
					
					shmid = shmget((key_t)1234, sizeof(int), 0666|IPC_CREAT);
					semid = semget((key_t)3477, 1, IPC_CREAT|0666); // 세마포어 생성은 클라이언트별 부모프로세스(Producer)가 실행

					shared_memory = shmat(shmid, NULL, 0);
					temp = (struct trans_data*) shared_memory; // 공유메모리로 동일한 구조체 선언
					sem_union.val = 1;
					semctl( semid, 0, SETVAL, sem_union);
					while(1){
						semop(semid, &semopen, 1); // P(), wait
						t_data = func_HW(t_data);
						sleep(1);
						*temp = t_data;
						semop(semid, &semclose, 1); // V(), signal
					}
					
				}

				else if(pid2 ==0){ //자식프로세스라면 공유메모리에 있는 내용을 가져와서 무한히 송신
					shmid = shmget((key_t)1234, sizeof(int), 0666);
					semid = semget((key_t)3477, 0, 0666); // 클라이언트별 자식프로세스는 세마포어 생성 플래그 X
					shared_memory = shmat(shmid, NULL, 0);
					temp = (struct trans_data*) shared_memory; // 공유메모리로 동일한 구조체 선언
					while(1){
						semop(semid, &semopen, 1); // P(), wait
						// sleep(2);
						t_data = *temp; // 공유메모리로 부터 값을 전송할 값을 받아옴
						write(client_fd, (void *)&t_data, sizeof(t_data));
						semop(semid, &semclose, 1); // V(), signal
					}
				}

			}
			close(client_fd);
			return 0;
		}
		else if (pid > 0)
			close(client_fd);
	}
	return 0;
}
