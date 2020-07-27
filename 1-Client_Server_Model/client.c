#include<stdio.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<string.h>
#include<fcntl.h> // for open
#include<unistd.h> // for close
#include <stdlib.h>

#define DEST_IP "192.168.10.21"
#define DEST_PORT 4200

void main(){
  printf(".......CLIENT.........\n");
  int sockfd;
  struct sockaddr_in dest_addr;
  
  sockfd = socket(PF_INET, SOCK_STREAM, 0);
  if(sockfd == -1){
    printf("socket creation failed...\n");
    exit(0);
  }
  else
    printf("Socket successfully created..\n");
  //putting dest values in sockaddr_in
  dest_addr.sin_family = AF_INET;
  dest_addr.sin_port = htons(DEST_PORT);
  dest_addr.sin_addr.s_addr = inet_addr(DEST_IP);
  memset(&dest_addr.sin_zero, '\0',8);
  
  printf("Connecting....");
  connect(sockfd, (struct sockaddr*)&dest_addr, sizeof(struct sockaddr));
  //printf("%s", "connect error");
  printf("Connected");
  int ByteSend = 20;
  char *SendMsg = "Hi";
  send(sockfd, SendMsg, ByteSend, 0);
  close(sockfd);
}
