//
// Created by 陶源 on 15/4/29.
//

#include "ATOS.h"
#include "Watchdog.h"
#include <IntervalTimer.h>

#ifdef __CC3200R1M1RGC__
#include <driverlib/prcm.h>
#include <driverlib/utils.h>
#include <WiFi.h>

#endif

const uint8_t MAX_TASKS = 9; //max allowed g_tasks -1 (i.e.: 9 = 10-1)

static IntervalTimer OST;

volatile uint16_t _timeout;
static bool _initialized = false;
TaskEntry g_tasks[MAX_TASKS];
static TaskEntry *g_current_task;
static uint16_t _num_task = 0;


void _reboot() {
#ifdef __CC3200R1M1RGC__
    OST.end();

    MAP_PRCMMCUReset(1);
#else
#error Unknown how to reboot
#endif
}


void TaskSemaphore::signal() {
    //
    // Release the waiting task from wait state
    //
    if (_task != NULL) {
        _task->state = TASK_STATE_RUN;
        _task = NULL;
    }
}

int TaskSemaphore::wait() {
    //
    // Set the invoking task in wait state.
    // Only on task can use a sync object at a time
    //
    if (_task == NULL) {
        g_current_task->state = TASK_STATE_WAIT;
        _task = g_current_task;

        //
        // Return success
        //
        return ATOS_OK;
    }

    //
    // Return error
    //
    return ATOS_ERROR;
}


/////////////////////////////////////////////////////////////////



//ISR (Interrupt Service Routine)
void os_isr(void *params) {
    int i;
    int iRet;

    for (i = 0; i < _num_task; i++) {
        g_current_task = &g_tasks[i];
        if (TASK_STATE_RUN == g_current_task->state) {
            if (g_current_task->planned < millis()) {
                iRet = g_current_task->fn(g_current_task->params);
                if (TASK_RET_DONE == iRet) {
                    g_current_task->state = TASK_STATE_DONE;
                }
            }
        }
    }

    if (_timeout > 1) {
        Watchdog.reset();
    }
}


void ATOS::begin(uint16_t timeout) {
    _initialized = true;
    OST.begin(os_isr, 1000);
    _timeout = timeout;
    if (_timeout > 1) {
        Watchdog.enable(timeout);
    }
}

void ATOS::reset() {
    _reboot();
}

int ATOS::findTask(P_TOS_TASK_FN fn) {
    if (fn == NULL) return -1;
    for (int i = 0; i < _num_task; ++i) {
        if (g_tasks[i].fn == *fn) return i;
    }
    return -1;
}

int ATOS::findTask(const char *name) {
    if (name == NULL) return -1;
    for (int i = 0; i < _num_task; ++i) {
        if (g_tasks[i].name == NULL) continue;
        if (!(strcmp(g_tasks[i].name, name))) return i;
    }
    return -1;
}

int ATOS::createTask(P_TOS_TASK_FN fn) {
    return createTask(fn, NULL, NULL);
}

int ATOS::createTask(P_TOS_TASK_FN fn, const char *name) {
    return createTask(fn, name, NULL);
}

int ATOS::createTask(P_TOS_TASK_FN fn, void *params) {
    return createTask(fn, NULL, params);
}

int ATOS::createTask(P_TOS_TASK_FN fn, const char *name, void *params) {
    if (!_initialized || _num_task >= MAX_TASKS) {
        return ATOS_ERROR;
    }

    //
    // Initialize the task list if this is the first task created
    //
    if (_num_task == 0) {
        memset((char *) g_tasks, 0, sizeof(g_tasks));
    }

    //
    // Create the task
    //
    if (_num_task < MAX_TASKS) {
        g_tasks[_num_task].fn = fn;
        g_tasks[_num_task].state = TASK_STATE_RUN;
        g_tasks[_num_task].params = params;
        g_tasks[_num_task].planned = 0;
        _num_task++;

        return ATOS_OK;
    }

    return ATOS_OK;
}

void ATOS::sleep(unsigned long ms) {
    g_current_task->planned = millis() + ms;
}

int ATOS::stop() {
    if (!_initialized) {
        return ATOS_ERROR;
    }
    OST.end();
    return ATOS_OK;
}

int ATOS::enable() {
    if (!_initialized) {
        return ATOS_ERROR;
    }
    OST.enable();
    return ATOS_OK;
}

int ATOS::disable() {
    if (!_initialized) {
        return ATOS_ERROR;
    }
    OST.disable();
    return ATOS_OK;
}
