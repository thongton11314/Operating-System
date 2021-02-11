#include <pthread.h>
#include <unistd.h>
#include <iostream>

using namespace std;

void * ThreadFunc(void*);

// When compile need to link -lpthread
int main () {

    // thread variable
    pthread_attr_t attr;
    pthread_t tid;

    // initialize thread
    pthread_attr_init(&attr);

    // create thread by id, attribute, function
    int err = pthread_create(&tid, &attr, ThreadFunc, NULL);

    cout << "In Main" << endl;
    for (int i = 0; i < 10; i++) {
        sleep(1);
        cout << "Main counts:" << i << endl;
    }
    cout << "Main End" << endl;
    pthread_join(tid, nullptr);
    return 0;
}

void * ThreadFunc(void* args) {
    cout << "In Thread" << endl;
    for (int i = 0; i < 10; i++) {
        sleep(3);
        cout << "Thread counts:" << i << endl;
    }
    cout << "Thread end" << endl;
}