#include <sys/socket.h>  /* 소켓 관련 함수 */
#include <arpa/inet.h>   /* 소켓 지원을 위한 각종 함수 */
#include <sys/stat.h>
#include <stdio.h>      /* 표준 입출력 관련 */
#include <string.h>     /* 문자열 관련 */
#include <unistd.h>     /* 각종 시스템 함수 */
#include <stdlib.h>

#define MAXLINE    1024

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
    in_addr_t IP;
    in_port_t port;
};



int main(int argc, char **argv)
{
    struct sockaddr_in serveraddr;
    struct pre_trans_data s_data;
    struct trans_data r_data;
    int server_sockfd;
    int client_len;
    char buf[MAXLINE];
    char *myIP = "127.0.0.1";

    if ((server_sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) 
    {
        perror("error :");
        return 1;
    }

    /* 연결요청할 서버의 주소와 포트번호 프로토콜등을 지정한다. */
    server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serveraddr.sin_port = htons(atoi(argv[1]));

    client_len = sizeof(serveraddr);

    /* 서버에 연결을 시도한다. */
    if (connect(server_sockfd, (struct sockaddr *)&serveraddr, client_len)  == -1)
    {
        perror("connect error :");
        return 1;
    }

    memset(buf, 0x00, MAXLINE);
    read(0, buf, sizeof(buf));    /* 키보드 입력을 기다린다. */
    s_data.num = htonl(atoi(buf)); // 클라이언트의 키보드 입력값
    s_data.IP = inet_addr("127.0.0.1"); // 클라이언트가 정의한 본인의 IP
    s_data.port =  htons(atoi(argv[1])); // 클라이언트가 바이너리 실행 시 동시에 인자로 입력한 포트번호
    // 전송을 위한 바이트형태로의 변환


    if (write(server_sockfd, (void *)&s_data, sizeof(s_data)) <= 0) /* 입력 받은 데이터를 서버로 전송한다. */
    {
        perror("write error : ");
        return 1;
    }
    // memset(buf, 0x00, MAXLINE);
    /* 서버로 부터 데이터를 읽는다. */
    if (read(server_sockfd, (void *)&r_data, sizeof(r_data)) <= 0)
    {
        perror("read error : ");
        return 1;
    }
    printf("max : %d from %s:%d\n", ntohl(r_data.max), inet_ntoa(r_data.max_IP), ntohs(r_data.max_PORT));
    printf("min : %d from %s:%d\n", ntohl(r_data.min), inet_ntoa(r_data.min_IP), ntohs(r_data.min_PORT));
    printf("avg : %d\n", ntohl(r_data.avg));
    close(server_sockfd);
    return 0;
}
