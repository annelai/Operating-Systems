#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <vector>

using namespace std;

struct IORequest {
    int idx;
    int ts;
    int track;
    int turnaround;
    int waittime;
};

class Scheduler {
public:
    Scheduler() {
        this->curr_track = 0;
    }

    virtual void add(IORequest* ioReq) = 0;
    virtual IORequest* get_next() = 0;

    vector<IORequest*> queue;
    int curr_track;
};

class FIFO : public Scheduler {
public:
    FIFO();
    void add(IORequest* ioReq);
    IORequest* get_next();
};

class SSTF : public Scheduler {
public:
    SSTF();
    void add(IORequest* ioReq);
    IORequest* get_next();
};

class SCAN : public Scheduler {
public:
    SCAN();
    void add(IORequest* ioReq);
    IORequest* get_next();

    int direction;
};

class CSCAN : public Scheduler {
public:
    CSCAN();
    void add(IORequest* ioReq);
    IORequest* get_next();
};

class FSCAN : public Scheduler {
public:
    FSCAN();
    void add(IORequest* ioReq);
    IORequest* get_next();

    int direction;
    vector<IORequest*> queue_inactive;
};

#endif
