#include<iostream>
#include <sys/types.h>    // socket, bind
#include <sys/socket.h>   // socket, bind, listen, inet_ntoa
#include <netinet/in.h>   // htonl, htons, inet_ntoa
#include <arpa/inet.h>    // inet_ntoa
#include <netdb.h>     // gethostbyname
#include <unistd.h>    // read, write, close
#include <strings.h>     // bzero
#include <netinet/tcp.h>  // SO_REUSEADDR
#include <sys/uio.h>      // writev
#include <sys/time.h>
#include <string.h>

struct thread_data {
    int repetition;
    int sd;
};

using namespace std;
const int BUF_SIZE = 1500;

void * ServerThread(void * storage);
void* count_reads(void * ptr);

int main(int argc, char* argv[] ) {
    if (argc != 3) {
        cerr << "Invalid Parameters" << endl;
    }
    int repetition = atoi(argv[2]);

    int server_port = atoi(argv[1]);  // the last 4 digits of your student id

    sockaddr_in acceptSock;
    bzero((char*) &acceptSock, sizeof(acceptSock));  // zero out the data structure
    acceptSock.sin_family = AF_INET;   // using IP
    acceptSock.sin_addr.s_addr = htonl(INADDR_ANY); // listen on any address this computer has
    acceptSock.sin_port = htons(server_port);  // set the port to listen on

    int serverSd = socket(AF_INET, SOCK_STREAM, 0); // creates a new socket for IP using TCP

    const int on = 1;

    setsockopt(serverSd, SOL_SOCKET, SO_REUSEADDR, (char *) &on, sizeof(int));  // this lets us reuse the socket without waiting for hte OS to recycle it

    // Bind the socket

    bind(serverSd, (sockaddr*) &acceptSock, sizeof(acceptSock));  // bind the socket using the parameters we set earlier
    
    // Listen on the socket
    int n = 5;
    listen(serverSd, n);  // listen on the socket and allow up to n connections to wait.

    // Accept the connection as a new socket

    sockaddr_in newsock;   // place to store parameters for the new connection
    socklen_t newsockSize = sizeof(newsock);

    while (1) {
        int newSd = accept(serverSd, (sockaddr *) &newsock, &newsockSize);
        thread_data* storage = new thread_data;
        storage->repetition = repetition;
        storage->sd = newSd;
        pthread_t newThread;
        pthread_create(&newThread, NULL, ServerThread, (void *) storage);
        
    }
    
    
    
}


void* ServerThread(void * storage) {
    thread_data * threadData = (thread_data *)storage;
    char databuf[BUF_SIZE];
    int serverSd = threadData->sd;
    int repetition = threadData->repetition;

    timeval curr_time;
    gettimeofday(&curr_time, NULL);
    unsigned long long startTime = curr_time.tv_usec;
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 5000;
    setsockopt(threadData->sd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);

    int count = 0;
    iovec temp;
    for (int i = 0; i < repetition; i++) {
        int nRead = 0;
        for ( int nRead = 0; (nRead += read(serverSd, databuf, BUF_SIZE - nRead)) < BUF_SIZE; ++count );
     count++;
    }
    gettimeofday(&curr_time, NULL);
    unsigned long long recievingTime = curr_time.tv_usec;
    cout << "data-recieving time = "<< recievingTime << " usec" << endl;
    write(serverSd, &count, sizeof(count));
    close(threadData->sd);
}
