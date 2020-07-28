#include <stdio.h> //printf
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h> //memset, strcmp
#include <stdlib.h> //malloc
#include <fcntl.h>  //for open
#include <unistd.h> //for close
#include <time.h>   //To use time library of C
#include <signal.h>
#include <errno.h> //global errno displaying only, I am not using it for error check (can do problem in multithreading)
#include <pthread.h>
#include <stdbool.h>
#include "msgToPrint.h"

#define BufferSize 1000 //bytes either to recv() or send()

int sockfd;
pthread_t DataSEND_thread_id, DataRECV_thread_id;

void closeAll()
{
  printf(MSG_SHUTTING_DOWN);
  fflush(stdout);
  close(sockfd);
  pthread_cancel(DataRECV_thread_id);
  pthread_cancel(DataSEND_thread_id);
}
void SafeShutDown()
{
  closeAll();
  /*No error encountered while running progEXIT_SUCCESS = 0*/
  exit(EXIT_SUCCESS);
}

void FailedShutDown()
{ //upon failure shutting down
  closeAll();
  /*Only EXIT_FAILURE is the standard 
  value for returning unsuccessful termination. 
  It means there must be an error EXIT_FAILURE = 1*/
  exit(EXIT_FAILURE);
}

/* Signal Handler*/
void signalHandler(int sig_num)
{
  /* Reset handler to catch SIGINT next time. SIGINT when ctrl+c pressed*/
  signal(SIGINT, signalHandler);
  signal(SIGPIPE, signalHandler);

  if (sig_num == SIGPIPE)
  {
    printf(MSG_BROKEN_PIPE_NO_READER);
    FailedShutDown();
  }
  else
  {
    printf(MSG_PRESSED_CTRL_C);
    SafeShutDown();
  }
}

void delay(int milli_seconds)
{
  clock_t start_time = clock(); // Storing start time
  while (clock() < start_time + milli_seconds)
    ; // looping till required time is not achieved
}

void setSockAddr_IN(struct sockaddr_in *addr, char *IP, unsigned short int portNo)
{
  addr->sin_family = AF_INET;
  addr->sin_port = htons(portNo);
  addr->sin_addr.s_addr = inet_addr(IP);
  memset(&addr->sin_zero, '\0', 8);
}

//###################################################################
void *DataSEND(void *discriptor)
{

  int mainfd = *((int *)(discriptor));
  int ByteSend;
  char *SendMsg = (char *)malloc(BufferSize * sizeof(char));
  char *TalkTo_IP = (char *)malloc(4 * sizeof(char));
  unsigned short int TalkTo_PORT;

  struct sockaddr_in TalkTo_addr;
  printf("\nPlease tell whom you want to talk to\n");
  printf(MSG_ENTER_TalkTo_IP);
  scanf("%s", TalkTo_IP);
  printf(MSG_ENTER_TalkTo_PORT);
  scanf("%hu", &TalkTo_PORT);

  //Not in use
  setSockAddr_IN(&TalkTo_addr, TalkTo_IP, TalkTo_PORT); //TalkTo sockaddr_in

  bool is_TalkTo_send = false;

  while (1)
  {
    ByteSend = 0;
    *SendMsg = '\0';
    if (!is_TalkTo_send)
    {
      sprintf(SendMsg, "%s %s %hu", ACK_For_TalkTo, TalkTo_IP, TalkTo_PORT);
      ByteSend = strlen(SendMsg);
      ByteSend = send(mainfd, SendMsg, ByteSend, 0);
      is_TalkTo_send = true;
    }
    else
    {
      printf("\nEnter an Msg: ");
      scanf(" %[^\n]%*c", SendMsg); //space so dont read pevious
      printf("Sending Msg = %s\n", SendMsg);
      ByteSend = strlen(SendMsg);
      ByteSend = send(mainfd, SendMsg, ByteSend, 0); //wait until send data, not set global errno if nobyte send
      //only return error or bytes sent
    }

    if (ByteSend > 0)
    {
      printf("ByteSent = %d bytes\n", ByteSend);

      if (!strcmp(SendMsg, "end"))
      {
        SafeShutDown();
        break;
      }
    }
    else if (ByteSend <= 0)
    { //connectioLost == 0, return error == -1
      printf(MSG_CONN_LOST);
      printf("error value: %d\n", errno);
      FailedShutDown();
      break;
    }

    delay(100); //wait 100ms
  }
}

//###################################################################
void *DataRECV(void *discriptor)
{
  int fd = *((int *)discriptor);
  int ByteRecv;
  char *rcvMsg = (char *)malloc(BufferSize * sizeof(char));

  while (1)
  {
    ByteRecv = 0;
    *rcvMsg = '\0';
    ByteRecv = recv(fd, rcvMsg, BufferSize, 0); //wait until receive data, not set global errno if nobyte recvd
    //only return error or bytes received
    rcvMsg[ByteRecv] = '\0'; //in case old data also present will not be processed
    if (ByteRecv > 0)
    {
      printf("\nMsg = %s\n", rcvMsg);
      printf("ByteReceived = %d bytes\n", ByteRecv);

      if (!strcmp(rcvMsg, "end"))
      {
        printf(MSG_SHUTTING_DOWN);
        SafeShutDown();
        break;
      }
    }
    else if (ByteRecv <= 0)
    { //connectioLost == 0, return error == -1
      printf(MSG_CONN_LOST);
      FailedShutDown();
    }

    delay(100); //wait 100ms
  }
}

//###################################################################
int main(int argc, char **argv)
{
  signal(SIGINT, signalHandler);
  signal(SIGPIPE, signalHandler);

  printf(MSG_CLIENT_TITLE);
  int ManualErrorCheck; //for safety, check only the return value of function
  struct sockaddr_in local_address, SERVER_addr;
  int addr_size = sizeof(local_address);

  char *SERVER_IP = (char *)malloc(4 * sizeof(char));
  unsigned short int SERVER_PORT;

  ManualErrorCheck = sockfd = socket(PF_INET, SOCK_STREAM, 0);
  if (ManualErrorCheck < 0)
  {
    perror(MSG_SOCKET_FAILED);
    printf("error value: %d\n", errno);
    printf(MSG_SHUTTING_DOWN);
    exit(EXIT_FAILURE);
  }
  else
    printf(MSG_SOCKET_SUCC);

  //when getting UP connect it to server, If cannot terminate
  //putting TalkTo values in sockaddr_in
  if (argc == 3)
  {
    SERVER_IP = argv[1];         //it start pointing to it
    SERVER_PORT = atoi(argv[2]); //integer char array to integer
  }
  else
  {
    printf(MSG_ENTER_SERVER_IP);
    scanf("%s", SERVER_IP);
    //printf("%s\n", SERVER_IP);
    printf(MSG_ENTER_SERVER_PORT);
    scanf("%hu", &SERVER_PORT);
    //printf("%hu\n", SERVER_PORT);
  }

  setSockAddr_IN(&SERVER_addr, SERVER_IP, SERVER_PORT); //server sock_addr_in

  //Trying to connect to server.......
  printf(MSG_TRY_CONNECTING);
  ManualErrorCheck = connect(sockfd, (struct sockaddr *)&SERVER_addr, sizeof(struct sockaddr));
  if (ManualErrorCheck < 0)
  {
    perror(MSG_CONN_FAILED); //it will also print error stands for
    printf("error value: %d\n", errno);
    FailedShutDown();
  }
  else
  {
    getsockname(sockfd, (struct sockaddr *)&local_address, &addr_size);
    printf("Your IP:%s, Your PORT:%d\n",
           inet_ntoa(local_address.sin_addr), htons(local_address.sin_port));
    printf("IP:%s, PORT:%d", inet_ntoa(SERVER_addr.sin_addr), htons(SERVER_addr.sin_port));
    printf(MSG_CONNECTED_SUCC);
  }

  //DataSEND(sockfd);

  pthread_create(&DataSEND_thread_id, NULL, DataSEND, &sockfd);
  pthread_create(&DataRECV_thread_id, NULL, DataRECV, &sockfd);
  pthread_join(DataSEND_thread_id, NULL);
  //pthread_join(DataRECV_thread_id, NULL);
  printf("After Thread\n");

  SafeShutDown();
  return 0;
}
