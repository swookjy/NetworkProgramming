#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>

void error_handling(char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}

int main(int argc, char *argv[])
{
    char *addr = "127.232.124.79";//char형 포인터 : 첫번째 문자의 주소값이 저장됨.
    struct sockaddr_in addr_inet;


    // int inet_aton(const char *string, struct in_addr *addr);
    // 1. 변환할 IP주소 정보를 담고 있는 문자열의 주소값
    // 2. 변환될 정보를 저장할 in_addr 구조체 변수의 주소값
    // 성공시 1, 실패시 0 반환

    if(!inet_aton(addr, &addr_inet.sin_addr)){
        error_handling("Conversion error");
    }
    else{
        printf("Network ordered integer addr: %#x \n", addr_inet.sin_addr.s_addr);
    }
    return 0;
}
