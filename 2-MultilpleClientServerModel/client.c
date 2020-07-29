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
#include "logger.h"

#define TCP_USER_TIMEOUT 18 // how long for loss retry before timeout [ms]
#define SOL_TCP 6           // socket options TCP level
#define SockBufferSize 1000 //bytes either to recv() or send()
#define LogBufferSize (2 * SockBufferSize)

int sockfd;
pthread_t DataSEND_thread_id, DataRECV_thread_id;

void closeAll()
{
  printf(MSG_SHUTTING_DOWN);
  logger_ConstFormat_Info("closeAll()", MSG_SHUTTING_DOWN);
  fflush(stdout);
  pthread_cancel(DataRECV_thread_id);
  pthread_cancel(DataSEND_thread_id);
  close(sockfd); //main socket
}
void SafeShutDown()
{
  logger_ConstFormat_Info("SafeShutDown()", "");
  closeAll();
  /*No error encountered while running progEXIT_SUCCESS = 0*/
  exit(EXIT_SUCCESS);
}

void FailedShutDown()
{ //upon failure shutting down
  logger_ConstFormat_Info("FailedShutDown()", "");
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
    logger_ConstFormat_Debug("signalHandler()", MSG_BROKEN_PIPE_NO_READER);
    FailedShutDown();
  }
  else
  {
    printf(MSG_PRESSED_CTRL_C);
    logger_ConstFormat_Debug("signalHandler()", MSG_PRESSED_CTRL_C);
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
  delay(500);//wait for the server response then start sending

  int mainfd = *((int *)(discriptor));
  int ByteSend;
  char *SendMsg = (char *)malloc(SockBufferSize * sizeof(char));
  char *TalkTo_IP = (char *)malloc(4 * sizeof(char));
  unsigned short int TalkTo_PORT;
  char *logMsg = (char *)malloc(LogBufferSize * sizeof(char));

  struct sockaddr_in TalkTo_addr;
  printf("\n.................................\n");
  printf("\nPlease tell whom you want to talk to\n");
  printf("\n.................................\n");
  printf(MSG_ENTER_TalkTo_IP);
  scanf("%s", TalkTo_IP);
  printf("\n.................................\n");
  printf(MSG_ENTER_TalkTo_PORT);
  scanf("%hu", &TalkTo_PORT);

  //logging
  sprintf(logMsg, "TalkTo IP:%s, TalkTo PORT:%d\n", TalkTo_IP, TalkTo_PORT);
  logger_ConstFormat_Info("*DataSEND()", logMsg);

  //Not in use
  setSockAddr_IN(&TalkTo_addr, TalkTo_IP, TalkTo_PORT); //TalkTo sockaddr_in

  bool is_TalkTo_send = false;

  while (1)
  {
    ByteSend = 0;
    *SendMsg = '\0';
    if (!is_TalkTo_send)
    {
      sprintf(SendMsg, "%s %s %d", ACK_For_TalkTo, TalkTo_IP, TalkTo_PORT);
      ByteSend = strlen(SendMsg);
      ByteSend = send(mainfd, SendMsg, ByteSend, 0);
      is_TalkTo_send = true;
    }
    else
    {
      printf("\n.................................\n");
      printf("\nEnter an Msg: ");
      scanf(" %[^\n]%*c", SendMsg); //space so dont read pevious
      ByteSend = strlen(SendMsg);
      ByteSend = send(mainfd, SendMsg, ByteSend, 0); //wait until send data, not set global errno if nobyte send
      //only return error or bytes sent
      printf("\n=>SendMsg=>%s\n", SendMsg);
    }

    if (ByteSend > 0)
    {
      if (!strcmp(SendMsg, "end"))
      {
        printf("%s", MSG_WROTE_END);
        logger_ConstFormat_Debug("*DataSEND()", MSG_WROTE_END);
        SafeShutDown();
        break;
      }
    }
    else if (ByteSend <= 0) //connectioLost == 0, return error == -1
    {
      printf(MSG_CONN_LOST);
      printf("error value: %d\n", errno);
      logger_ConstFormat_Error("*DataSEND()", MSG_CONN_LOST, errno);
      FailedShutDown();
      break;
    }

    //logging
    sprintf(logMsg, "SendMsg:%s, ByteSend:%d, is_TalkTo_send:%d\n",
            SendMsg, ByteSend, is_TalkTo_send);
    logger_ConstFormat_Info("*DataSEND()", logMsg);
  }
}

//###################################################################
void *DataRECV(void *discriptor)
{
  int fd = *((int *)discriptor);
  int ByteRecv;
  char *rcvMsg = (char *)malloc(SockBufferSize * sizeof(char));
  char *logMsg = (char *)malloc(LogBufferSize * sizeof(char));

  while (1)
  {
    ByteRecv = 0;
    *rcvMsg = '\0';
    ByteRecv = recv(fd, rcvMsg, SockBufferSize, 0); //wait until receive data, not set global errno if nobyte recvd only return error or bytes received
    rcvMsg[ByteRecv] = '\0';                        //in case old data also present will not be processed
    if (ByteRecv > 0)
    {
      printf("\nrcvMsg = %s\n", rcvMsg);

      if (!strcmp(rcvMsg, "end"))
      {
        printf(MSG_SHUTTING_DOWN);
        logger_ConstFormat_Debug("*DataRECV()", MSG_WROTE_END);
        SafeShutDown();
        break;
      }
    }
    else if (ByteRecv <= 0) //connectioLost == 0, return error == -1
    {
      printf(MSG_CONN_LOST);
      logger_ConstFormat_Error("*DataRECV()", MSG_CONN_LOST, errno);
      FailedShutDown();
    }

    sprintf(logMsg, "rcvMsg:%s, ByteRcv:%d\n", rcvMsg, ByteRecv);
    logger_ConstFormat_Info("*DataRECV()", logMsg);
  }
}

//###################################################################
int main(int argc, char **argv)
{
  signal(SIGINT, signalHandler);
  signal(SIGPIPE, signalHandler);
  loggerRemove(CLIENT_LOG_FILE);

  printf(MSG_CLIENT_TITLE);
  logger_ConstFormat_Info("main()", MSG_CLIENT_TITLE);

  int ManualErrorCheck; //for safety, check only the return value of function
  struct sockaddr_in local_address, SERVER_addr;
  int addr_size = sizeof(local_address);
  int timeout = 10000; // user timeout in milliseconds [ms]

  char *logMsg = (char *)malloc(LogBufferSize * sizeof(char));
  char *SERVER_IP = (char *)malloc(4 * sizeof(char));
  unsigned short int SERVER_PORT;

  ManualErrorCheck = sockfd = socket(PF_INET, SOCK_STREAM, 0);
  if (ManualErrorCheck < 0)
  {
    perror(MSG_SOCKET_FAILED);
    printf("error value: %d\n", errno);
    logger_ConstFormat_Error("main()", MSG_SOCKET_FAILED, errno);
    FailedShutDown();
  }
  else
  {
    //setsockopt(sockfd, SOL_TCP, TCP_USER_TIMEOUT, (char *)&timeout, sizeof(timeout));
    printf(MSG_SOCKET_SUCC);
  }

  //when getting IP connect it to server, If cannot terminate/either ask for input
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
    printf(MSG_ENTER_SERVER_PORT);
    scanf("%hu", &SERVER_PORT);
    sprintf(logMsg, "SERVER_IP=%s, SERVER_PORT=%hu\n", SERVER_IP, SERVER_PORT);
    logger_ConstFormat_Info("main()", logMsg);
  }

  setSockAddr_IN(&SERVER_addr, SERVER_IP, SERVER_PORT); //server sock_addr_in

  //Trying to connect to server.......
  printf(MSG_TRY_CONNECTING);
  logger_ConstFormat_Info("main()", MSG_TRY_CONNECTING);
  ManualErrorCheck = connect(sockfd, (struct sockaddr *)&SERVER_addr, sizeof(struct sockaddr));
  if (ManualErrorCheck < 0)
  {
    perror(MSG_CONN_FAILED); //it will also print error stands for
    printf("error value: %d\n", errno);
    logger_ConstFormat_Error("main()", MSG_CONN_FAILED, errno);
    FailedShutDown();
  }
  else
  {
    getsockname(sockfd, (struct sockaddr *)&local_address, &addr_size);

    //printing
    printf("SERVER_IP:%s, SERVER_PORT:%d\n", inet_ntoa(SERVER_addr.sin_addr), htons(SERVER_addr.sin_port));
    printf(MSG_CONNECTED_SUCC);

    //logging
    sprintf(logMsg, "Your IP:%s, Your PORT:%d\n",
            inet_ntoa(local_address.sin_addr), htons(local_address.sin_port));
    logger_ConstFormat_Info("main()", logMsg);
    sprintf(logMsg, "SERVER_IP:%s, SERVER_PORT:%d\n",
            inet_ntoa(local_address.sin_addr), htons(local_address.sin_port));
    logger_ConstFormat_Info("main()", logMsg);
    logger_ConstFormat_Info("main()", MSG_CONNECTED_SUCC);
  }

  pthread_create(&DataSEND_thread_id, NULL, DataSEND, &sockfd);
  pthread_create(&DataRECV_thread_id, NULL, DataRECV, &sockfd);
  pthread_join(DataSEND_thread_id, NULL); //wait for the thread to complete
  pthread_join(DataRECV_thread_id, NULL); //wait for the thread to complete

  //logging
  printf("\nThreads Ended\n");
  logger_ConstFormat_Info("main()", "Threads Ended\n");

  SafeShutDown();
  return 0;
}