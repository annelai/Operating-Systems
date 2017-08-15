#include <vector>
#include <iostream>
#include <algorithm>

#include "Pager.h"
#include "utils.cpp"

using namespace std;

FIFO::FIFO(int num_frames) : Pager(num_frames) {}
int FIFO::allocate_frame(){
    int replaced_frame_idx = frames.front();
    rotate(frames.begin(), frames.begin() + 1, frames.end());
    return replaced_frame_idx;
}
void FIFO::add_pages(int PAGE_IDX) {}
void FIFO::update(int PAGE_IDX, int FRAME_IDX) {}

SC::SC(int num_frames) : Pager(num_frames) {}
int SC::allocate_frame(){
    int replaced_frame_idx;
    PTE* pte;
    while (1) {
        replaced_frame_idx = frames.front();
        pte = page_table[frame_table[replaced_frame_idx]->PAGE_IDX];
        rotate(frames.begin(), frames.begin() + 1, frames.end());
        if (pte->REFERENCED) pte->REFERENCED = 0;
        else return replaced_frame_idx;
    }
}
void SC::add_pages(int PAGE_IDX) {}
void SC::update(int PAGE_IDX, int FRAME_IDX) {}

CPF::CPF(int num_frames) : Pager(num_frames) {
    this->hand = 0;
}
int CPF::allocate_frame(){
    int replaced_frame_idx = -1;
    PTE* pte;
    while (replaced_frame_idx == -1) {
        if (hand == frames.size()) hand = 0;
        pte = page_table[frame_table[frames[hand]]->PAGE_IDX];
        if (pte->REFERENCED) pte->REFERENCED = 0;
        else replaced_frame_idx = hand;
        ++hand;
    }
    return replaced_frame_idx;
}
void CPF::add_pages(int PAGE_IDX) {}
void CPF::update(int PAGE_IDX, int FRAME_IDX) {}

CVP::CVP(int num_frames) : Pager(num_frames) {
    this->hand = 0;
}
int CVP::allocate_frame(){
    int replaced_frame_idx = -1;
    while (replaced_frame_idx == -1) {
        if (hand == 64) hand = 0;
        PTE* pte = page_table[hand];
        if (pte->PRESENT) {
            if (pte->REFERENCED) pte->REFERENCED = 0;
            else replaced_frame_idx = pte->FRAME_IDX;
        }
        ++hand;
    }
    return replaced_frame_idx;
}
void CVP::add_pages(int PAGE_IDX) {}
void CVP::update(int PAGE_IDX, int FRAME_IDX) {}

APF::APF(int num_frames) : Pager(num_frames) {
    for (int i = 0; i < num_frames; ++i) {
        bitset<32> c;
        c.reset();
        this->counter.push_back(c);
    }
}
int APF::allocate_frame() {
    for (vector<int>::iterator it = frames.begin(); it != frames.end(); ++it) {
        counter[(*it)] >>= 1;
        PTE* pte = page_table[frame_table[(*it)]->PAGE_IDX];
        if (pte->REFERENCED) {
            pte->REFERENCED = 0;
            counter[(*it)].set(31);
        }
    }
    int replaced_frame_idx = 0;
    bitset<32> lowest_counter;
    lowest_counter.set();
    for (vector<int>::iterator it = frames.begin(); it != frames.end(); ++it) {
        if (counter[(*it)].to_string() < lowest_counter.to_string()) {
            replaced_frame_idx = *(it);
            lowest_counter = counter[(*it)];
        }
    }
    counter[replaced_frame_idx].reset();
    return replaced_frame_idx;
}
void APF::add_pages(int PAGE_IDX) {}
void APF::update(int PAGE_IDX, int FRAME_IDX) {}

AVP::AVP(int num_frames) : Pager(num_frames) {
    for (int i = 0; i < num_frames; ++i) {
        bitset<32> c;
        c.reset();
        this->counter.push_back(c);
    }
}
int AVP::allocate_frame() {
    for (int i = 0; i < 64; ++i) {
        PTE* pte = page_table[i];
        if (!pte->PRESENT) continue;
        counter[pte->FRAME_IDX] >>= 1;
        if (pte->REFERENCED) {
            pte->REFERENCED = 0;
            counter[pte->FRAME_IDX].set(31);
        }
    }
    int replaced_frame_idx = 0;
    bitset<32> lowest_counter;
    lowest_counter.set();
    for (int i = 0; i < 64; ++i) {
        PTE* pte = page_table[i];
        if (!pte->PRESENT) continue;
        if (counter[pte->FRAME_IDX].to_string() < lowest_counter.to_string()) {
            replaced_frame_idx = pte->FRAME_IDX;
            lowest_counter = counter[pte->FRAME_IDX];
        }
    }
    counter[replaced_frame_idx].reset();
    return replaced_frame_idx;
}
void AVP::add_pages(int PAGE_IDX) {}
void AVP::update(int PAGE_IDX, int FRAME_IDX) {}

Random::Random(int num_frames, int num_random, int* randvals) : Pager(num_frames) {
    this->ofs = 0;
    this->num_random = num_random;
    this->randvals = randvals;
}
int Random::allocate_frame() {
    return myrandom(randvals, ofs, num_random, num_frames);
}
void Random::add_pages(int PAGE_IDX) {}
void Random::update(int PAGE_IDX, int FRAME_IDX) {}

NRU::NRU(int num_frames, int num_random, int* randvals) : Pager(num_frames) {
    this->ofs = 0;
    this->num_random = num_random;
    this->randvals = randvals;

    this->num_request = 0;

    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 64; ++j) {
            this->in_classes[i][j] = false;
        }
    }
}
void NRU::add_pages(int PAGE_IDX) {
    in_classes[0][PAGE_IDX] = true;
}
int NRU::allocate_frame() {
    int c = 0, num_pages_in_array = 0;
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 64; ++j) {
            if (in_classes[i][j]) num_pages_in_array += 1;
        }
        if (num_pages_in_array) {
            c = i;
            break;
        }
    }
    int random_idx = 1 + myrandom(randvals, ofs, num_random, num_pages_in_array);
    int cnt = 0;
    for (int j = 0; j < 64; ++j) {
        if (in_classes[c][j]) cnt += 1;
        if (cnt == random_idx) {
            in_classes[c][j] = false;
            reset();
            return page_table[j]->FRAME_IDX;
        }
    }
}

void NRU::reset() {
    num_request += 1;
    if (num_request < 10) return;
    num_request = 0;
    // reset REFERENCE bit
    for (vector<int>::iterator it = frames.begin(); it != frames.end(); ++it) {
        PTE* pte = page_table[frame_table[*it]->PAGE_IDX];
        pte->REFERENCED = 0;
    }
    // update class
    for (int j = 0; j < 64; ++j) {
        in_classes[0][j] |= in_classes[2][j];
        in_classes[1][j] |= in_classes[3][j];
        in_classes[2][j] = false;
        in_classes[3][j] = false;
    }

}

void NRU::update(int PAGE_IDX, int FRAME_IDX) {
    // update class
    PTE* pte = page_table[PAGE_IDX];
    if (pte->MODIFIED) {
        in_classes[3][PAGE_IDX] = true;
        in_classes[2][PAGE_IDX] = false;
        in_classes[1][PAGE_IDX] = false;
        in_classes[0][PAGE_IDX] = false;
    }
    else {
        in_classes[2][PAGE_IDX] = true;
        in_classes[0][PAGE_IDX] = false;
    }
}
