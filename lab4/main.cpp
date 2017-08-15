#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include <ctype.h>      /* getopt */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <cmath>        /* abs */
#include <vector>
#include "Scheduler.h"

using namespace std;

void parse_command(int argc, char* argv[], string& algo) {
    int c;
    while ( (c = getopt(argc, argv, "s:")) != -1 )
        switch (c) {
            case 's':
                algo = optarg;
                break;
            default:
                abort();
        }
}

vector<string> read_file(string path) {
    ifstream infile;
    infile.open(path);
    string line;
    vector<string> lines;
    while (!infile.eof()) {
        getline(infile, line);
        if (line.size() and line[0]!='#') lines.push_back(line);
    }
    infile.close();
    return lines;
}

void report(int total_time, vector<IORequest*> done_ioReqs) {
    int tot_movement = 0, max_waittime = 0;
    double avg_turnaround = 0, avg_waittime = 0;
    int prev_track = 0, num_reqs;

    num_reqs = done_ioReqs.size();

    for (int i = 0; i < num_reqs; ++i) {
        tot_movement += abs(done_ioReqs[i]->track - prev_track);
        prev_track = done_ioReqs[i]->track;
        avg_turnaround += done_ioReqs[i]->turnaround;
        avg_waittime += done_ioReqs[i]->waittime;
        if (done_ioReqs[i]->waittime > max_waittime) {
            max_waittime = done_ioReqs[i]->waittime;
        }
    }

    avg_turnaround /= num_reqs;
    avg_waittime /= num_reqs;
    printf("SUM: %d %d %.2lf %.2lf %d\n", total_time, \
                                          tot_movement, \
                                          avg_turnaround, \
                                          avg_waittime, \
                                          max_waittime);
}

Scheduler* setScheduler(string& algo) {
    if ( algo[0] == 'i') {
        return new FIFO();
    } else if (algo[0] == 'j') {
        return new SSTF();
    } else if (algo[0] == 's') {
        return new SCAN();
    } else if (algo[0] == 'c') {
        return new CSCAN();
    } else if (algo[0] == 'f') {
        return new FSCAN();
    } else { // default
        return new FIFO();
    }
}

int main(int argc, char *argv[]) {
    /* 0: User Input and Initialization. */
    string algo;
    parse_command(argc, argv, algo);
    Scheduler* s = setScheduler(algo);


    vector<IORequest*> ioReqs;
    vector<IORequest*> done_ioReqs;

    /* 1: Read Input File. */
    vector<string> lines = read_file(argv[argc-1]);
    for (int i = 0; i < lines.size(); ++i) {
        stringstream ss(lines[i]);
        int ts, track;
        IORequest* ioReq = new IORequest{};

        ss >> ts >> track;
        ioReq->idx = i;
        ioReq->ts = ts;
        ioReq->track = track;
        ioReqs.push_back(ioReq);
    }

    /* 2: Simulation. */
    bool is_busy = false;
    int curr_time = -1;
    int num_reqs = ioReqs.size();
    IORequest* running;
    while (done_ioReqs.size() != num_reqs) {
        curr_time += 1;
        if (!ioReqs.empty() && ioReqs[0]->ts == curr_time) { // IO arrived
            s->add(ioReqs[0]);
            ioReqs.erase(ioReqs.begin());
        }
        if (is_busy) { // active IO running
            if (running->track == s->curr_track) { // IO completed
                running->turnaround = curr_time - running->ts;
                done_ioReqs.push_back(running);
                is_busy = false;
            }
        }
        while (!is_busy) { // Issue an IO Request
            IORequest* ioReq = s->get_next();
            if (ioReq == NULL) break;
            if (ioReq) {
                ioReq->waittime = curr_time - ioReq->ts;
                running = ioReq;
                is_busy = true;
            }
            if (running->track == s->curr_track) { // IO completed
                running->turnaround = curr_time - running->ts;
                done_ioReqs.push_back(running);
                is_busy = false;
            }
        }
        if (is_busy) {
            if (running->track > s->curr_track) { s->curr_track += 1; }
            else if (running->track < s->curr_track) { s->curr_track -= 1; }
        }

    }

    /* 4: Output Results. */
    report(curr_time, done_ioReqs);

    return 0;
}
