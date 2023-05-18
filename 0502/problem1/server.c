#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/select.h>

#define BUF_SIZE 100

void error_handling(char *buf)
{
	fputs(buf, stderr);
	fputc('\n', stderr);
	exit(1);
}

int main(int argc, char *argv[])
{
	int serv_sock, clnt_sock;
    int count = 0;
	struct sockaddr_in serv_adr, clnt_adr;
	struct timeval timeout;
	fd_set reads, cpy_reads;

	socklen_t adr_sz;
	int fd_max, str_len, fd_num, i;
	
    
    char buf1[BUF_SIZE];
    char buf2[BUF_SIZE];
    char buf3[BUF_SIZE];
	if(argc != 2) {
		printf("Usage : %s <port>\n", argv[0]);
		exit(1);
	}

	serv_sock = socket(PF_INET, SOCK_STREAM, 0);
	
    memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family = AF_INET;
	serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_adr.sin_port = htons(atoi(argv[1]));
	
	if(bind(serv_sock, (struct sockaddr*) &serv_adr, sizeof(serv_adr))==-1)
		error_handling("bind() error");
	if(listen(serv_sock, 5)==-1)
		error_handling("listen() error");

	FD_ZERO(&reads);
	FD_SET(serv_sock, &reads);
	fd_max=serv_sock;

	while(1)
	{
		cpy_reads = reads;
		timeout.tv_sec = 5;
		timeout.tv_usec = 5000;

		if((fd_num = select(fd_max+1, &cpy_reads, 0, 0, &timeout))==-1)//error
			break;
		
		if(fd_num == 0)//time out
			continue;

		for(i = 0; i < fd_max+1; i++)
		{
			if(FD_ISSET(i, &cpy_reads))//수신한 파일 디스크럽터 i
			{
				if(i == serv_sock)//서버가 수신했을 때 -> client 등록
				{
                    adr_sz = sizeof(clnt_adr);
					clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_adr, &adr_sz);
					
                    FD_SET(clnt_sock, &reads);//관리 대상에 등록
					if(fd_max < clnt_sock){
						fd_max = clnt_sock;
                    }
                    //새로 추가된 클라이언트는 cpy_reads엔 없고, reads엔 등록된 상태.

					printf("connected client: %d \n", clnt_sock);
                    
                    sprintf(buf1, "Server : Welcome~");
                    write(clnt_sock, buf1, sizeof(buf1));
                    sprintf(buf2, "Server : The number of clients is %d now", ++count);
                    write(clnt_sock, buf2, sizeof(buf2));

                    //기존에 다른 클라이언트가 있었을 경우, 해당 클라이언트에게 메세지 전송
                    sprintf(buf3, "Server : Client %d has joined this chatting room", clnt_sock);

                    // i : 서버 소켓
                    // clnt_sock : 새로 접속한 클라이언트 소켓.

                    for(int j=serv_sock+1; j<fd_max+1; j++){
                        
                        if(FD_ISSET(j, &reads)){
                            if(j == clnt_sock)  continue;
                            else    write(j, buf3, sizeof(buf3));
                        }
                    }
				}
				else//클라이언트가 수신했을 때
				{
                    char buf[BUF_SIZE];
                    memset(buf, 0, sizeof(buf));
					str_len = read(i, buf, BUF_SIZE);//i는 전송한 클라이언트 넘버

                    if(str_len == 0)//종료 요청
					{
						FD_CLR(i, &reads);
						close(i);
						printf("closed client: %d \n", i);
                        char buf_bye[BUF_SIZE];
                        sprintf(buf_bye, "Client %d has left this chatting room", i);
                        
                        for(int j=serv_sock+1; j<fd_max+1; j++){
                            if(FD_ISSET(j, &reads)){
                                if(j == i)  continue;
                                else    write(j, buf_bye, sizeof(buf_bye));//다른 클라이언트들에게 전송해야함
                            }
                        }
                        
                        count--;
					}
					
                    else//multicast!
					{
                        char buf_tmp[BUF_SIZE];
                        sprintf(buf_tmp, "client %d : %s", i, buf);

                        for(int j=serv_sock+1; j<fd_max+1; j++){
                        
                            if(FD_ISSET(j, &reads)){
                                if(j == i)  continue;
                                else    write(j, buf_tmp, sizeof(buf_tmp));
                            }
                        }
					}
                }
			}
		}
	}
    
	close(serv_sock);
	return 0;
}





/*주고받는 data가 사라지지 않음. buffer 비우기만 하면 될듯*/