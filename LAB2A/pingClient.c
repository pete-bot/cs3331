// Client to send ping requests over UDP (USER DATAGRAM PROTOCOL)
// 24/07/14
// Peter Kydd
// http://www.linuxhowtos.org/C_C++/socket.htm

#define TRUE 1
#define FALSE 0
#define ARG_COUNT 3
#define PORT 1025
#define SERVER "127.0.0.1"
#define PINGMAX 10
#define TIMEOUT_DELAY 1000       // delay in ms for ping timeout

#include <stdio.h>         // standard input/output
#include <stdlib.h>        // standard libraries EXIT_SUCCESS, etc
#include <unistd.h>        // **not sure if needed**
#include <string.h>        // string functions
#include <sys/types.h>     // definitions of data types used below (in socket,h etc)
#include <sys/socket.h>    // definitions/structures required for sockets
#include <netinet/in.h>    // constants and structures required for domain addresses
#include <netdb.h>         // defines structure hostent
#include <sys/time.h>      // for time
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/select.h>

void error(char *msg);

int main(int argc, char *argv[]){

   struct sockaddr_in serv_addr;
   struct hostent *server;
   
   int exit = EXIT_SUCCESS;   
   int sockfd = 0;    // array subscriptor for system call
   int portNum = 0;    // port number on which the client will recieve connections
   int n = 0;         // number of chars read or written in read() write() calls
   char buffer[256];  // may need to adjust the zise of buffer - will not need 256 bytes
   
   if (argc < ARG_COUNT){
      printf("usage pingClient <hostname> <port>\n");
      exit = EXIT_FAILURE;
   
   } else {
      
      portNum = atoi(argv[2]);
     
      sockfd = socket(AF_INET, SOCK_DGRAM, 0);// Create a network socket with system call
      
      if (sockfd < 0){                         // socket() returns -1 if error occurs
         error("ERROR opening socket");   
         return EXIT_FAILURE;
      }
      
      server = gethostbyname(argv[1]); //take server address from command line
      
      if (server == NULL) {
         fprintf(stderr,"ERROR, no such host\n");
         return EXIT_FAILURE;
      }      
      
      memset((char *) &serv_addr, 0, sizeof(serv_addr));
      
      serv_addr.sin_family = AF_INET;
      bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
      serv_addr.sin_port = htons(portNum);
       
      if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) {
         error("ERROR connecting");
         return EXIT_FAILURE;
      }
       
      
      struct timeval start, stop, timeOut;
      int pingCount, rFlag;
      unsigned long long tStart, tStop, RTT_VAL;
      
      timeOut.tv_sec = 0;
      timeOut.tv_usec = 0;      
      
      memset(buffer,0,256);
      
      fd_set readSet;
 
      for(pingCount = 0; pingCount < PINGMAX; pingCount++ ){
         
         // clear buffer
         memset(buffer,0,256);
         
         // mark: packet send
         gettimeofday(&start, NULL);
         tStart = (unsigned long long)(start.tv_sec) * 1000 
            + (unsigned long long)(start.tv_usec) / 1000;
         
         // write out to byte stream over sockfd
         sprintf( buffer, "PING seqNum: %d/%d \r\n", (pingCount+1), PINGMAX);
         n = write(sockfd,buffer,strlen(buffer));
         
         if (n < 0){ 
            error("ERROR writing to socket");
            return EXIT_FAILURE;
         }
         
         // set select variables
         FD_ZERO(&readSet);        // clear the 'set'
         FD_SET(sockfd, &readSet); // add sockfd to the set
         timeOut.tv_usec = 1000000;
         
         rFlag = select((sockfd+1), &readSet, NULL, NULL, &timeOut);
         
         if(rFlag == -1) {
         
            error("ERROR reading from socket");
                  return EXIT_FAILURE;
            
         } else if( rFlag == 0) {
         
            printf("time out occurred\n\n");
            
         } else {
         
            memset(buffer,0,256);
            read(sockfd,buffer,255);
            gettimeofday(&stop, NULL);
                        
            // MARK: packet return
            tStop = (unsigned long long)(stop.tv_sec) * 1000 
            + (unsigned long long)(stop.tv_usec) / 1000;
         
            printf("MSG ECHO: %s", buffer);     
            RTT_VAL = (tStop - tStart); 
            printf("PING: %s on PORT: %s. RTT(ms): %3llu \n\n", argv[1],argv[2], RTT_VAL);
            
         }
      }
      
      
      close(sockfd);
   }
   
   return exit;
}

void error(char *msg){
   perror(msg);
}

