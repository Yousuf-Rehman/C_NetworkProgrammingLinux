#include<stdio.h>   //printf
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<string.h> //memset
#include<stdlib.h> //malloc
#include<fcntl.h> // for open
#include<unistd.h> // for close

#define MYPORT 4200
#define MyIP "192.168.10.21"
#define BACKLOG 10

int main(int argc, char *argv[]){
  printf(".......SERVER.........");
  int error;
  int sockfd; //socket file descriptor
  struct sockaddr_in server_addr;
  //getting socket
  printf("Creating Socket....");
  sockfd = socket(AF_INET, SOCK_STREAM,0);
  if(sockfd == -1){
    printf("socket creation failed...\n");
    exit(0);
  }
  else
    printf("Socket successfully created..\n");
  //putting values in sockaddr_in
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(MYPORT);
  server_addr.sin_addr.s_addr = inet_addr(MyIP);
  memset(&server_addr.sin_zero, '\0', 8);//zero all values
  
  printf("Binding Port = %d\n", MYPORT); 
  error = bind(sockfd,(struct sockaddr*)&server_addr,sizeof(struct sockaddr));
  if(error == -1){
    printf("bind error!!\n");
    exit(0);
  }
  
  printf("Listening to Port = %d\n", MYPORT);  
  error = listen(sockfd, BACKLOG);
  if(error == -1){
    printf("%s", "listen error");
    exit(0);
  }

  int new_fd;
  struct sockaddr_in client_addr;
  int sin_size = sizeof(struct sockaddr);

  new_fd = accept(sockfd,(struct sockaddr*)&client_addr, &sin_size);

  if(new_fd == -1)
    printf("%s", "accept error");

  char *rcvMsg = (char *)malloc(20*sizeof(char));
  int ByteRecv = 20;
  ByteRecv = recv(new_fd, rcvMsg, ByteRecv, 0);
  printf("%s", rcvMsg);
  close(new_fd);
  close(sockfd);
  
  return 0;
}
