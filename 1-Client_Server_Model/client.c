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
#include <errno.h> //global errno displaying only, I am not using it for error check (can do problem in multithreading) 

#define DEST_IP "192.168.100.24"
#define DEST_PORT 4200
#define BufferSize 1000 //bytes either to recv() or send()

int sockfd;


/* Signal Handler for SIGINT */
void sigintHandler(int sig_num)
{
  /* Reset handler to catch SIGINT next time. SIGINT when ctrl+c pressed*/
  signal(SIGINT, sigintHandler);
  signal(SIGPIPE, sigintHandler);

  if(sig_num == SIGPIPE){
    printf("\nBroken pipe: write to pipe with no readers, means other host is gone!!!\n");
  }
  else
    printf("\nYou have pressed Ctrl+c, safe shutting down client!!!\n");

  fflush(stdout);
  close(sockfd);
  exit(EXIT_SUCCESS);
}

void delay(int milli_seconds)
{
  // Storing start time 
  clock_t start_time = clock();
  // looping till required time is not achieved 
  while (clock() < start_time + milli_seconds) ;
}

int main(int argc, char **argv){
  signal(SIGINT, sigintHandler);
  signal(SIGPIPE, sigintHandler);
  printf("\n.......CLIENT.........\n");
  int ManualErrorCheck; //for safety, check only the return value of function
  struct sockaddr_in dest_addr;
  
  ManualErrorCheck = sockfd = socket(PF_INET, SOCK_STREAM, 0);
  if(ManualErrorCheck < 0){
    perror("socket creation failed...\n");
    printf("error value: %d\n", errno);
    printf("\nshutting down...\n");
    exit(EXIT_FAILURE);
  }
  else
    printf("Socket successfully created..\n");


  //putting dest values in sockaddr_in
  dest_addr.sin_family = AF_INET;
  dest_addr.sin_port = htons(DEST_PORT);
  dest_addr.sin_addr.s_addr = inet_addr(DEST_IP);
  memset(&dest_addr.sin_zero, '\0',8);
  
  printf("Connecting....\n");
  ManualErrorCheck = connect(sockfd, (struct sockaddr*)&dest_addr, sizeof(struct sockaddr));
  if(ManualErrorCheck < 0){
    perror("connect error\n");//it will also print error stands for
    printf("error value: %d\n", errno);
    printf("\nshutting down...\n");
    exit(EXIT_FAILURE);
  }
  else
    printf("Connected\n");

  int ByteSend;
  char *SendMsg = (char *)malloc(BufferSize*sizeof(char));
  while(1){
    ByteSend = 0;
    *SendMsg = '\0';
    printf("\nEnter an Msg: ");
    scanf("%[^\n]%*c", SendMsg);
    printf("Sending Msg = %s\n", SendMsg);
    ByteSend = strlen(SendMsg);
    ByteSend = send(sockfd, SendMsg, ByteSend, 0);//wait until send data, not set global errno if nobyte send
    //only return error or bytes sent

    if(ByteSend > 0){
      printf("ByteSent = %d bytes\n", ByteSend);
      
      if(!strcmp(SendMsg, "end")){
        printf("\nshutting down...\n");
        break;
      }

    }
    else if(ByteSend <= 0){//connectioLost == 0, return error == -1
      printf("\nConnection lost...\n");
      printf("error value: %d\n", errno); 
      break;
    }

    delay(100);//wait 100ms
  }
  
  close(sockfd);
  return 0;
}
