// Client to send ping requests over UDP (USER DATAGRAM PROTOCOL)
// 24/07/14
// Peter Kydd
// http://www.linuxhowtos.org/C_C++/socket.htm

#define ARG_COUNT 3
#define PORT 1025
#define SERVER "127.0.0.1"
#define MILLI_MULT 100000

#include <stdio.h>         // standard input/output
#include <stdlib.h>        // standard libraries EXIT_SUCCESS, etc
#include <unistd.h>        // **not sure if needed**
#include <string.h>        // string functions
#include <sys/types.h>     // definitions of data types used below (in socket,h etc)
#include <sys/socket.h>    // definitions/structures required for sockets
#include <netinet/in.h>    // constants and structures required for domain addresses
#include <netdb.h>         // defines structure hostent
#include <sys/time.h>      // for time

// function prototype
void error(char *msg);

// structs below are given in headers above, however, they are also here for my own edification. 


typedef struct _in_addr {
   unsigned long s_addr;
} in_addr;

typedef struct _sockAddr_in {
   short sin_family;
   unsigned short sin_port;
   in_addr s_addr;
   char sin_zero[8];       // not used, must be zero
} sockAddr_in;
   
typedef struct  _hostent{
   char    *h_name;                 // official name of host
   char    **h_aliases;             // alias list 
   int     h_addrtype;              // host address type 
   int     h_length;                // length of address 
   char    **h_addr_list;           // list of addresses from name server 
   #define h_addr  h_addr_list[0]   // address, for backward compatiblity
} hostent;

   
int main(int argc, char *argv[]){

   int exit = EXIT_SUCCESS;   
   int sockfd = 0;    // array subscriptor for system call
   int portNum = 0;    // port number on which the client will recieve connections
   int n = 0;         // number of chars read or written in read() write() calls
   
   struct sockaddr_in serv_addr;
   struct hostent *server;
   

   char buffer[256];  //The client reads characters from the socket connection into this buffer.
   
   if (argc < ARG_COUNT){
      printf("usage pingClient <hostname> <port>\n");
      exit = EXIT_FAILURE;
   
   } else {
      
      portNum = atoi(argv[2]);
     
      sockfd = socket(AF_INET, SOCK_DGRAM, 0);// Create a network socket with system call
      
      if (sockfd < 0){                         // socket() returns -1 if error occurs
         error("ERROR opening socket");   
      }
      
      server = gethostbyname(argv[1]); //take server address from command line
      
      if (server == NULL) {
         fprintf(stderr,"ERROR, no such host\n");
         return EXIT_FAILURE;
      }      
      
      memset((char *) &serv_addr, 0, sizeof(serv_addr));
      
      // populate the server struct
      serv_addr.sin_family = AF_INET;
      bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
      serv_addr.sin_port = htons(portNum);
       
      if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) {
         error("ERROR connecting");
         return EXIT_FAILURE;
      }
      
      
      // set up time information here 
      
      struct timeval stop, start;
      double RTT_VAL;
      double tStart;
      double tStop;
      
      printf("Please enter the message: ");
      memset(buffer,0,256);
           
      while(fgets(buffer,255,stdin)){
         
         gettimeofday(&start, NULL);   
         
         n = write(sockfd,buffer,strlen(buffer));
         
         if (n < 0){ 
            error("ERROR writing to socket");
            return EXIT_FAILURE;
         }
         
         
         memset(buffer,0,256);
         n = read(sockfd,buffer,255);
         gettimeofday(&stop, NULL);
         

         
         if (n < 0){ 
            error("ERROR reading from socket");
            return EXIT_FAILURE;
         }
      
         tStart = (int)start.tv_usec;
         tStop = (int)stop.tv_usec;
              
         RTT_VAL = ((tStop - tStart)/MILLI_MULT);   
         printf("PING: %s on PORT: %s. RTT(ms): %f \n", argv[1],argv[2], RTT_VAL);
      
         printf("Please enter the message: ");
         memset(buffer,0,256);   
      
      }
      
      
      close(sockfd);
   }
   
   return exit;
}

void error(char *msg){

   // void perror(const char *str) prints a descriptive error message to stderr
   perror(msg);
}

