// The server will create a tcp Socket that listens on a port(the last 4
// digits of your ID number. The server will accept an incoming connection
// and then create a new thread(use the pthreads lobrary) that will handle
// the connection. The new thread will read all the data from the client
// and respond back to it (acknowledgement). The response detail will be
// provided in Server.cpp

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>      // bzero
#include <sys/socket.h>   // socket, bind, listen, inet_ntoa
#include <netinet/in.h>   // htonl, htons, inet_ntoa
#include <sys/types.h>    // socket, bind
#include <sys/time.h>     // gettimeofday
#include <sys/uio.h>      // writev
#include <netdb.h>        // gethostbyname
#include <unistd.h>       // read, write, close
#include <arpa/inet.h>    // inet_ntoa
#include <netinet/tcp.h>  // SO_REUSEADDR
#include <pthread.h>

const int BUFSIZE = 1500;
int serverSD;
int repetition;
using namespace std;

//Forward declaration of thread_data
struct thread_data
{
  int repetition;
  int sd;
};

// The server must include your_function, which is the function called
// by the new thread.
void *count_reads(void *ptr)
{
  int thread_ptr = ((struct thread_data*)ptr)->sd;
  struct timeval start;
  struct timeval stop;
  int count;
  int elapsed_time;

  // 1. Allocate databuf[BUFSIZE] where BUFSIZE = 1500
  char databuf[BUFSIZE];

  // 2. Start a timer by calling gettimeofday
  gettimeofday(&start, NULL); // Timezone not specified

  // 3. Repeat reading data from client int databuf[BUFSIZE]
  for(int i = 0; i < repetition; i++)  
  {
    for(int nRead = 0; (nRead += read(thread_ptr, databuf, BUFSIZE - nRead)) < BUFSIZE; count++);
  }
  // 4. Stop the timer by calling gettimeofday
  gettimeofday(&stop, NULL); // Timezone not specified;

  // 5. Send the number of read() calls made, (ie., count in the above)
  //    as an acknowledgement
  write(thread_ptr, &count, sizeof(count));

  // 6. Print out the statistics as shown below
  elapsed_time = (((stop.tv_sec - start.tv_sec) * 1000000L) + (stop.tv_usec - start.tv_usec));
  cout << "data receiving time = " << elapsed_time << " usec" << endl;

  // 7. close the connection
  close(serverSD);

  // 8. Optionally, terminate the server process by calling exit(0)
  exit(0);

}

int main(int argc, char const *argv[])
{
  int port = atoi(argv[1]);
  repetition = atoi(argv[2]);
  int n_connection_rq = 5; // not sure how many this is supposed to be
  
  // 1. Declare a sockaddr_in structure, zero initialize by calling bzero,
  //    and set its data members as follows
  sockaddr_in acceptSockAddr;
  bzero((char*)&acceptSockAddr, sizeof(acceptSockAddr));
  acceptSockAddr.sin_family = AF_INET; // Address family Internet
  acceptSockAddr.sin_addr.s_addr = htonl(INADDR_ANY);
  acceptSockAddr.sin_port = htons(port);

  // 2. Open a stream-oriented socket with the Internet address family
  serverSD = socket(AF_INET, SOCK_STREAM, 0);

  // 3. Set the SO-REUSEADDR options
  const int on = 1;
  setsockopt(serverSD, SOL_SOCKET, SO_REUSEADDR, (char*)&on, sizeof(int));

  // 4. Bind this socket to its local address by calling bind as passing the
  //    following arguement: the socket descriptor, the sockaddr_in structure
  //    defined above, and its data size
  bind(serverSD, (sockaddr*)&acceptSockAddr, sizeof(acceptSockAddr));

  // 5. Instruct the operating system to listen up to n connection requests
  //    from clients at a time by calling listen
  listen(serverSD, n_connection_rq);

  // 6. Recieve a request from a client by calling accept that will return
  //    a new socket specific to this connection request
  sockaddr_in newSockAddr;
  socklen_t newSockAddrSize = sizeof(newSockAddr);
  while(1)
    {
      int newSD = accept(serverSD, (sockaddr*)&newSockAddr, &newSockAddrSize);
      if(newSD < 0)
        {
           perror("Error: failure to accept new socket");
	         cout << endl;
	         exit(EXIT_FAILURE);
        }
      pthread_t new_thread;
      struct thread_data *data = new thread_data;
      data->repetition = repetition;
      data->sd = newSD;
      int iretl = pthread_create(&new_thread, NULL, count_reads, (void*) data);// count_reads was thread_server
    }

  return 0;
}