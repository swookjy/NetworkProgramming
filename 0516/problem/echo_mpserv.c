/* using epoll, half-close, fget, fputs, etc., for message transmission */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/epoll.h>

#define BUF_SIZE 100
#define EPOLL_SIZE 50

void error_handling(char *buf)
{
    fputs(buf, stderr);
    fputc('\n', stderr);

    exit(1);
}

int main(int argc, char *argv[])
{
    int serv_sock, clnt_sock;
    struct sockaddr_in serv_adr, clnt_adr;
    socklen_t adr_sz;
    int str_len;
    int k = 0;
    char buf[BUF_SIZE];

    struct epoll_event *ep_events;
    struct epoll_event event;
    int epfd, event_cnt;

    FILE *readfp;
    FILE *writefp;
    int len;
    char buf2[BUF_SIZE];

    if (argc != 2)
    {
        printf("Usage : %s<port>\n", argv[0]);
        exit(1);
    }

    serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    memset(&serv_adr, 0, sizeof(serv_adr));

    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_adr.sin_port = htons(atoi(argv[1]));

    if (bind(serv_sock, (struct sockaddr *)&serv_adr, sizeof(serv_adr)) == -1)
        error_handling("bind() error");

    if (listen(serv_sock, 5) == -1)
        error_handling("listen() error");

    // create epoll instance (epoll file descriptor 저장소)
    epfd = epoll_create(EPOLL_SIZE);//return file descriptor

    ep_events = malloc(sizeof(struct epoll_event) * EPOLL_SIZE);
    
    // 구조체 epoll_event는 이벤트가 발생한 파일 디스크립터를 묶는 용도로도 사용됨
    event.events = EPOLLIN;         // 수신할 데이터가 존재하는 상황 -> 이벤트 발생
    event.data.fd = serv_sock;      // 서버 소켓에.

    // 관찰 대상이 되는 file descriptor 등록
    epoll_ctl(epfd, EPOLL_CTL_ADD, serv_sock, &event);
    // epfd(파일 디스크립터)에, serv_sock(파일 디스크립터)을 관찰대상으로 등록
    // event를 통해 전달된 이벤트의 관찰을 목적


    while (1)
    {
        // 두번째 인자는 동적으로 할당해야함. : 이벤트가 발생한 파일 디스크럽터가 채워질 버퍼의 주소
        event_cnt = epoll_wait(epfd, ep_events, EPOLL_SIZE, -1);
        // return : 이벤트가 발생한 파일 디스크럽터의 수
        // 두 번째 인자로 전달된 주소값의 버퍼에는, 이벤트가 발생한 "파일 디스크럽터의 정보"가 묶임 -> select()처럼 반복문 삽입 불필요

        if (event_cnt == -1) //epoll_wait() 호출 실패시 -1 return
        {
            puts("epoll_wait() error");
            break;
        }

        for (int i = 0; i < event_cnt; i++)
        {

            readfp = fdopen(ep_events[i].data.fd, "r");
            writefp = fdopen(dup(ep_events[i].data.fd), "w");
            //server에 이벤트 발생 : client연결
            if (ep_events[i].data.fd == serv_sock)
            {
                adr_sz = sizeof(clnt_adr);
                clnt_sock = accept(serv_sock, (struct sockaddr *)&clnt_adr, &adr_sz);
                
                //client socket에, 수신할 데이터가 존재하면 event 발생!
                event.events = EPOLLIN;     
                event.data.fd = clnt_sock;
                epoll_ctl(epfd, EPOLL_CTL_ADD, clnt_sock, &event);

                fputs("New Client connected...\n",stdout);
            }

            //client에 이벤트 발생 : echo!
            else
            {               
                fgets(buf, BUF_SIZE, readfp);   
                
                if(!strcmp(buf,"q\n") || !strcmp(buf,"Q\n")){
                    printf("client disconnected...\n");
                    epoll_ctl(epfd, EPOLL_CTL_DEL, ep_events[i].data.fd, NULL);
                    
                    shutdown(fileno(writefp), SHUT_WR);
                    fgets(buf, BUF_SIZE, readfp);
                    fputs(buf, stdout);
                }
                else
                {
                    fputs(buf, writefp);
                    fflush(writefp);
                }
            }
        }
    }
    fclose(readfp);
    fclose(writefp);

    close(serv_sock);   //close socket
    close(epfd);        //close file descriptor

    return 0;
}