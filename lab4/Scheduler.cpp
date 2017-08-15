#include <cmath>        /* abs */
#include "Scheduler.h"

using namespace std;

FIFO::FIFO() : Scheduler() {}
void FIFO::add(IORequest* ioReq){
    queue.push_back(ioReq);
}
IORequest* FIFO::get_next() {
    if (queue.empty()) return NULL;
    IORequest* next = queue.front();
    queue.erase(queue.begin());
    return next;
}

SSTF::SSTF() : Scheduler() {}
void SSTF::add(IORequest* ioReq){
    queue.push_back(ioReq);
}
IORequest* SSTF::get_next() {
    if (queue.empty()) return NULL;
    int min_i = 0;
    int min_dist = fabs(queue[0]->track - curr_track);
    for (int i = 0; i < queue.size(); ++i) {
        if (fabs(queue[i]->track - curr_track) < min_dist) {
            min_dist = fabs(queue[i]->track - curr_track);
            min_i = i;
        }
    }
    IORequest* next = queue[min_i];
    queue.erase(queue.begin()+min_i);
    return next;
}

SCAN::SCAN() : Scheduler() { this->direction = 1; }
void SCAN::add(IORequest* ioReq){
    queue.push_back(ioReq);
}
IORequest* SCAN::get_next() {
    if (queue.empty()) return NULL;

    int min_i = -1, min_dist, tmp_dist;
    for (int i = 0; i < queue.size(); ++i) {
        tmp_dist = queue[i]->track - curr_track;
        if (tmp_dist*direction >= 0) { // search travel direction
            tmp_dist = abs(tmp_dist);
            if (min_i != -1) {
                if (tmp_dist < min_dist) {
                    min_i = i;
                    min_dist = tmp_dist;
                }
            }
            else {
                min_i = i;
                min_dist = tmp_dist;
            }
        }
    }
    if (min_i == -1) {                // search the other direction
        direction *= -1;
        for (int i = 0; i < queue.size(); ++i) {
            tmp_dist = queue[i]->track - curr_track;
            if (tmp_dist*direction >= 0) {
                tmp_dist = abs(tmp_dist);
                if (min_i != -1) {
                    if (tmp_dist < min_dist) {
                        min_i = i;
                        min_dist = tmp_dist;
                    }
                }
                else {
                    min_i = i;
                    min_dist = tmp_dist;
                }
            }
        }
    }
    IORequest* next = queue[min_i];
    queue.erase(queue.begin()+min_i);
    return next;
}

CSCAN::CSCAN() : Scheduler() {}
void CSCAN::add(IORequest* ioReq){
    queue.push_back(ioReq);
}
IORequest* CSCAN::get_next() {
    if (queue.empty()) return NULL;

    int min_i = -1, min_dist, tmp_dist;
    for (int i = 0; i < queue.size(); ++i) {
        tmp_dist = queue[i]->track - curr_track;
        if (tmp_dist >= 0) {
            if (min_i != -1) {
                if (tmp_dist < min_dist) {
                    min_i = i;
                    min_dist = tmp_dist;
                }
            }
            else {
                min_i = i;
                min_dist = tmp_dist;
            }
        }
    }
    if (min_i == -1) { // wrap around
        for (int i = 0; i < queue.size(); ++i) {
            tmp_dist = queue[i]->track - curr_track;
            if (min_i != -1) {
                if (tmp_dist < min_dist) {
                    min_i = i;
                    min_dist = tmp_dist;
                }
            }
            else {
                min_i = i;
                min_dist = tmp_dist;
            }
        }
    }
    IORequest* next = queue[min_i];
    queue.erase(queue.begin()+min_i);
    return next;
}

FSCAN::FSCAN() : Scheduler() { this->direction = 1; }
void FSCAN::add(IORequest* ioReq){
    queue_inactive.push_back(ioReq);
}
IORequest* FSCAN::get_next() {
    if (queue.empty()) { // active queue is empty
        if (queue_inactive.empty()) return NULL;
        vector<IORequest*> tmp;
        tmp = queue;
        queue = queue_inactive;
        queue_inactive = tmp;
        direction = 1;
    }

    int min_i = -1, min_dist, tmp_dist;
    for (int i = 0; i < queue.size(); ++i) {
        tmp_dist = queue[i]->track - curr_track;
        if (tmp_dist*direction >= 0) { // search travel direction
            tmp_dist = abs(tmp_dist);
            if (min_i != -1) {
                if (tmp_dist < min_dist) {
                    min_i = i;
                    min_dist = tmp_dist;
                }
            }
            else {
                min_i = i;
                min_dist = tmp_dist;
            }
        }
    }
    if (min_i == -1) {                // search the other direction
        direction *= -1;
        for (int i = 0; i < queue.size(); ++i) {
            tmp_dist = queue[i]->track - curr_track;
            if (tmp_dist*direction >= 0) {
                tmp_dist = abs(tmp_dist);
                if (min_i != -1) {
                    if (tmp_dist < min_dist) {
                        min_i = i;
                        min_dist = tmp_dist;
                    }
                }
                else {
                    min_i = i;
                    min_dist = tmp_dist;
                }
            }
        }
    }
    IORequest* next = queue[min_i];
    queue.erase(queue.begin()+min_i);
    return next;
}
