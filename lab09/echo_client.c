#include <sys/socket.h>  /* 소켓 관련 함수 */
#include <arpa/inet.h>   /* 소켓 지원을 위한 각종 함수 */
#include <sys/stat.h>
#include <stdio.h>      /* 표준 입출력 관련 */
#include <string.h>     /* 문자열 관련 */
#include <unistd.h>     /* 각종 시스템 함수 */
#include <stdlib.h>

#define MAXLINE    1024

struct trans_data{
    char in_str[MAXLINE];
    int in_num;
};  // 입력받은 문자열과 정수 전달을 위한 구조체 지정(서버, 클라이언트 동일)

int main(int argc, char **argv)
{
    struct sockaddr_in serveraddr;
    int server_sockfd;
    int client_len;
    char buf[MAXLINE];
    struct trans_data t_data;

    if ((server_sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) 
    {
        perror("error :");
        return 1;
    }

    /* 연결요청할 서버의 주소와 포트번호 프로토콜등을 지정한다. */
    server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serveraddr.sin_port = htons(3600);

    client_len = sizeof(serveraddr);

    /* 서버에 연결을 시도한다. */
    if (connect(server_sockfd, (struct sockaddr *)&serveraddr, client_len)  == -1)
    {
        perror("connect error :");
        return 1;
    }

    memset(buf, 0x00, MAXLINE);
    printf("input string: ");
    scanf("%s", buf); // 입력받을 string

    strcpy(t_data.in_str, buf);

    memset(buf, 0x00, MAXLINE);

    printf("input integer: ");
    scanf("%s", buf); // 입력받을 inteager
    t_data.in_num =  atoi(buf);


    if (write(server_sockfd, (void *)&t_data, sizeof(t_data)) <= 0) /* 입력 받은 데이터를 서버로 전송한다. */
    {
        perror("write error : ");
        return 1;
    }
    memset(buf, 0x00, MAXLINE);
    /* 서버로 부터 데이터를 읽는다. */
    while (read(server_sockfd, (void *)&t_data, sizeof(t_data)) > 0) // 서버가 보내는 값이 있다면 무한히 읽고, 출력
    {
        printf("read : %s and %d \n", t_data.in_str, t_data.in_num);
    }
    close(server_sockfd);
    return 0;
}
