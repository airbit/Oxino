#ifndef TOS_H
#define TOS_H

#define ATOS_VERSION (001)

#include "Arduino.h"

#if !defined (__CC3200R1M1RGC__)
#error Sorry, this MCU is only support CC3200 for now!
#endif

#define ATOS_OK                 0
#define ATOS_ERROR              (-1)

#define TASK_RET_INPROG         0
#define TASK_RET_DONE           1

#define TASK_STATE_RUN          2
#define TASK_STATE_WAIT         1
#define TASK_STATE_DONE         0

typedef int (*P_TOS_TASK_FN)(void *pValue);

struct TaskEntry {
    const char *name;
    P_TOS_TASK_FN fn;
    void *params;
    unsigned long planned;
    char state;
};

class TaskSemaphore {
public:
    TaskSemaphore() : _task(NULL) {
    }

    void signal();

    int wait();

private:
    TaskEntry *_task;
};

class ATOS {
public:
    void begin(uint16_t timeout = 0);

    int createTask(P_TOS_TASK_FN fn);

    int createTask(P_TOS_TASK_FN fn, void *params);

    int createTask(P_TOS_TASK_FN fn, const char *name);

    int createTask(P_TOS_TASK_FN fn, const char *name, void *params);

    void sleep(unsigned long ms);

    int stop();

    int enable();

    int disable();

    void reset();

private:
    int findTask(P_TOS_TASK_FN fn);

    int findTask(const char *name);
};


#endif //TOS_H
