#ifndef PROCESS_H
#define PROCESS_H

class Process {
public:
    Process(int pid, int AT, int TC, int CB, int IO, int S_PR) {
        this->pid = pid;
        this->AT = AT;
        this->TC = TC;
        this->RC = TC;
        this->CB = CB;
        this->IO = IO;
        this->RB = 0;
        this->tmp_burst = 0;
        this->S_PR = S_PR;
        this->D_PR = S_PR;

        this->FT = AT;
        this->IT = 0;
        this->CW = 0;
    }
    Process();
    ~Process();

    int pid;
    int AT;                 /* Arrival Time */
    int TC;                 /* Total CPU Time */
    int RC;                 /* Remaining CPU Time */
    int CB;                 /* CPU Burst */
    int IO;                 /* IO Burst */

    int RB;                 /* Remaining CPU Burst */
    int tmp_burst;          /* temporary CPU/IO burst */

    int S_PR;               /* PRIO scheduler */
    int D_PR;

    int FT;                 /* Finishing Time */
    int IT;                 /* IO Time */
    int CW;                 /* CPU Waiting Time */

};

#endif
