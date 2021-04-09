// the client will create a new socket, connect to the server and send data
// using 3 different ways of writing data (data transferring). It will then
// wait for a response and output the response.
//
// port: a server IP port
// repetition: the repetition of sending a set of data buffers
// nbufs: the number of data buffers
// bufsize: the size of each data buffer (in bytes)
// serverIp: a server ip name
// type: the type of transfer scenario: 1, 2, or 3
// Resources used: https://www.geeksforgeeks.org/socket-programming-cc/
//                 https://stackoverflow.com/questions/12722904/how-to-use-struct-timeval-to-get-execution-time

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>      // bzero
#include <sys/socket.h>   // socket, bind, listen, inet_ntoa
#include <netinet/in.h>   // htonl. htons. inet_ntoa
#include <sys/types.h>    // socket, bind
#include <sys/time.h>     // gettimeofday
#include <sys/uio.h>      // writev
#include <netdb.h>        // gethostbyname
#include <unistd.h>       // read, write, close
#include <arpa/inet.h>    // inet_ntoa
#include <netinet/tcp.h>  // SO_REUSEADDR

const int BUFSIZE = 1500;

using namespace std;

int main(int argc, char const *argv[])
{
  int port = atoi(argv[1]);
  int repetition = atoi(argv[2]);
  int nbufs = atoi(argv[3]);
  int bufsize = atoi(argv[4]);
  const char *serverIP = argv[5];
  int type = atoi(argv[6]);
  struct timeval start;
  struct timeval stop;
  struct timeval lap;
  long transfer_time;
  long total_time;

  // 2. Retrieve a hostent structure corresponding to this IP name by
  //    calling gethostbyname().

  struct hostent* host = gethostbyname(serverIP);

  //3. Declare a sockaddr_in structure, zero-initialize it by calling
  //   b_zero, and set its data members as follows:

  sockaddr_in sendSockAddr;
  bzero((char*)&sendSockAddr, sizeof(sendSockAddr));
  sendSockAddr.sin_family = AF_INET; // Address Family Internet
  sendSockAddr.sin_addr.s_addr = inet_addr(inet_ntoa(*(struct in_addr*)*host->h_addr_list));
  sendSockAddr.sin_port = htons(port);

  // 4. Open a stream-oriented socket with the Internet address family
  int clientSD = socket(AF_INET, SOCK_STREAM, 0);
  if(clientSD < 0)
    {
      perror("Error: creating socket");
      close(clientSD);
      cout << endl;  
      exit(EXIT_FAILURE);
    }

  // 5. Connect this socket to the server by calling connect as passing
  //    the following arguements: the socket descripto, the sockaddr_in
  //    structure defined above, and its data size(obtained from the
  //    sizeof() function

  int connectStatus = connect(clientSD, (sockaddr*)&sendSockAddr, sizeof(sendSockAddr));
  if(connectStatus < 0)
    {
      perror("Error: Failed to connect to the server~~");
      close(clientSD);
      cout << endl;
      exit(EXIT_FAILURE);
    }
  
  // Allocate databuf[nbufs][bufsize] where nbufs * bufsize = 1500
  char databuf[nbufs][bufsize];

  // Start a timer by calling gettimeofday
  gettimeofday(&start, NULL); // no timezone specified
	//char vector_buf = malloc(sizeof(char * nbufs *bufsize));
  //start doing the write portion
  struct iovec vector[nbufs];
  for (int i = 0; i < repetition; i++)
    {
      if(type == 1) // multiple writes
				{
	  		 for (int j = 0; j < nbufs; j++)
	        {
	          write(clientSD, databuf[j], bufsize); // sd: socket descriptor
	        }
	     }
      else if(type == 2) // writev
	     {
	       for (int j = 0; j < nbufs; j++)
	        {
	          vector[j].iov_base = databuf[j];
	          vector[j].iov_len = bufsize;
	        }
	        writev(clientSD, vector, nbufs); // sd: socket descriptor
	     }
      else // type 3 implied, single write;
    	{
	      write(clientSD, databuf, nbufs * bufsize);
	    }
    }

  // 5.  Lap the timer by calling gettimeofday
  //     where lap - start = data-sending time.
  gettimeofday(&lap, NULL); // no timezone specified

  // 6. Receive from the server an integer acknowledgement that
  //    shows how many times the server called read().
  int read_count = 0;
  read(clientSD, &read_count, sizeof(read_count));

  // 7. Stop the timer by calling gettimeofday,
  //    where stop - start = tround-trip time.
  gettimeofday(&stop, NULL); // no timezone specified
  transfer_time = ((lap.tv_sec - start.tv_sec) * 1000000L)
    + (lap.tv_usec - start.tv_usec);
  total_time = ((stop.tv_sec - start.tv_sec) * 1000000L)
    + (stop.tv_usec - start.tv_usec);

  cout << "Test " << type << ": data-sending time = " << transfer_time << endl;
  cout << "                     roundtrip time    = " << total_time << endl;
  cout << "                     # reads           = " << read_count << endl;

  // 8. Close the socket by calling close
  close(clientSD);
  return 0;
}
