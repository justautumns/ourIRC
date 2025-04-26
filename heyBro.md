## Socket Functions 
### Class Name -> <sys/socket.h>
- socket()	->  `creates a new socket`
- close()	-> `to close the socket`
- setsockopt() -> `to optimize the socket specialities`
- getsockname() -> `gets the adress of a socket which is connected`
- bind() -> `binds the socket IP or port`
- connect() -> `connects to the server :D I could skip these ones`
- listen () -> `I think it's obvious what it does :D`
- accept() -> `accepts the connection and returns new socket`

## Server Side Functions
### Class Name -> <arpa/inet.h> ve <netdb.h>
- htonl() -> `32 host → network byte order`
- htons() -> `16-bit host → network byte order (for port numbers)`
- ntohl() -> `32-bit network → host byte order`
- ntohs() -> `16-bit network → host byte order`
- inet_addr() -> `IP string (exp: "192.168.1.1") → 32-bit binary (network byte order)`
- inet_ntoa() -> `32-bit binary → IP string`
- gethostbyname() -> `Hostname → IP (DNS check)`
- getprotobyname() -> `Protokol adı → protocol number (Exp: "tcp" → 6)`
- getaddrinfo()	-> `Modern adress Lösung(I coudn't find the word in English to explain this) (IPv4/IPv6 support)`
- freeaddrinfo() -> `getaddrinfo() frees the adress of getaddinfo function`


## Data Transfer
### Class Name -> (<sys/socket.h>)
- send() -> `send TCP data`
- recv() -> `receives TCP data`
## Singnal Management
### ClassName -> (<signal.h>)
- signal() -> `I am pretty sure this will be included in your part :D`
- sigaction() -> `Same :D`
## File/Soket Control 
### ClassName -> (<unistd.h>, <sys/stat.h>, <fcntl.h>)
- lseek() -> `Changes the file location`
- fstat() -> `gets the file info description`
- fcntl() -> `Sets the file descriptor's specialities (Exp: O_NONBLOCK)`
## Event Polling 
### ClassName -> (<poll.h>)
-  poll() -> `Wait for events in multi socket system (alternative to select())`



#### ALL CLASSES AND FUNCTIONS TO REMEMBER
- #include <sys/socket.h>  // socket(), bind(), listen(), accept(), etc.
- #include <netinet/in.h>  // sockaddr_in, htonl(), htons(), etc.
- #include <arpa/inet.h>   // inet_addr(), inet_ntoa()
- #include <netdb.h>       // getaddrinfo(), gethostbyname()
- #include <unistd.h>      // close(), lseek()
- #include <fcntl.h>       // fcntl(), O_NONBLOCK
- #include <poll.h>        // poll()
- #include <signal.h>      // signal(), sigaction()`
