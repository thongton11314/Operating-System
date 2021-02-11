#include <setjmp.h> // setjmp( )
#include <signal.h> // signal( )
#include <unistd.h> // sleep( ), alarm( )
#include <stdio.h>  // perror( )
#include <stdlib.h> // exit( )
#include <iostream> // cout
#include <string.h> // memcpy
#include <queue>    // queue

/*
Initializes the sthread scheduler. It must be called before
launching any sthreads.
*/
#define scheduler_init()            \
{			                           \
   if (setjmp(main_env) == 0)	      \
   {                                \
      scheduler();				      \
   }                                \
}

/*
Starts the sthread scheduler. It must be called after
launching all sthreads
*/
#define scheduler_start()           \
{			                           \
   if (setjmp(main_env) == 0)       \
   {                                \
      longjmp( scheduler_env, 1 );  \
   }                                \
}

/*
Captures the current thread's jmp_env and activation
record into cur_tcb. This is a helper function called from
sthread_init( ) and sthread_yield( ). 
*/
#define capture()                                                      \
{                                                                      \
   /* create stack pointer, and base pointer */                        \
   register void *sp asm ("sp");                                       \
   register void *bp asm ("bp");                                       \
                                                                       \
   /* get size */                                                      \
   cur_tcb->size_ = (int)((long long int)bp - (long long int)sp);      \
   cur_tcb->sp_ = sp;                                                  \
   cur_tcb->stack_ = new int [cur_tcb->size_];                         \
                                                                       \
   /* copy memory */                                                   \
   memcpy(cur_tcb->stack_, sp, cur_tcb->size_);                        \
                                                                       \
   /* put it in queue */                                               \
   thr_queue.push(cur_tcb);                                            \
}

/*
Is called by each user thread to voluntarily yield the
CPU. Only after an timer interrupt, calls capture( ) and
goes back to the scheduler. When the control comes
back from the scheduler, retrieve this thread activation
record from cur_tcb->stack.
*/  
#define sthread_yield()                                            \
{                                                                  \
                                                                   \
   /* This is done by other part */                                \
    if (alarmed) {                                                 \
        alarmed = false;                                           \
                                                                   \
        /* save current tcb register content */                    \
        if (setjmp(cur_tcb->env_) == 0) {                          \
                                                                   \
            /* capture new track */                                \
            capture();                                             \
                                                                   \
            /* jump to the scheduler */                            \
            longjmp(scheduler_env, 1);                             \
        }                                                          \
                                                                   \
        /* retrieve  */                                            \
        memcpy(cur_tcb->sp_, cur_tcb->stack_, cur_tcb->size_);	    \
   }                                                               \
}

/*
Is called by each user thread as soon as it starts for going
back to the main( ) program.
*/
#define sthread_init()              \
{                                   \
   if (setjmp(cur_tcb->env_) == 0 ) \
   {                                \
      capture();                    \
      longjmp(main_env, 1);	      \
   }                                \
   memcpy(cur_tcb->sp_, cur_tcb->stack_, cur_tcb->size_);	\
}

/*
Launches a new thread that invokes a given function as
passing arguments to it.
*/
#define sthread_create(function, arguments) \
{                                           \
   if (setjmp(main_env) == 0)               \
   {                                        \
      func = &function;				           \
      args = arguments;				           \
      thread_created = true;			        \
      cur_tcb = new TCB();			           \
      longjmp(scheduler_env, 1);            \
   }                                        \
}

/*
Is called when the current thread terminates itself. It
deallocates cur_tcb->stack and jumps to the scheduler.
*/
#define sthread_exit()              \
{			                           \
   if (cur_tcb->stack_ != NULL)		\
   {                                \
      free(cur_tcb->stack_);			\
   }                                \
   longjmp(scheduler_env, 1);		   \
}


using namespace std;
static jmp_buf main_env;
static jmp_buf scheduler_env;
const int kTimeQuantum = 5;

// Thread control block
class TCB 
{
public:
   TCB() : sp_(NULL), stack_(NULL), size_(0) {}
   jmp_buf env_;  // the execution environment captured by set_jmp()
   void* sp_;     // the stack pointer 
   void* stack_;  // the temporary space to maintain the latest stack contents
   int size_;     // the size of the stack contents
};

static TCB* cur_tcb;   // the TCB of the current thread in execution

// The queue of active threads
static queue<TCB*> thr_queue;

// Alarm caught to switch to the next thread
static bool alarmed = false;
static void sig_alarm(int signo) 
{
   alarmed = true;
}

// A function to be executed by a thread
void (*func)(void *);
void *args = NULL;
static bool thread_created = false;

static void scheduler() 
{
   // invoke scheduler
   if (setjmp(scheduler_env) == 0) 
   {
      cerr << "scheduler: initialized" << endl;
      if (signal(SIGALRM, sig_alarm) == SIG_ERR) 
      {
         perror("signal function");
         exit(-1);
      }
      longjmp(main_env, 1);
   }

   // check if it was called from sthread_create()
   if ( thread_created == true ) 
   {
      thread_created = false;
      (*func)(args);
   }

   // restore the next thread's environment
   if ((cur_tcb = thr_queue.front()) != NULL) 
   {

      // many thread to one, run here
      thr_queue.pop();

      // allocate a time quantum
      alarm(kTimeQuantum);

      // return to the next thread's execution
      longjmp(cur_tcb->env_, 1);
   }

   // no threads to schedule, simply return
   cerr << "scheduler: no more threads to schedule" << endl;
   longjmp(main_env, 2);
}
