// this is schduler_rr

// include libraries needed
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "schedulers.h"
#include "list.h"
#include "cpu.h"

// protyping
Task *pickNextTask();

struct node **g_head = NULL;

// add a task to the list 
void add(char *name, int priority, int burst) {
    if (g_head == NULL) {
        g_head = malloc(sizeof(struct node));
        *g_head = NULL;
    }

    // create new task
    Task *newTask = malloc(sizeof(Task));
    newTask->name = name;
    newTask->priority = priority;
    newTask->burst = burst;
    insert(g_head, newTask);
}

// invoke the scheduler
void schedule() {
    int timeNow = 0;
    int currentBurst = 0;
    while (*g_head != NULL) {

        // get teh task to be execute
        Task *temp = pickNextTask();

        if (temp->burst > QUANTUM) {
            currentBurst = QUANTUM;
            insert(g_head, temp);
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

// based on traverse from list.c
// finds the task whose name comes first in dictionary
Task *pickNextTask() {

  // if list is empty, nothing to do
  if (!*g_head)
    return NULL;

  struct node *temp;
  temp = *g_head;
  Task *best_sofar = temp->task;

  while (temp != NULL) {
      best_sofar = temp->task;
      temp = temp->next;
  }  

  // delete the node from list, Task will get deleted later
  delete (g_head, best_sofar);
  return best_sofar;
}