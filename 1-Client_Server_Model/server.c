#include<stdio.h>   //printf
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<string.h> //memset, strcmp
#include<stdlib.h> //malloc
#include<fcntl.h> //for open
#include<unistd.h> //for close
#include<time.h> //To use time library of C
#include<signal.h>


#define MYPORT 4200
#define MyIP "192.168.10.21"
#define BACKLOG 10

int sockfd;//socket file descriptor
int new_fd;
/* Signal Handler for SIGINT */
void sigintHandler(int sig_num) 
{ 
  /* Reset handler to catch SIGINT next time. SIGINT when ctrl+c pressed*/
  signal(SIGINT, sigintHandler);
  printf("\nYou have pressed Ctrl+c, safe shutting down server!!!\n"); 
  fflush(stdout);
  close(new_fd);
  close(sockfd);
  exit(0);
} 

void delay(int milli_seconds) 
{  
  // Storing start time 
  clock_t start_time = clock(); 
  // looping till required time is not achieved 
  while (clock() < start_time + milli_seconds) ; 
} 



int main(int argc, char *argv[]){
  signal(SIGINT, sigintHandler);
  printf("\n.......SERVER.........\n");
  int error;
  struct sockaddr_in server_addr;
  
  //getting socket
  printf("Creating Socket....\n");
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
  
  
  
  error = listen(sockfd, BACKLOG);
  if(error == -1){
    printf("\nlisten error\n");
  }
  else
    printf("Listening to Port = %d\n", MYPORT);

  
  struct sockaddr_in client_addr;
  int sin_size = sizeof(struct sockaddr);

  new_fd = accept(sockfd,(struct sockaddr*)&client_addr, &sin_size);

  if(new_fd == -1){
    printf("\naccept error\n");
  }
  else 
    printf("Connection Accepted from  IP = %s, Port = %d\n", inet_ntoa(client_addr.sin_addr), client_addr.sin_port);

  int ByteRecv = 20;
  char *rcvMsg = (char *)malloc(ByteRecv*sizeof(char));
  
  while(1){
    ByteRecv = 20;
    *rcvMsg = '\0';
    ByteRecv =  recv(new_fd, rcvMsg, ByteRecv, 0);
    
    if(ByteRecv > 0){
      printf("\nMsg = %s\n", rcvMsg);
      printf("ByteReceived = %d bytes\n", ByteRecv);
      
      if(!strcmp(rcvMsg, "end")){
        printf("\nshutting down...\n");
        break;
      }

    }
    
    else if(ByteRecv == -1){
      printf("\nConnection Lost....\n");
    }
    
    delay(100);//wait 100ms
  }

  close(new_fd);
  close(sockfd);
  
  return 0;
}
