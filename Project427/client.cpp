#include <iostream>
#include <sys/time.h>
#include <netdb.h>
#include <string>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/uio.h>      // writev
#include <unistd.h>
#include <vector>


using namespace std;

int main(int argc, char *argv[])
{
    if (argc != 7)
    {
        cout << argc << endl;
        cerr << "Six Parameter Required" << endl;;
        return -1;
    }

    int socket = atoi(argv[1]);
    int nbufs = atoi(argv[3]);
    int bufsize = atoi(argv[4]);
    int repetition = atoi(argv[2]);
    char databuf[nbufs][bufsize];
    cout << socket << endl;
    struct timeval curr_time;
    gettimeofday(&curr_time, NULL);
    unsigned long long startTime = curr_time.tv_usec;
    char *ipAddr = argv[5];

    for (int i = 0; i < nbufs; i++)
    {
        for (int j = 0; j < bufsize; j++)
        {
            databuf[i][j] = 'A';
        }
    }
    struct hostent *host = gethostbyname(ipAddr);
    sockaddr_in sendSockAddr;
    bzero((char *)&sendSockAddr, sizeof(sendSockAddr));
    sendSockAddr.sin_family = AF_INET;
    sendSockAddr.sin_addr.s_addr = inet_addr(inet_ntoa(*(struct in_addr *)
    *host->h_addr_list)); //assign ip address into sendSockAddr header inet_ntoa converts into dot notation, inent_addr converts into IPV4
    sendSockAddr.sin_port = htons(socket);

    int clientSd = ::socket(AF_INET, SOCK_STREAM, 0);
    int connectStatus = connect(clientSd, (sockaddr *)&sendSockAddr, sizeof(sendSockAddr));
    if (connectStatus < 0) {
        cerr << "Failed to connect to Server" << endl;
        return -1;
    }
    for (int j = 0; j < repetition; j++)
    {
        int type = atoi(argv[6]);
        if (type == 1)
        {
            for (int i = 0; i < nbufs; i++)
            {
                write(clientSd, databuf[i], bufsize);
            }
        }
        else if (type == 2)
        {
            struct iovec vector[nbufs];
            for (int i = 0; i < nbufs; i++)
            {
                vector[i].iov_base = databuf[i];
                vector[i].iov_len = bufsize;
            }
            writev(clientSd, vector, nbufs);
        }
        else if (type == 3)
        {
            write(clientSd, databuf, nbufs * bufsize);
        }
    }

    int count = 0;
    read(clientSd, &count, sizeof(count));
    gettimeofday(&curr_time, NULL);
    unsigned long long roundTrip = curr_time.tv_usec - startTime;
    std::cout << "Test 1: data-sending time = " <<  startTime << " usec, round-trip time = " << roundTrip << " usec, #reads = " << count << endl;
    close(clientSd);

    return 0;
}

