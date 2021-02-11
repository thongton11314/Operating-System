#include <iostream>
#include <signal.h>
#include <unistd.h>

using namespace std;
static bool alarmed = false;
static int int_count = 0;

static void sig_alarm(int signo);
static void sig_int(int signo);

int main() {
    if (signal(SIGALRM, sig_alarm) == SIG_ERR) {
        cerr << "Error setting signal" << endl; 
        exit(-1);
    }

    if (signal(SIGINT, sig_int) == SIG_ERR) {
        cerr << "Error setting signal" << endl;
        exit(-1);
    }

    while (true) {
        sleep(1);
        if(!alarmed) {
            alarm(3);
            alarmed = true;
        }
        cout << "Main loop..." << endl;
    }

    return 0;
}

void sig_alarm(int signo) {
    cout << "Alarm fired!!! " << endl;
    alarmed = false;
}

void sig_int(int signo) {
    if (int_count < 3)   {
        int_count++;
        cout << "Ouch!!! " << endl;
    }
    else {
        exit(-1);
    }
}
