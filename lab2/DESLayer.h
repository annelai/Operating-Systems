#ifndef DESLAYER_H
#define DESLAYER_H

#include <vector>
#include <string>
#include "Event.h"

using namespace std;

class DESLayer {
public:
    void put_event(int evtTimeStamp, Process* evtProcess, Transition evtTrans) {
        for (vector<Event*>::iterator it = event_queue.begin(); it != event_queue.end() ; ++it) {
            if (evtTimeStamp < (*it)->evtTimeStamp) {
                event_queue.insert(it, new Event(evtTimeStamp, evtProcess, evtTrans));
                return;
            }

        }
        event_queue.push_back(new Event(evtTimeStamp, evtProcess, evtTrans));
    }

    Event* get_event() {
        if (event_queue.size() == 0) return NULL;
        else {
            Event* next_event = event_queue.front();
            event_queue.erase(event_queue.begin());
            return next_event;
        }
    }

    void show_event() {
        cout << "show_event\n";
        for (vector<Event*>::iterator it = event_queue.begin(); it != event_queue.end(); ++it) {
            cout << (*it)->evtProcess->pid << endl;
        }
    }

    int get_next_event_time() {
        if (event_queue.size() == 0) return -1;
        else return event_queue.front()->evtTimeStamp;
    }

    vector<Event*> event_queue;
    string info;
};

#endif
