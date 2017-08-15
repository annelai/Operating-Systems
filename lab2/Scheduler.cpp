#include "Scheduler.h"

using namespace std;

FCFS::FCFS() : Scheduler() {}
void FCFS::add_process(Process* evtProcess) {
    process_queue.push_back(evtProcess);
}
Process* FCFS::get_next_process(){
    if (process_queue.empty()) return NULL;
    else {
        Process* next_process = process_queue.front();
        process_queue.erase(process_queue.begin());
        return next_process;
    }
}


LCFS::LCFS() : Scheduler() {}
void LCFS::add_process(Process* evtProcess) {
    process_queue.insert(process_queue.begin(), evtProcess);
}
Process* LCFS::get_next_process(){
    if (process_queue.empty()) return NULL;
    else {
        Process* next_process = process_queue.front();
        process_queue.erase(process_queue.begin());
        return next_process;
    }
}


SJF::SJF() : Scheduler() {}
void SJF::add_process(Process* evtProcess) {
    for (vector<Process*>::iterator it = process_queue.begin(); it != process_queue.end(); ++it) {
        if (evtProcess->RC < (*it)->RC) {
            process_queue.insert(it, evtProcess);
            return;
        }
    }
    process_queue.push_back(evtProcess);
}
Process* SJF::get_next_process(){
    if (process_queue.empty()) return NULL;
    else {
        Process* next_process = process_queue.front();
        process_queue.erase(process_queue.begin());
        return next_process;
    }
}


RR::RR(int quantum){
    this->quantum = quantum;
}
void RR::add_process(Process* evtProcess) {
    process_queue.push_back(evtProcess);
}
Process* RR::get_next_process(){
    if (process_queue.empty()) return NULL;
    else {
        Process* next_process = process_queue.front();
        process_queue.erase(process_queue.begin());
        return next_process;
    }
}

PRIO::PRIO(int quantum){
    this->quantum = quantum;
}
void PRIO::add_process(Process* evtProcess) {
    bool added = false;
    if (evtProcess->D_PR == -1 ) { // add to expired queue
        evtProcess->D_PR = evtProcess->S_PR - 1;
        if (expired_process_queue.empty())
            expired_process_queue.push_back(evtProcess);
        else {
            for (vector<Process*>::iterator it = expired_process_queue.begin(); it != expired_process_queue.end(); ++it) {
                if (evtProcess->D_PR > (*it)->D_PR) {
                    expired_process_queue.insert(it, evtProcess);
                    added = true;
                    break;
                }
            }
            if (added == false) expired_process_queue.push_back(evtProcess);
        }
    }
    else {  // add to active queue
        if(process_queue.empty())
            process_queue.push_back(evtProcess);
        else {
            for (vector<Process*>::iterator it = process_queue.begin(); it != process_queue.end(); ++it) {
                if (evtProcess->D_PR > (*it)->D_PR) {
                    process_queue.insert(it, evtProcess);
                    added = true;
                    break;
                }
            }
            if (added == false) process_queue.push_back(evtProcess);
        }
    }
}
Process* PRIO::get_next_process() {
    if(process_queue.empty()) { // if active queue is empty, swap!
        vector<Process*> tmp = process_queue;
        process_queue = expired_process_queue;
        expired_process_queue = tmp;
    }

    if (process_queue.empty()) {
        return NULL;
    }
    else {
        Process* next_process = process_queue.front();
        process_queue.erase(process_queue.begin());
        return next_process;
    }
}
