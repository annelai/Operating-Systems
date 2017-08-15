#ifndef PAGER_H
#define PAGER_H

#include <vector>
#include <bitset>

using namespace std;

struct Frame {
    int FRAME_IDX;
    int PAGE_IDX;
};

struct PTE {
    unsigned PRESENT:1;
    unsigned MODIFIED:1;
    unsigned REFERENCED:1;
    unsigned PAGEDOUT:1;
    unsigned FRAME_IDX:8;
};

class Pager {
public:
    Pager(int num_frames) {
        this->num_frames = num_frames;
        this->page_table = new PTE*[64];
        this->frame_table = new Frame*[num_frames];

        for (int i = 0; i < 64; ++i) {
            PTE* pte = new PTE {0,0,0,0};
            page_table[i] = pte;
        }
        for (int i = 0; i < num_frames; ++i) {
            Frame* frame = new Frame {i,-1};
            frame_table[i] = frame;
        }
    }
    void add_frame(int FRAME_IDX) {
        frames.push_back(FRAME_IDX);
    }
    virtual void add_pages(int PAGE_IDX) = 0;
    virtual int allocate_frame() = 0;
    virtual void update(int PAGE_IDX, int FRAME_IDX) = 0;

    int num_frames;
    vector<int> frames;
    PTE** page_table;
    Frame** frame_table;
};

class FIFO : public Pager {
public:
    FIFO(int num_frames);
    int allocate_frame();
    void add_pages(int PAGE_IDX);
    void update(int PAGE_IDX, int FRAME_IDX);
};

class SC: public Pager {
public:
    SC(int num_frames);
    int allocate_frame();
    void add_pages(int PAGE_IDX);
    void update(int PAGE_IDX, int FRAME_IDX);
};

class CPF: public Pager {
public:
    CPF(int num_frames);
    int allocate_frame();
    void add_pages(int PAGE_IDX);
    void update(int PAGE_IDX, int FRAME_IDX);

    int hand;
};

class CVP: public Pager {
public:
    CVP(int num_frames);
    int allocate_frame();
    void add_pages(int PAGE_IDX);
    void update(int PAGE_IDX, int FRAME_IDX);

    int hand;
    vector<int> pages;
};

class APF: public Pager {
public:
    APF(int num_frames);
    int allocate_frame();
    void add_pages(int PAGE_IDX);
    void update(int PAGE_IDX, int FRAME_IDX);

    vector<bitset<32>> counter;
};

class AVP: public Pager {
public:
    AVP(int num_frames);
    int allocate_frame();
    void add_pages(int PAGE_IDX);
    void update(int PAGE_IDX, int FRAME_IDX);

    vector<bitset<32>> counter;
};

class Random: public Pager {
public:
    Random(int num_frames, int num_random, int* randvals);
    int allocate_frame();
    void add_pages(int PAGE_IDX);
    void update(int PAGE_IDX, int FRAME_IDX);

    int ofs;
    int num_random;
    int* randvals;
};

class NRU: public Pager {
public:
    NRU(int num_frames, int num_random, int* randvals);
    void add_pages(int PAGE_IDX);
    int allocate_frame();
    void reset();
    void update(int PAGE_IDX, int FRAME_IDX);

    int ofs;
    int num_random;
    int* randvals;

    int num_request;
    bool in_classes[4][64];
};

#endif
