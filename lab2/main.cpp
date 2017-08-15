#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <stdlib.h>     /* atoi */

#include <ctype.h>      /* getopt */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "DESLayer.h"
#include "Scheduler.h"
#include "Process.h"

using namespace std;

int* randvals;
int num_random, ofs = 0;

int s2i(string str) { return atoi(str.c_str()); }

void parse_command(int argc, char* argv[], bool& verbose, string& schedspec) {
    int c;
    while ( (c = getopt(argc, argv, "vs:")) != -1 )
        switch (c) {
            case 'v':
                verbose = true;
                break;
            case 's':
                schedspec = optarg;
                break;
            default:
                abort();
        }
}

vector<string> read_file(string file_name) {
    ifstream infile;
    infile.open(file_name);
    string line;
    vector<string> processInfo;
    while (!infile.eof()) {
        getline(infile, line);
        if (line.size()) processInfo.push_back(line);
    }
    infile.close();
    return processInfo;
}

void load_random(string file_name) {
    ifstream infile;
    infile.open(file_name);
    string line;
    getline(infile, line);

    num_random = s2i(line);
    randvals = new int[num_random];
    for (int i = 0; i < num_random; ++i) {
        getline(infile, line);
        randvals[i] = s2i(line);
    }
}

int myrandom(int burst) {
    if (ofs > num_random-1) ofs = 0;
    int random = 1 + (randvals[ofs] % burst);
    ofs += 1;
    return random;
}

void report(Scheduler* s, vector<Process*> processList, string schedspec, int quantum) {
    /* 0: initialization */
    int num_process = processList.size();
    int total_tt = 0;
    int total_cw = 0;
    int cpu_util = 0;

    /* 1: Scheduler information. */
    if (schedspec == "F") cout << "FCFS" << endl;
    else if (schedspec == "L") cout << "LCFS" << endl;
    else if (schedspec == "S") cout << "SJF" << endl;
    else if (schedspec[0] == 'R') cout << "RR " << quantum << endl;
    else cout << "PRIO " << quantum << endl;

    /* 2: Per process information. */
    for (vector<Process*>::iterator it = processList.begin(); it != processList.end(); ++it) {
        printf("%04d: %4d %4d %4d %4d %1d | %5d %5d %5d %5d\n",
                (*it)->pid,
                (*it)->AT, (*it)->TC, (*it)->CB, (*it)->IO, (*it)->S_PR,
                (*it)->FT, // last time stamp
                (*it)->FT - (*it)->AT,
                (*it)->IT,
                (*it)->CW);
        total_tt += ((*it)->FT-(*it)->AT);
        total_cw += (*it)->CW;
        cpu_util += (*it)->TC;
    }
    /* 3: Summary Information. */
    printf("SUM: %d %.2lf %.2lf %.2lf %.2lf %.3lf\n",
           s->finish_exe,
           100.0*cpu_util/s->finish_exe,
           100.0*s->io_util/s->finish_exe,
           1.0*total_tt/num_process,
           1.0*total_cw/num_process,
           100.0*num_process/s->finish_exe);
}

Scheduler* setScheduler(string& schedspec, int& quantum) {
    if ( schedspec[0] == 'F') {
        return new FCFS();
    } else if (schedspec[0] == 'L') {
        return new LCFS();
    } else if (schedspec[0] == 'S') {
        return new SJF();
    } else if (schedspec[0] == 'R') {
        quantum = s2i(schedspec.substr(1));
        return new RR(quantum);
    } else if (schedspec[0] == 'P') {
        quantum = s2i(schedspec.substr(1));
        return new PRIO(quantum);
    } else {
        schedspec = "F"; // default scheduler: FCFS
        return new FCFS();
    }
}

int main (int argc, char *argv[]) {
    /* 0: User Input and Initialization. */
    bool verbose = false;
    string schedspec;
    int pid = 0;
    int quantum = 10000;    // treat non-preemtive as preemptive with very large timeout
    int io_start;           // time when the first process enters I/O (block) state
    int io_num;             // number of process in the I/O (block) state

    parse_command(argc, argv, verbose, schedspec);
    Scheduler* s = setScheduler(schedspec, quantum);
    DESLayer* d = new DESLayer();


    /* 1: Read Input/Random Number Files. */
    vector<string> processInfo = read_file(argv[argc-2]);
    load_random(argv[argc-1]);

    /* 2: Create Events for Simulation. */
    vector<Process*> processList;
    for(vector<string>::iterator it = processInfo.begin(); it != processInfo.end(); ++it) {
        stringstream ss(*it);
        int AT,TC,CB,IO;
        ss >> AT >> TC >> CB >> IO;
        Process* p = new Process(pid,AT,TC,CB,IO,myrandom(4));
        processList.push_back(p);
        d->put_event(AT,p,TRANS_TO_READY);
        pid += 1;
    }
    /* 3: Simulation. */
    Event* evt;
    int burst = 0;                // record temp CPU/IO burst
    bool cpu_busy = false;        // CPU busy when a process in RUN state
    bool io_busy = false;         // IO busy when at least one process in BLOCK state
    while( (evt = d->get_event()) ) {
        bool CALL_SCHEDULER = false;
        Process* CURRENT_RUNNING_PROCESS = NULL;
        int CURRENT_TIME = evt->evtTimeStamp;
        evt->evtProcess->FT = CURRENT_TIME;
        if (verbose) cout << CURRENT_TIME << " " << evt->evtProcess->pid;
        switch(evt->evtTrans) {
            case TRANS_TO_READY:
                if (verbose) cout << " -> READY\n";
                evt->evtProcess->D_PR -= 1;
                if (evt->evtProcess->tmp_burst != 0) { // come from BLOCK
                    io_num -= 1;
                    if (io_num == 0) {
                        io_busy = false;
                        s->io_util += (CURRENT_TIME - io_start);
                    }
                }
                evt->evtProcess->D_PR = evt->evtProcess->S_PR - 1;
                s->add_process(evt->evtProcess);
                CALL_SCHEDULER = true;
                break;
            case TRANS_TO_RUN:
                if (verbose) cout << " -> RUNNG\n";
                evt->evtProcess->RC -= evt->evtProcess->tmp_burst;
                evt->evtProcess->FT += evt->evtProcess->tmp_burst;
                if (evt->evtProcess->RC == 0) { // process done!
                    d->put_event(evt->evtProcess->FT, evt->evtProcess, TRANS_TO_DONE);
                    break;
                }
                if (evt->evtProcess->RB == 0) {
                    burst = myrandom(evt->evtProcess->IO);
                    evt->evtProcess->tmp_burst = burst;
                    d->put_event(evt->evtProcess->FT, evt->evtProcess, TRANS_TO_BLOCK);
                }
                else {
                    d->put_event(evt->evtProcess->FT, evt->evtProcess, TRANS_TO_PREEMPT);
                }
                break;
            case TRANS_TO_BLOCK:
                if (verbose) cout << " -> BLOCK\n";
                if (!io_busy) {
                    io_start = CURRENT_TIME;
                    io_busy = true;
                }
                io_num += 1;
                cpu_busy = false;
                evt->evtProcess->IT += evt->evtProcess->tmp_burst;
                evt->evtProcess->FT += evt->evtProcess->tmp_burst;
                d->put_event(evt->evtProcess->FT, evt->evtProcess, TRANS_TO_READY);
                CALL_SCHEDULER = true;
                break;
            case TRANS_TO_PREEMPT:
                if (verbose) cout << "-> PREEMT\n";
                cpu_busy = false;
                evt->evtProcess->D_PR -= 1;
                s->add_process(evt->evtProcess);
                CALL_SCHEDULER = true;
                break;
            case TRANS_TO_DONE:
                if (verbose) cout << "-> DONE\n";
                cpu_busy = false;
                s->finish_exe = evt->evtProcess->FT;
                CALL_SCHEDULER = true;
                break;
        }
        delete evt;

        if(CALL_SCHEDULER && !cpu_busy) {
            if (d->get_next_event_time() == CURRENT_TIME) { continue; }
            CURRENT_RUNNING_PROCESS = s->get_next_process();
            if (CURRENT_RUNNING_PROCESS == NULL) { continue; }
            cpu_busy = true;
            CURRENT_RUNNING_PROCESS->CW += (CURRENT_TIME - CURRENT_RUNNING_PROCESS->FT);
            if (CURRENT_RUNNING_PROCESS->RB == 0)
                burst = myrandom(CURRENT_RUNNING_PROCESS->CB);
            else
                burst = CURRENT_RUNNING_PROCESS->RB;
            if (quantum < burst) {   // preempt
                CURRENT_RUNNING_PROCESS->RB = burst - min(quantum,CURRENT_RUNNING_PROCESS->RC);
                burst = min(quantum,CURRENT_RUNNING_PROCESS->RC);
            }
            else {
                CURRENT_RUNNING_PROCESS->RB = burst - min(burst,CURRENT_RUNNING_PROCESS->RC);
                burst = min(burst,CURRENT_RUNNING_PROCESS->RC);
            }
            CURRENT_RUNNING_PROCESS->tmp_burst = burst;
            d->put_event(CURRENT_TIME, CURRENT_RUNNING_PROCESS, TRANS_TO_RUN);
        }
    }

    /* 4: Output Results. */
    report(s, processList, schedspec, quantum);

    return 0;
}
