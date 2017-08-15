#ifndef EVENT_H
#define EVENT_H

#include <string>
#include "Process.h"
enum Transition { TRANS_TO_READY, TRANS_TO_RUN, TRANS_TO_BLOCK, TRANS_TO_PREEMPT, TRANS_TO_DONE };

class Event {
public:
    Event(int evtTimeStamp, Process* evtProcess, Transition evtTrans) {
        this->evtTimeStamp = evtTimeStamp;
        this->evtProcess = evtProcess;
        this->evtTrans = evtTrans;
    }

    int evtTimeStamp;
    Process* evtProcess;
    Transition evtTrans;
};

#endif
