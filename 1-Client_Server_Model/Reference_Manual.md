# #include<stdio.h>   
for IO e.g. printf(), scanf() functions
```cpp
printf("Character is %c \n", ch);
printf("String is %s \n" , str);
printf("Float value is %f \n", flt);
printf("Integer value is %d\n" , no);
printf("Double value is %lf \n", dbl);//lf = long float(double)
printf("Octal value is %o \n", no);
printf("Hexadecimal value is %x \n", no);
```
```cpp
int integer; 
char ch;
char str[100];
scanf("%d", &integer);//decimal
scanf("%c", &ch);
scanf("%s", &str);
```

# #include<sys/types.h>
This header file contains definitions to allow for the porting of BSD programs.

<sys/types.h> **must usually be included before other socket-related header files;** refer to the individual header file descriptions that follow for the specific dependency.


The following typedef name is commonly used for buffer pointers:
```cpp
typedef char * caddr_t;
```
This header file also defines the FD_SET , FD_ZERO , FD_CLR , FD_ISSET , and FD_SETSIZE macros used by select to manipulate socket descriptors. 

e.g use by following macros: (here giving short list)
```
- accept
- accept_and_recv
- bind
- connect
- gethostbyaddr
- gethostbyname
- gethostname
- getnetbyname
- getnetent
```
# #include<errno.h>
Almost each function sets it. I have used this global errno for displaying only, I am not using it for error check (may do problem in multithreading) 

# #include<sys/socket.h>
The <sys/socket.h> header defines the sockaddr structure
```
SOCK_DGRAM Datagram socket
SOCK_STREAM Byte-stream socket
SOCK_SEQPACKET Sequenced-packet socket
```
The <sys/socket.h> header defines the following macro for use as the level argument of setsockopt() and getsockopt().
SOL_SOCKET
Options to be accessed at socket level, not protocol level.

Following Functions are given with it.
```cpp
int accept(int socket, struct sockaddr *address, socklen_t *address_len);
int bind(int socket, const struct sockaddr *address, socklen_t address_len);
int connect(int socket, const struct sockaddr *address, socklen_t address_len);
int getpeername(int socket, struct sockaddr *address, socklen_t *address_len);
int getsockname(int socket, struct sockaddr *address, socklen_t *address_len);
int getsockopt(int socket, int level, int option_name,
void *option_value, socklen_t *option_len);
int listen(int socket, int backlog);
ssize_t recv(int socket, void *buffer, size_t length, int flags);
ssize_t recvfrom(int socket, void *buffer, size_t length,
int flags, struct sockaddr *address, socklen_t *address_len);
ssize_t recvmsg(int socket, struct msghdr *message, int flags);
ssize_t send(int socket, const void *message, size_t length, int flags);
ssize_t sendmsg(int socket, const struct msghdr *message, int flags);
ssize_t sendto(int socket, const void *message, size_t length, int flags,
const struct sockaddr *dest_addr, socklen_t dest_len);
int setsockopt(int socket, int level, int option_name,
const void *option_value, socklen_t option_len);
int shutdown(int socket, int how);
int socket(int domain, int type, int protocol);
int socketpair(int domain, int type, int protocol, int socket_vector[2]);
```
# #include<netinet/in.h>
The <netinet/in.h> header shall define the following types:

in_port_t	Equivalent to the type uint16_t as defined in <inttypes.h> .
in_addr_t	Equivalent to the type uint32_t as defined in <inttypes.h> .
The sa_family_t type shall be defined as described in <sys/socket.h>.

The uint8_t and uint32_t type shall be defined as described in <inttypes.h>. Inclusion of the <netinet/in.h> header may also make visible all symbols from <inttypes.h> and <sys/socket.h>.

The <netinet/in.h> header shall define the in_addr structure that includes at least the following member:
```
in_addr_t  s_addr
```
This header shall define the sockaddr_in structure that includes at least the following members:
```
sa_family_t     sin_family   AF_INET. 
in_port_t       sin_port     Port number. 
struct in_addr  sin_addr     IP address. 
```
The sin_port and sin_addr members shall be in network byte order.

The sockaddr_in structure is used to store addresses for the Internet address family. Values of this type shall be cast by applications to struct sockaddr for use with socket functions.

**[IP6]** This header shall define the in6_addr structure that contains at least the following member:
```
uint8_t s6_addr[16]
```
This array is used to contain a 128-bit IPv6 address, stored in network byte order.

The <netinet/in.h> header shall define the sockaddr_in6 structure that includes at least the following members:
```
sa_family_t      sin6_family    AF_INET6. 
in_port_t        sin6_port      Port number. 
uint32_t         sin6_flowinfo  IPv6 traffic class and flow information. 
struct in6_addr  sin6_addr      IPv6 address. 
uint32_t         sin6_scope_id  Set of interfaces for a scope.
```

# #include<arpa/inet.h> 
The <arpa/inet.h> header makes available the type in_port_t and the type in_addr_t as defined in the description of <netinet/in.h>.
The <arpa/inet.h> header makes available the in_addr structure, as defined in the description of <netinet/in.h>.

The following may be declared as functions, or defined as macros, or both:

```cpp
uint32_t htonl(uint32_t hostlong);
uint16_t htons(uint16_t hostshort);
uint32_t ntohl(uint32_t netlong);
uint16_t ntohs(uint16_t netshort);
```

The following are declared as functions, and may also be defined as macros:

```cpp
in_addr_t      inet_addr(const char *cp);
in_addr_t      inet_lnaof(struct in_addr in);
struct in_addr inet_makeaddr(in_addr_t net, in_addr_t lna);
in_addr_t      inet_netof(struct in_addr in);
in_addr_t      inet_network(const char *cp);
char          *inet_ntoa(struct in_addr in);
```

# Socket()
Upon successful completion, socket() shall return a non-negative integer, the socket file descriptor. Otherwise, a value of -1 shall be returned and errno set to indicate the error.
