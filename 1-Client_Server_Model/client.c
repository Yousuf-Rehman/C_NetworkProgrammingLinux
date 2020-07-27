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

#define DEST_IP "192.168.10.21"
#define DEST_PORT 4200

int sockfd;

/* Signal Handler for SIGINT */
void sigintHandler(int sig_num)
{
  /* Reset handler to catch SIGINT next time. SIGINT when ctrl+c pressed*/
  signal(SIGINT, sigintHandler);
  printf("\nYou have pressed Ctrl+c, safe shutting down client!!!\n");
  fflush(stdout);
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

void main(){
  signal(SIGINT, sigintHandler);
  printf("\n.......CLIENT.........\n");
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
  
  printf("Connecting....\n");
  if(connect(sockfd, (struct sockaddr*)&dest_addr, sizeof(struct sockaddr)) == -1)
    printf("connect error\n");
  else
    printf("Connected\n");

  int ByteSend = 20;
  char *SendMsg = (char *)malloc(ByteSend*sizeof(char));
  while(1){
    printf("\nEnter an Msg: ");
    scanf("%s", SendMsg);
    ByteSend = send(sockfd, SendMsg, ByteSend, 0);
    
    if(ByteSend > 0){
      printf("ByteSent = %d bytes\n", ByteSend);
      
      if(!strcmp(SendMsg, "end")){
        printf("\nshutting down...\n");
        break;
      }

    }
    else if(ByteSend == -1){
      printf("\nConnection lost...\n");
      break;
    }

    delay(100);//wait 100ms
  }
  
  close(sockfd);
}
