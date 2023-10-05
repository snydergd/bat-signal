
#include "delayable_looper.h"
#include <Arduino.h>

DelayableLooper::DelayableLooper() {
    for (int i = 0; i < DELAYABLE_LOOPER_MAX_TASKS; i++) {
        tasks[i] = 0;
        timeouts[i] = 0;
    }
}
void DelayableLooper::addLoop(void (*task)()) {
    if (numTasks < DELAYABLE_LOOPER_MAX_TASKS) {
        tasks[numTasks] = task;
        timeouts[numTasks] = 0;
        numTasks++;
    } else {
        Serial.println("ERROR: too many tasks");
    }
}
void DelayableLooper::loop() {
    for (currentTask = 0; currentTask < numTasks; currentTask++) {
        // last check is to make sure that the timeout hasn't wrapped around
        if (timeouts[currentTask] == 0 || (millis() > timeouts[currentTask] && millis()-timeouts[currentTask] < 100000)) {
            timeouts[currentTask] = 0;
            tasks[currentTask]();
        }
    }
}
void DelayableLooper::delay(int millisToAdd) {
    timeouts[currentTask] = millis() + millisToAdd;
    if (timeouts[currentTask] == 0) {
        timeouts[currentTask] = 1; // 0 is reserved for "no timeout"
    }
}
