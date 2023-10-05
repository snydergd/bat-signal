
#ifndef DELAYABLE_LOOPER_MAX_TASKS
#define DELAYABLE_LOOPER_MAX_TASKS 10
#endif
class DelayableLooper {
    public:
        DelayableLooper();

        void addLoop(void (*task)());
        void loop();
        void delay(int millis); // called from within 

    private:
        // array of tasks that addLoop adds to
        void (*tasks[DELAYABLE_LOOPER_MAX_TASKS])();
        unsigned long timeouts[DELAYABLE_LOOPER_MAX_TASKS];
        int numTasks;
        int currentTask;
};
