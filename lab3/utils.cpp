int myrandom(int* randvals, int& ofs, int num_random, int m) {
    if (ofs > num_random-1) ofs = 0;
    int random = (randvals[ofs] % m);
    ofs += 1;
    return random;
}
