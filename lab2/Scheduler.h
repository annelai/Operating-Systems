#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <vector>
#include "Process.h"
#include <iostream>

using namespace std;

class Scheduler {
public:
    Scheduler() {
        io_util = 0;
        finish_exe = 0;
    }
    virtual void add_process(Process* evtProcess) = 0;
    virtual Process* get_next_process() = 0;

    void show_process() {
        cout << "show_process\n";
        for (vector<Process*>::iterator it = process_queue.begin(); it != process_queue.end(); ++it) {
            cout << "id: " << (*it)->pid << " prio: " << (*it)->D_PR << endl;
        }
    }

    bool has_run() {
        return (!process_queue.empty());
    }
    vector<Process*> process_queue;
    int io_util;
    int finish_exe;
};

class FCFS : public Scheduler {
public:
    FCFS();
    void add_process(Process* evtProcess);
    Process* get_next_process();
};

class LCFS : public Scheduler {
public:
    LCFS();
    void add_process(Process* evtProcess);
    Process* get_next_process();
};

class SJF : public Scheduler {
public:
    SJF();
    void add_process(Process* evtProcess);
    Process* get_next_process();
};

class RR : public Scheduler {
public:
    RR(int quantum);
    void add_process(Process* evtProcess);
    Process* get_next_process();

private:
    int quantum;
};

class PRIO : public Scheduler {
public:
    PRIO(int quantum);
    void add_process(Process* evtProcess);
    Process* get_next_process();

private:
    int quantum;
    vector<Process*> expired_process_queue;
};

#endif
