#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <cstring>


int main(int argc, char const* argv[])
{
    int sockD = socket(AF_INET, SOCK_STREAM, 0);
  
    struct sockaddr_in servAddr;
  
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(8089);
    servAddr.sin_addr.s_addr = INADDR_ANY;
  
    int connectStatus
        = connect(sockD, (struct sockaddr*)&servAddr,
                  sizeof(servAddr));
  
    if (connectStatus == -1) {
        printf("connect\n");
        return 1;
    }
  
    char request[] = {"Hello, echo-server"};

    if (write(sockD, request, strlen(request)) == -1) {
        printf("write\n");
        return 1;
    }
    
    char response[255] = {};
    auto si = read(sockD, response, sizeof(response) - 1);
    if (si == -1) {
        printf("read\n");
        return 1;
    }

    response[si] = 0;
    printf("%s\n", response);
    
  
    return 0;
}