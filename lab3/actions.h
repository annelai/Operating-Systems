#ifndef ACTIONS_H
#define ACTIONS_H

#include <iostream>
#include <iomanip>      /* setw */
#include "Pager.h"

using namespace std;

struct Stats {
    int unmaps;
    int maps;
    int ins;
    int outs;
    int zeros;
    int rws;
};

void mm_unmap(PTE* pte, Frame* frame, Stats& stats, int inst_cnt, bool option_o) {
    if (option_o) cout << inst_cnt << ": UNMAP" << setw(4) << setfill(' ') << frame->PAGE_IDX << setw(4) << setfill(' ') << frame->FRAME_IDX << endl;
    pte->PRESENT = 0;
    pte->REFERENCED = 0;
    stats.unmaps += 1;
    if (pte->MODIFIED) {
        if (option_o) cout << inst_cnt << ": OUT" << setw(6) << setfill(' ') << frame->PAGE_IDX << setw(4) << setfill(' ') << frame->FRAME_IDX << endl;
        pte->MODIFIED = 0;
        pte->PAGEDOUT = 1;
        stats.outs += 1;
    }
}
void mm_in(Frame* frame, int VP, Stats& stats, int inst_cnt, bool option_o) {
    if (option_o) cout << inst_cnt << ": IN" << setw(7) << setfill(' ') << VP << setw(4) << setfill(' ') << frame->FRAME_IDX << endl;
    frame->PAGE_IDX = VP;
    stats.ins += 1;
}
void mm_zero(Frame* frame, int VP, Stats& stats, int inst_cnt, bool option_o) {
    if (option_o) cout << inst_cnt << ": ZERO" << setw(9) << setfill(' ') << frame->FRAME_IDX << endl;
    frame->PAGE_IDX = VP;
    stats.zeros += 1;
}
void mm_map(PTE* pte, Frame* frame, Stats& stats, int inst_cnt, bool option_o) {
    if (option_o) cout << inst_cnt << ": MAP" << setw(6) << setfill(' ') << frame->PAGE_IDX << setw(4) << setfill(' ') << frame->FRAME_IDX << endl;
    pte->PRESENT = 1;
    pte->FRAME_IDX = frame->FRAME_IDX;
    stats.maps += 1;
}

#endif
