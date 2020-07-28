#include <stdio.h> //printf, scanf
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
#include "msgToPrint.h"
#include <stdbool.h>

#define _MYPORT 4200
#define _MyIP "192.168.100.24"
//Max Client can connect
#define BACKLOG 10
//bytes either to recv() or send()
#define BufferSize 1000

unsigned short MYPORT;
char *MyIP;

pthread_t listenAndAccept_thread_id; //a thread only for listen and accept, work on main sock
pthread_t DataRECV_thread_id[BACKLOG];
pthread_t DataSEND_thread_id[BACKLOG];
//pthread_t Data
int sockfd;    //socket file descriptor, main discriptor
int TotalConn; //total live conn

struct OneClientInstance
{
  bool isalive;
  int discriptor; //every instance has its on socket discriptor
  struct sockaddr_in this;
  struct sockaddr_in TalkTo;
};

struct OneClientInstance clientInstancesArray[BACKLOG];
//#########################################################################3
void closeAll()
{
  printf(MSG_SHUTTING_DOWN);
  fflush(stdout);
  close(sockfd);
  int i;
  for (i = 0; i < BACKLOG; i++)
    pthread_cancel(DataRECV_thread_id[i]);
  pthread_cancel(listenAndAccept_thread_id);
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
  // Storing start time
  clock_t start_time = clock();
  // looping till required time is not achieved
  while (clock() < start_time + milli_seconds)
    ;
}
//##################################################################################
void setSockAddr_in_IP_PORT(struct sockaddr_in *addr, char *IP, unsigned short int portNo)
{
  addr->sin_family = AF_INET;
  addr->sin_port = htons(portNo);
  addr->sin_addr.s_addr = inet_addr(IP);
  memset(&addr->sin_zero, '\0', 8);
}
void setSockAddr_IN(struct sockaddr_in *addr, struct sockaddr_in *client_addr)
{
  addr->sin_family = client_addr->sin_family;
  addr->sin_port = client_addr->sin_port;
  addr->sin_addr.s_addr = client_addr->sin_addr.s_addr;
  memset(&addr->sin_zero, '\0', 8);
}
//###################################################################
/*void *DataSEND(void *discriptor)
{

  int mainfd = *((int *)(discriptor));
  int ByteSend;
  char *SendMsg = (char *)malloc(BufferSize * sizeof(char));

  while (1)
  {
    ByteSend = 0;
    *SendMsg = '\0';

    //printf("\nEnter an Msg: ");
    scanf("%s", SendMsg); //space so dont read pevious
    //printf("Sending Msg = %s\n", SendMsg);
    ByteSend = strlen(SendMsg);
    ByteSend = send(mainfd, SendMsg, ByteSend, 0); //wait until send data, not set global errno if nobyte send
                                                   //only return error or bytes sent

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
*/
void DataSEND(int newfd, char *SendMsg)
{

  int ByteSend;

  ByteSend = 0;
  ByteSend = strlen(SendMsg);
  ByteSend = send(newfd, SendMsg, ByteSend, 0); //wait until send data, not set global errno if nobyte send
                                                //only return error or bytes sent

  if (ByteSend > 0)
  {
    printf("ByteSent = %d bytes\n", ByteSend);
  }
  else if (ByteSend <= 0)//connectioLost == 0, return error == -1
  { 
    printf(MSG_CONN_LOST);
    printf("error value: %d\n", errno);
  }

  delay(100); //wait 100ms
}

struct OneClientInstance *findTalkToUserInstance(struct OneClientInstance *instance)
{

  int i;
  for (i = 0; i < BACKLOG; i++)
  {
    if (clientInstancesArray[i].isalive &&
        instance->TalkTo.sin_addr.s_addr == clientInstancesArray[i].this.sin_addr.s_addr && //s_addr is uint32_t
        instance->TalkTo.sin_port == clientInstancesArray[i].this.sin_port)
    {
      return &clientInstancesArray[i]; //index
    }
    /*
    if (clientInstancesArray[i].isalive){
        printf("\nIIII: %d %d\n", instance->TalkTo.sin_addr.s_addr, clientInstancesArray[i].this.sin_addr.s_addr);
        printf("\nIIII: %d %d\n", htons(instance->TalkTo.sin_port), htons(clientInstancesArray[i].this.sin_port));
    }*/
  }
  return NULL;
}
//#################################################################
void *DataRECV(void *pinstance)
{
  struct OneClientInstance *instance = ((struct OneClientInstance *)pinstance);

  int fd = instance->discriptor; //listening only to specific discriptor
  int ByteRecv;
  char *rcvMsg = (char *)malloc(BufferSize * sizeof(char));
  char *rcvMsg_tmp;
  char *TalkTo_IP;
  unsigned short int TalkTo_PORT;
  bool is_TalkTo_rcv = false;

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
      printf("ByteReceived = %d bytes\n", ByteRecv); //only print if \n used in thread
      /*rcvMsg_tmp = strstr(rcvMsg, ACK_For_TalkTo);
      if(rcvMsg_tmp){//if not found give NULL
        printf("\nFound %s\n", rcvMsg_tmp);
      }*/
      if (!is_TalkTo_rcv)
      {
        rcvMsg_tmp = strtok(rcvMsg, " ");
        if (rcvMsg_tmp != NULL)
        {
          TalkTo_IP = strtok(NULL, " "); //next
          //printf("\nIP RECVD: %s\n", TalkTo_IP);
          TalkTo_PORT = atoi(strtok(NULL, " ")); //next
          //printf("\nPORT RECVD: %d\n", TalkTo_PORT);
          setSockAddr_in_IP_PORT(&instance->TalkTo, TalkTo_IP, TalkTo_PORT);
          printf("\nIP RECVD: %s\n", inet_ntoa(instance->TalkTo.sin_addr));
          printf("\nPORT RECVD: %d\n", htons(instance->TalkTo.sin_port));
        }

        is_TalkTo_rcv = true;
      }
      else
      {
        struct OneClientInstance *TalkToInstance = findTalkToUserInstance(instance);
        if (TalkToInstance != NULL) //end User
        {
          printf("\n..............FOUND END USER.................\n");
          DataSEND(TalkToInstance->discriptor, rcvMsg);
        }
        else
        {
          printf("\n..............NOT FOUND END USER..................\n");
          //printf("%d\n", TalkToInstance);
        }
      }

      if (strcmp(rcvMsg, "end") == 0)
      {
        printf(MSG_SHUTTING_DOWN);
        break;
      }
    }
    else if (ByteRecv <= 0)
    { //connectioLost == 0, return error == -1
      printf(MSG_CONN_LOST_SPECIFIC_USER, inet_ntoa(instance->this.sin_addr), ntohs(instance->this.sin_port));
      TotalConn--;
      printf(MSG_TOTAL_CONN, TotalConn);
      pthread_exit(NULL);
    }

    delay(100); //wait 100ms
  }
}

struct OneClientInstance *AddConnection(int specialDiscriptor, struct sockaddr_in *client_addr)
{

  int i;
  for (i = 0; i < BACKLOG; i++)
  {
    if (clientInstancesArray[i].isalive == false)
    {
      clientInstancesArray[i].isalive = true;
      clientInstancesArray[i].discriptor = specialDiscriptor;
      setSockAddr_IN(&clientInstancesArray[i].this, client_addr);
      /*his IP and port will need to be changed and that is done in DataRECV()*/
      setSockAddr_IN(&clientInstancesArray[i].TalkTo, client_addr);
      return &clientInstancesArray[i]; //break, dont forget to add only one
    }
  }

  return NULL;
}

void *listenAndAccept(void *mainfd_ptr)
{
  int mainfd = *((int *)(mainfd_ptr));
  int new_fd;

  int ManualErrorCheck; //for safety, check only the return value of function
  while (1)
  {
    ManualErrorCheck = listen(mainfd, BACKLOG); //wait here until get connection, listen on main fd
    if (ManualErrorCheck < 0)
    {
      perror(MSG_LISTEN_FAILED);
      printf("error value: %d\n", errno);
      return (void *)ManualErrorCheck;
    }
    else
      printf(MSG_LISTENING_TO, MYPORT);

    struct sockaddr_in client_addr;
    int sin_size = sizeof(struct sockaddr);
    ManualErrorCheck = new_fd = accept(mainfd, (struct sockaddr *)&client_addr, &sin_size);

    if (ManualErrorCheck < 0)
    {
      perror(MSG_ACCEPT_FAILED);
      printf("error value: %d\n", errno);
      return (void *)ManualErrorCheck;
    }
    else
    {
      printf(MSG_ACCEPT_SUCC, inet_ntoa(client_addr.sin_addr), htons(client_addr.sin_port));
      struct OneClientInstance *temp = AddConnection(new_fd, &client_addr);
      if (temp != NULL)
      {
        if (TotalConn < BACKLOG)
        {
          pthread_create(&DataRECV_thread_id[TotalConn], NULL, DataRECV, temp);
          //pthread_create(&DataSEND_thread_id[TotalConn], NULL, DataSEND, &temp->discriptor);
        }
        TotalConn++;
        printf(MSG_TOTAL_CONN, TotalConn);
      }
      else
        printf("\nNo Space!\n");
    }
  }

  return (void *)new_fd;
}

//###################################################################
int main(int argc, char **argv)
{
  signal(SIGINT, signalHandler);
  signal(SIGPIPE, signalHandler);
  printf(MSG_SERVER_TITLE);
  int ManualErrorCheck; //for safety, check only the return value of function
  struct sockaddr_in server_addr;
  MyIP = (char *)malloc(4 * sizeof(char));

  //initializing array
  int i;
  for (i = 0; i < BACKLOG; i++)
    clientInstancesArray[i].isalive = false;

  //getting socket
  ManualErrorCheck = sockfd = socket(PF_INET, SOCK_STREAM, 0);
  if (ManualErrorCheck < 0)
  {
    perror(MSG_SOCKET_FAILED);
    exit(EXIT_FAILURE);
  }
  else
    printf(MSG_SOCKET_SUCC);

  //putting values in sockaddr_in
  if (argc == 3)
  {
    MyIP = argv[1];
    MYPORT = atoi(argv[2]); //integer char array to integer
  }
  else
  {
    MyIP = _MyIP;
    MYPORT = _MYPORT;
  }

  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = inet_addr(MyIP);
  server_addr.sin_port = htons(MYPORT);
  memset(&server_addr.sin_zero, '\0', 8); //zero all values

  //Binding
  printf("Binding IP = %s with Port = %d\n", MyIP, MYPORT);
  ManualErrorCheck = bind(sockfd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr));
  if (ManualErrorCheck < 0)
  {
    perror(MSG_BIND_FAILED);
    printf("error value: %d\n", errno);
    FailedShutDown();
  }

  pthread_create(&listenAndAccept_thread_id, NULL, listenAndAccept, &sockfd);
  pthread_join(listenAndAccept_thread_id, NULL);
  SafeShutDown();
  return 0;
}
