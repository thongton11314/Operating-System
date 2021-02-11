// this is schduler_priotiry_rr

// include libraries needed
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "schedulers.h"
#include "list.h"
#include "cpu.h"

// typedef define name
typedef struct node node;

// protyping
Task *pickNextTask(int priority);

node **g_heads[MAX_PRIORITY + 1] = { NULL, NULL, 
                                    NULL, NULL, 
                                    NULL, NULL, 
                                    NULL, NULL, 
                                    NULL, NULL, 
                                    NULL };

// add a task to the list 
void add(char *name, int priority, int burst) {

    // create new task
    Task *newTask = malloc(sizeof(Task));
    newTask->name = name;
    newTask->priority = priority;
    newTask->burst = burst;

    if (g_heads[priority] == NULL) {
        g_heads[priority] = malloc(sizeof(node));
        *g_heads[priority] = NULL;
    }   
    insert(g_heads[priority], newTask);
}

// invoke the scheduler
void schedule() {
    int timeNow = 0;
    int currentBurst = 0;
    
    // run through all lists
    for (int i = MAX_PRIORITY; i >= MIN_PRIORITY; i--) {

        // check if list is not empty
        if (g_heads[i] != NULL) {

            // run task in list
            while (*g_heads[i] != NULL) {               
               
                // get the task to be execute
                Task *temp = pickNextTask(i);

                if (temp->burst > QUANTUM) {
                    currentBurst = QUANTUM;
                    insert(g_heads[i], temp);
                    run(temp, currentBurst);
                    temp->burst -= QUANTUM;
                }
                else {
                    currentBurst = temp->burst;
                    run(temp, currentBurst);
                }

                // get current time
                timeNow += currentBurst;
                printf("\tTime is now: %d\n", timeNow);
            }               
        }
    }
}

// based on traverse from list.c
// finds the task whose name comes first in dictionary
Task *pickNextTask(int priority) {

  // if list is empty, nothing to do
  if (!*g_heads[priority])
    return NULL;

  struct node *temp;
  temp = *g_heads[priority];
  Task *best_sofar = temp->task;

  while (temp != NULL) {
      best_sofar = temp->task;
      temp = temp->next;
  }

  // delete the node from list, Task will get deleted later
  delete (g_heads[priority], best_sofar);
  return best_sofar;
}