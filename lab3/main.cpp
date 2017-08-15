#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <stdlib.h>     /* atoi */
#include <ctype.h>      /* getopt */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "Pager.h"
#include "actions.h"

using namespace std;

int* randvals;
int num_random;

int curr_frame_idx = 0;

int s2i(string str) { return atoi(str.c_str()); }

void parse_command(int argc, char* argv[], string& algo, string& options, int& num_frames) {
    int c;
    while ( (c = getopt(argc, argv, "a:o:f:")) != -1 )
        switch (c) {
            case 'a':
                algo = optarg;
                break;
            case 'o':
                options = optarg;
                break;
            case 'f':
                num_frames = atoi(optarg);
                break;
            default:
                abort();
        }
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
    infile.close();
}

string get_next_instruction(ifstream& infile) {
    string line = "";
    while (!infile.eof()) {
        getline(infile, line);
        if (line.size() and line[0]!='#') return line;
    }
    infile.close();
    return line;
}

void update_pte(PTE* pte, int RW) {
    if (RW) //write
        pte->MODIFIED = 1;
    pte->REFERENCED = 1;
}

int get_frame_idx(Pager* p, int num_frames) {
    int frame_idx;
    if (curr_frame_idx >= num_frames) {
        frame_idx = p->allocate_frame();
    }
    else {
        frame_idx = curr_frame_idx;
        p->add_frame(frame_idx);
        curr_frame_idx += 1;
    }
    return frame_idx;
}

Pager* setPager(string algo, int num_frames) {
    if (algo == "a")                    // Aging (Physical Frame)
        return new APF(num_frames);
    else if (algo == "c")               // Clock (Physical Frame)
        return new CPF(num_frames);
    else if (algo == "f")               // FIFO  (Physical Frame)
        return new FIFO(num_frames);
    else if (algo == "N")               // NRU   (Virtual Page)
        return new NRU(num_frames,num_random,randvals);
    else if (algo == "r")               // Random (Physical Frame)
        return new Random(num_frames,num_random,randvals);
    else if (algo == "s")               // Second Chance (Physical Frame)
        return new SC(num_frames);
    else if (algo == "X")               // Clock (Virtual Page)
        return new CVP(num_frames);
    else if (algo == "Y")               // Aging (Virtual Page)
        return new AVP(num_frames);
    else
        return new FIFO(num_frames);
}

int main(int argc, char *argv[]) {
    /* 0: User Input and Initialization. */
    int RW, VP; // read-write bit, index of virtual page
    int num_frames;
    int inst_cnt = 0;
    string algo, options;
    parse_command(argc, argv, algo, options, num_frames);
    bool option_o = false, option_p = false, option_f = false, option_s = false;
    for (int i = 0; i < options.size(); ++i) {
        if (options[i] == 'O') option_o = true;
        else if (options[i] == 'P') option_p = true;
        else if (options[i] == 'F') option_f = true;
        else if (options[i] == 'S') option_s = true;
    }

    Stats stats {0,0,0,0,0};

    /* 1: Read Random Number Files. */
    load_random(argv[argc-1]);

    /* 2: Simulation. */
    Pager* p = setPager(algo, num_frames);

    ifstream infile;
    infile.open(argv[argc-2]);
    string instr = get_next_instruction(infile);
    PTE* pte1;
    PTE* pte2;
    Frame* frame;
    int frame_idx;

    while (!instr.empty()) {

        stringstream ss(instr);
        ss >> RW >> VP;
        if (option_o) cout << "==> inst: " << instr << endl;
        stats.rws += 1;

        pte1 = p->page_table[VP];

        if (!pte1->PRESENT) {
            frame_idx = get_frame_idx(p,num_frames);
            frame = p->frame_table[frame_idx];
        }
        else {
            frame = p->frame_table[pte1->FRAME_IDX];
        }
        if (frame->PAGE_IDX != VP) {
            p->add_pages(VP);
            pte2 = p->page_table[frame->PAGE_IDX];
            if (frame->PAGE_IDX != -1) {
                mm_unmap(pte2, frame, stats, inst_cnt, option_o);
            }
            if (pte1->PAGEDOUT) { // PAGE IN
                mm_in(frame, VP, stats, inst_cnt, option_o);
            }
            else { // ZERO
                mm_zero(frame, VP, stats, inst_cnt, option_o);
            }
            // MAP
            mm_map(pte1, frame, stats, inst_cnt, option_o);
        }

        update_pte(pte1, RW);
        p->update(VP,pte1->FRAME_IDX);

        inst_cnt += 1;

        instr = get_next_instruction(infile);
    }

    /* 3: Output Results. */
    if (option_p) { // page table option
        for (int i = 0; i < 64; ++i) {
            PTE* pte = p->page_table[i];
            unsigned R = pte->REFERENCED, M = pte->MODIFIED, S = pte->PRESENT, P = pte->PAGEDOUT;
            if (!S) {
                if (!P) cout << "* ";
                else cout << "# ";
            }
            else {
                cout << i << ":";
                if (R) cout << "R";
                else cout << "-";
                if (M) cout << "M";
                else cout << "-";
                if (P) cout << "S";
                else cout << "-";
                cout << " ";
            }
        }
        cout << endl;
    }
    if (option_f) { // frame table option
        for (int i = 0; i < num_frames; ++i) {
            Frame* frame = p->frame_table[i];
            if (frame->PAGE_IDX == -1) cout << "* ";
            else cout << frame->PAGE_IDX << " ";
        }
        cout << endl;
    }

    if (option_s) { // Summery
        printf("SUM %d U=%d M=%d I=%d O=%d Z=%d ===> %llu\n",
               inst_cnt, stats.unmaps, stats.maps, stats.ins, stats.outs, stats.zeros, stats.unmaps*400 + stats.maps*400 + stats.ins*3000 + stats.outs*3000 + stats.zeros*150 + stats.rws);
    }

    delete pte1;
    delete pte2;
    delete frame;
    return 0;
}
