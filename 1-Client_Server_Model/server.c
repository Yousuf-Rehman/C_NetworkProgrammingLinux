#include<stdio.h>   //printf, scanf
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


#define _MYPORT 4200
#define _MyIP "192.168.100.24"
//Max Client can connect
#define BACKLOG 10
//bytes either to recv() or send()
#define BufferSize 1000

unsigned short MYPORT;
char *MyIP;

int sockfd;//socket file descriptor
int new_fd;

/* Signal Handler for SIGINT */
void sigintHandler(int sig_num) 
{ 
  /* Reset handler to catch SIGINT next time again. SIGINT when ctrl+c pressed*/
  signal(SIGINT, sigintHandler);
  printf("\nYou have pressed Ctrl+c, safe shutting down server!!!\n"); 
  fflush(stdout);
  close(new_fd);
  close(sockfd);
  exit(EXIT_SUCCESS); //No error encountered while running progEXIT_SUCCESS = 0
} 

void delay(int milli_seconds) 
{  
  // Storing start time 
  clock_t start_time = clock(); 
  // looping till required time is not achieved 
  while (clock() < start_time + milli_seconds) ; 
} 

int listenAndAccept(struct sockaddr_in *client_addr, int *sin_size){
  int ManualErrorCheck; //for safety, check only the return value of function

  ManualErrorCheck = listen(sockfd, BACKLOG); //wait here until get connection
  if(ManualErrorCheck < 0){
    perror("\nlisten error\n");
    printf("error value: %d\n", errno); 
    return ManualErrorCheck;
  }
  else
    printf("Listening to Port = %d\n", MYPORT);

  ManualErrorCheck = new_fd = accept(sockfd,(struct sockaddr*)client_addr, sin_size);

  if(ManualErrorCheck < 0){
    perror("\naccept error\n");
    printf("error value: %d\n", errno); 
    return ManualErrorCheck;//Only EXIT_FAILURE is the standard value for returning unsuccessful termination. It means there must be an error EXIT_FAILURE = 1
  }
  else 
    printf("Connection Accepted from  IP = %s, Port = %d\n", inet_ntoa(client_addr->sin_addr), client_addr->sin_port);
  
  return new_fd;
}

int main(int argc, char **argv){
  signal(SIGINT, sigintHandler);
  printf("\n.......SERVER.........\n");
  int ManualErrorCheck; //for safety, check only the return value of function
  struct sockaddr_in server_addr;
  MyIP = (char *) malloc(4*sizeof(char));

  //getting socket
  printf("Creating Socket....\n");
  ManualErrorCheck = sockfd = socket(PF_INET, SOCK_STREAM, 0);
  if(ManualErrorCheck < 0){
    perror("socket creation failed...\n");
    exit(EXIT_FAILURE);
  }
  else
    printf("socket successfully created..\n");
  
  
  //putting values in sockaddr_in
  if(argc == 3){
    MyIP = argv[1];
    MYPORT = atoi(argv[2]);//integer char array to integer
  }
  else{
    MyIP = _MyIP;
    MYPORT = _MYPORT;
  }

  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = inet_addr(MyIP);
  server_addr.sin_port = htons(MYPORT);
  memset(&server_addr.sin_zero, '\0', 8);//zero all values
  
  //Binding
  printf("Binding IP = %s with Port = %d\n", MyIP, MYPORT); 
  ManualErrorCheck = bind(sockfd,(struct sockaddr*)&server_addr,sizeof(struct sockaddr));
  if(ManualErrorCheck < 0 ){
    perror("bind error!!\n");
    printf("error value: %d\n", errno); 
    exit(EXIT_FAILURE);
  }
  
  struct sockaddr_in client_addr;
  int sin_size = sizeof(struct sockaddr);
  
  listenAndAccept(&client_addr, &sin_size);

  int ByteRecv;
  char *rcvMsg = (char *)malloc(BufferSize*sizeof(char));
  
  while(1){
    ByteRecv = 0;
    *rcvMsg = '\0';
    ByteRecv = recv(new_fd, rcvMsg, BufferSize, 0);//wait until receive data, not set global errno if nobyte recvd
    //only return error or bytes received
    rcvMsg[ByteRecv] = '\0'; //in case old data also present will not be processed
    if(ByteRecv > 0){
      printf("\nMsg = %s\n", rcvMsg);
      printf("ByteReceived = %d bytes\n", ByteRecv);

      if(!strcmp(rcvMsg, "end")){
        printf("\nshutting down...\n");
        break;
      }

    }
    else if(ByteRecv <= 0){//connectioLost == 0, return error == -1
      printf("\nConnection lost...\n");
      printf("\nListening Again.....Server Should Never be down :) \n");
      listenAndAccept(&client_addr, &sin_size);
    }
    
    delay(100);//wait 100ms
  }

  close(new_fd);
  close(sockfd);
  
  return 0;
}
