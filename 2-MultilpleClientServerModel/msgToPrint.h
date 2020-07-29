#ifndef msgToPrint_H
#define msgToPrint_H

//Main Title
#define MSG_CLIENT_TITLE "\n.......CLIENT.........\n"
#define MSG_SERVER_TITLE "\n.......SERVER.........\n"

//Input from Client => CLIENT SPECIFIC
#define MSG_ENTER_SERVER_IP "Enter SERVER IP: "
#define MSG_ENTER_SERVER_PORT "Enter SERVER PORT: "
#define MSG_ENTER_TalkTo_IP "Enter TalkTo IP: "
#define MSG_ENTER_TalkTo_PORT "Enter TalkTo PORT: "

//socket
#define MSG_SOCKET_FAILED "Socket Creation Failed!\n"
#define MSG_SOCKET_SUCC "Socket successfully created!\n"

//connection
#define MSG_CONN_FAILED "Connection Failed!\n\n"
#define MSG_CONN_LOST "Connection Lost!\n\n"
#define MSG_TRY_CONNECTING "Try Connecting....\n"
#define MSG_CONNECTED_SUCC "Connected Successfully!\n\n"
#define MSG_CONN_LOST_SPECIFIC_USER "Connection Lost! IP:%s , PORT=%d\n\n"

//SERVER TOTAL CONNECTIONS, => SERVER SPECIFIC
#define MSG_TOTAL_CONN "Current Total Connections: %d\n\n"

//BIND, => SERVER SPECIFIC
#define MSG_BIND_FAILED "Bind Failed!\n\n"
#define MSG_BIND_SUCC "Bind Successfully!\n\n"

//LISTEN, => SERVER SPECIFIC
#define MSG_LISTEN_FAILED "LISTENING Failed!\n\n"
#define MSG_LISTENING_TO "Listening to Port = %d\n\n"

//Accept, => SERVER SPECIFIC
#define MSG_ACCEPT_FAILED "Connection Accept Failed!\n\n"
#define MSG_ACCEPT_SUCC "Connection Accepted of IP = %s, Port = %d\n\n"

//SEND()
#define MSG_ByteSend "ByteSent = %d bytes\n"
#define MSG_ByteRecv "ByteReceived = %d bytes\n"

//shutdown
#define MSG_WROTE_END "You have written \"end\" only its a preserved word\n\n"
#define MSG_PRESSED_CTRL_C "You have pressed Ctrl+c, safely shutting down!!!\n\n"
#define MSG_BROKEN_PIPE_NO_READER "Broken pipe: write to pipe with no readers, means other host is gone, shutting down!!!\n\n"
#define MSG_SHUTTING_DOWN "\n...........shutting down..........\n"

//Ack and code to send by client whom he want to talk
#define ACK_For_TalkTo "${100001}"

//SEARCHING ARRAY, => SERVER SPECIFIC
#define MSG_FOUND_TALKTO "FOUND END USER(TalkTo) During Search In clientInstancesArray\n\n"
#define MSG_NOT_FOUND_TALKTO "CANNOT FOUND END USER(TalkTo) During Search In clientInstancesArray\n\n"

//LOG FILE
#define CLIENT_LOG_FILE "client.log"
#define SERVER_LOG_FILE "server.log"

#endif